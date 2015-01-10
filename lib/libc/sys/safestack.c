/*
 * Copyright (c) 2015 Alexandre BIQUE <bique.alxexandre@gmail.com>
 *	All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <ucontext.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "libc_private.h"

#define SAFESTACK_ALLOCATOR_BLOCK_SIZE (1 << 14)

#define safestack_array_size(X) (sizeof (X) / sizeof (X[0]))

#define safestack_dlist_append(Head, Item)              \
        do {                                            \
                if (!(Head)) {                          \
                        (Head) = (Item);                \
                        (Item)->next = (Item);          \
                        (Item)->prev = (Item);          \
                } else {                                \
                        (Item)->next = (Head);          \
                        (Item)->prev = (Head)->prev;    \
                        (Item)->prev->next = (Item);    \
                        (Item)->next->prev = (Item);    \
                }                                       \
        } while (0)

#define safestack_dlist_remove(Head, Item)                      \
        do {                                                    \
                if ((Item)->next == (Item))                     \
                        (Head) = NULL;                          \
                else {                                          \
                        (Head) = (Item)->next;                  \
                        (Item)->next->prev = (Item)->prev;      \
                        (Item)->prev->next = (Item)->next;      \
                }                                               \
                (Item)->next = NULL;                            \
                (Item)->prev = NULL;                            \
        } while (0)

struct safestack_uctx_entry {
        struct safestack_uctx_entry *prev;
        struct safestack_uctx_entry *next;

        void   *ptr;
        void   *unsafe_ptr; /* unsafe stack associated to this entry */
        size_t  size;
};

struct safestack_uctx_allocator_block {
        struct safestack_uctx_allocator_block *next;
        struct safestack_uctx_entry            entries[0];
};

struct safestack_uctx_allocator {
        struct safestack_uctx_allocator_block *blocks;
        struct safestack_uctx_entry           *free_entries; /* dlist */
};

struct safestack_uctx {
        pthread_mutex_t                   mutex;
        struct safestack_uctx_allocator   allocator;
        struct safestack_uctx_entry     **entries;
        size_t                            entries_size;
        size_t                            entries_count;
};

static struct safestack_uctx g_safestack_uctx = {
        PTHREAD_MUTEX_INITIALIZER,
        { NULL, NULL },
        NULL,
        0,
        0
};

/* both set/get functions will be replaced by
 * __builtin_safestack_{get,set}_usp() */
static inline void
safestack_set_usp(void *ptr)
{
#ifdef __x86_64__
        __asm __volatile("movq %0, %%fs:0x18" :: "r" (ptr));
#elif defined(__i386__)
        __asm __volatile("movl %0, %%gs:0xc" :: "r" (ptr));
#else
# error "not supported yet"
#endif
}

static inline void *
safestack_get_usp(void)
{
        void *ptr;
#ifdef __x86_64__
        __asm __volatile("movq %%fs:0x18, %0" : "=r" (ptr));
#elif defined(__i386__)
        __asm __volatile("movl %%gs:0xc, %0" : "=r" (ptr));
#else
# error "not supported yet"
#endif
        return ptr;
}

/* This hash function removes the uninteresting part of the pointer. */
static inline uint32_t
safestack_hash_stack(void *addr, size_t size)
{
        return ((ptrdiff_t)addr) / size;
}

/* Use some prime numbers from google for the hast table size.
 * Also, has we have vectors multiples of PAGE_SIZE, I removed
 * the first entries. */
static const uint32_t safestack_hash_table_size[] = {
        389,
        769,
        1543,
        3079,
        6151,
        12289,
        24593,
        49157,
        98317,
        196613,
        393241,
        786433,
        1572869,
        3145739,
        6291469,
        12582917,
        25165843,
        50331653,
        100663319,
        201326611,
        402653189,
        805306457,
        1610612741,
};

/* Finds a good size for the hash table.
 * It uses some prime numbers from google to reduce clustering. */
static uint32_t
safestack_hash_best_size(void)
{
        uint32_t i;
        for (i = 0; i < safestack_array_size(safestack_hash_table_size); ++i) {
                if (g_safestack_uctx.entries_count < safestack_hash_table_size[i])
                        return safestack_hash_table_size[i];
        }
        return safestack_hash_table_size[i - 1];
}

/* Compute a valid size for mmap. */
static inline size_t
safestack_hash_mmap_size(size_t size)
{
        return ((size * sizeof (void*)) & ~4095ULL) + 4096;
}

static void
safestack_hash_rehash(size_t new_size);

/* Adds an entry to the hash table.
 * Automaticaly rehash when needed, but only grow, never shrink. */
static void
safestack_hash_insert(struct safestack_uctx_entry *entry)
{
        size_t best_size = safestack_hash_best_size();
        if (best_size > g_safestack_uctx.entries_size)
                safestack_hash_rehash(best_size);

        size_t hash = safestack_hash_stack(entry->ptr, entry->size);
        size_t index = hash % g_safestack_uctx.entries_size;
        safestack_dlist_append(g_safestack_uctx.entries[index], entry);
        ++g_safestack_uctx.entries_count;
}

/* Removes an entry from the hash table.
 * Does not free the entry.
 * Does not rehash the hash table. */
static void
safestack_hash_remove(struct safestack_uctx_entry *entry)
{
        size_t hash = safestack_hash_stack(entry->ptr, entry->size);
        size_t index = hash % g_safestack_uctx.entries_size;
        safestack_dlist_remove(g_safestack_uctx.entries[index], entry);
        --g_safestack_uctx.entries_count;
}

/* Rehash the hash table. */
static void
safestack_hash_rehash(size_t new_size)
{
        /* allocate new vector */
        size_t new_mmap_size = safestack_hash_mmap_size(new_size);
        struct safestack_uctx_entry **new_entries = mmap(
                NULL, new_mmap_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
        if (new_entries == MAP_FAILED)
                return;
        memset(new_entries, 0, new_mmap_size);

        /* backup old container */
        struct safestack_uctx_entry **old_entries = g_safestack_uctx.entries;
        size_t old_size = g_safestack_uctx.entries_size;
        size_t old_count = g_safestack_uctx.entries_count;
        size_t old_mmap_size = safestack_hash_mmap_size(old_size);

        /* copy new container */
        g_safestack_uctx.entries = new_entries;
        g_safestack_uctx.entries_size = new_size;
        g_safestack_uctx.entries_count = 0;

        /* was old container allocated? */
        if (!old_entries)
                return;

        /* add old entries */
        for (size_t i = 0; i < old_size; ++i) {
                while (old_entries[i]) {
                        struct safestack_uctx_entry *entry = old_entries[i];
                        safestack_dlist_remove(old_entries[i], entry);
                        safestack_hash_insert(entry);
                }
        }
        assert(old_count == g_safestack_uctx.entries_count);

        /* move entries */
        munmap(old_entries, old_mmap_size);
}

/* Find an entry that matches (addr, size). */
static struct safestack_uctx_entry *
safestack_uctx_find(void *addr, size_t size)
{
        if (size == 0 || g_safestack_uctx.entries_count == 0)
                return NULL;

        size_t hash = safestack_hash_stack(addr, size);
        size_t index = hash % g_safestack_uctx.entries_size;
        struct safestack_uctx_entry *it = g_safestack_uctx.entries[index];

        if (!it)
                return NULL;

        do {
                if (addr == it->ptr && size == it->size)
                        return it;

                it = it->next;
        } while (it != g_safestack_uctx.entries[index]);

        return NULL;
}

/* allocate a new entry, and if needed a new allocator block */
static inline struct safestack_uctx_entry *
safestack_uctx_entry_alloc(void)
{
        struct safestack_uctx_entry *entry = g_safestack_uctx.allocator.free_entries;

        /* do we have free entries? */
        if (entry) {
                safestack_dlist_remove(g_safestack_uctx.allocator.free_entries, entry);
                return entry;
        }

        /* needs to allocate a new block */
        void *ptr = mmap(NULL, SAFESTACK_ALLOCATOR_BLOCK_SIZE, PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED)
                return NULL;

        /* calculating the end of the block */
        struct safestack_uctx_entry *end = ptr + SAFESTACK_ALLOCATOR_BLOCK_SIZE;
        struct safestack_uctx_allocator_block *block = ptr;
        block->next = g_safestack_uctx.allocator.blocks;
        g_safestack_uctx.allocator.blocks = block->next;

        /* initialize every entries */
        for (entry = block->entries; entry < end; ++entry)
                safestack_dlist_append(g_safestack_uctx.allocator.free_entries, entry);

        /* take the first entry */
        entry = g_safestack_uctx.allocator.free_entries;
        safestack_dlist_remove(g_safestack_uctx.allocator.free_entries, entry);
        return entry;
}

/* Adds an entry to the free list. */
static inline void
safestack_uctx_entry_free(struct safestack_uctx_entry *val)
{
        safestack_dlist_append(g_safestack_uctx.allocator.free_entries, val);
}

/* Allocates a new entry, allocate a new unsafe stack and save the
 * entry into the hash table */
static struct safestack_uctx_entry *
safestack_uctx_add(void *addr, size_t len)
{
        if (len == 0)
                return NULL;

        struct safestack_uctx_entry *entry = safestack_uctx_entry_alloc();
        if (!entry)
                return NULL;
        entry->ptr  = addr;
        entry->size = len;
        entry->unsafe_ptr = mmap(
                NULL, len, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_STACK | MAP_ANONYMOUS,
                -1, 0);

        /* XXX: add guards? */

        if (entry->unsafe_ptr == MAP_FAILED) {
                safestack_uctx_entry_free(entry);
                return NULL;
        }

        safestack_hash_insert(entry);
        return entry;
}

/* Checks that ucp has a safestack allocated, and if not it tries to allocate
 * it and save it into the hash table has well.
 * On success returns 0, -1 otherwise. */
static int
safestack_ensure_allocated(ucontext_t *ucp)
{
        if (ucp->uc_usp_ptr)
                return 0;

        /* lock the global map and find/allocate an entry */
        pthread_mutex_lock(&g_safestack_uctx.mutex);
        struct safestack_uctx_entry *entry = safestack_uctx_find(
                ucp->uc_stack.ss_sp, ucp->uc_stack.ss_size);
        if (!entry) {
                entry = safestack_uctx_add(ucp->uc_stack.ss_sp, ucp->uc_stack.ss_size);
                if (!entry) {
                        pthread_mutex_unlock(&g_safestack_uctx.mutex);
                        errno = ENOMEM;
                        return -1;
                }
                ucp->uc_usp_ptr = entry->unsafe_ptr + entry->size;
        }
        pthread_mutex_unlock(&g_safestack_uctx.mutex);
        return 0;
}

/***************************
 * SafeStack libc wrappers *
 ***************************/

/* If we track munmap on a user stack that was used for swapcontex(),
 * or setcontext(), then we also munmap the unsafe stack we allocated
 * for it. */
static int
safestack_munmap(void *addr, size_t len)
{
        /* The following test is always correct regardless of the missing
         * synchronisation.
         * Also it did prevent a bug during jemalloc initialization, which occured in
         * pthread_mutex_lock(&g_safestack_uctx.mutex). */
        if (g_safestack_uctx.entries_count == 0)
                return __sys_munmap(addr, len);

        pthread_mutex_lock(&g_safestack_uctx.mutex);
        struct safestack_uctx_entry *entry = safestack_uctx_find(addr, len);
        if (entry) {
                __sys_munmap(entry->ptr, entry->size);
                safestack_hash_remove(entry);
                safestack_uctx_entry_free(entry);
        }
        pthread_mutex_unlock(&g_safestack_uctx.mutex);
        return __sys_munmap(addr, len);
}

/* Check if the unsafe stack is allocated for ucp.
 * If not, then allocate it ;-). */
static int
safestack_setcontext(const ucontext_t *ucp)
{
        /* do we already have an unsafe stack? */
        if (safestack_ensure_allocated((ucontext_t *)ucp))
                return -1;

        /* set the new unsafe stack pointer */
        safestack_set_usp(ucp->uc_usp_ptr);
        return __sys_setcontext(ucp);
}

/* Save the unsafe stack pointer to oucp. */
static int
safestack_getcontext(ucontext_t *ucp)
{
        ucp->uc_usp_ptr = safestack_get_usp();
        return __sys_getcontext(ucp);
}

/* Check if the unsafe stack is allocated for ucp.
 * If not, then allocate it ;-).
 * Also save the unsafe stack pointer to oucp. */
static int
safestack_swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
        oucp->uc_usp_ptr = safestack_get_usp();

        /* do we already have an unsafe stack? */
        if (safestack_ensure_allocated((ucontext_t *)ucp))
                return -1;

        /* set the new unsafe stack pointer */
        safestack_set_usp(ucp->uc_usp_ptr);
        return __sys_swapcontext(oucp, ucp);
}


/*********************
 * LIBC entry points *
 *********************/

int munmap(void *addr, size_t len)
{
        return safestack_munmap(addr, len);
}

int setcontext(const ucontext_t *ucp)
{
        return safestack_setcontext(ucp);
}

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
        return safestack_swapcontext(oucp, ucp);
}

int getcontext(ucontext_t *ucp)
{
        return safestack_getcontext(ucp);
}
