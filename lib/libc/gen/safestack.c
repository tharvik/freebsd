/*-
 * Copyright 2015 Volodymyr Kuznetsov <ks.vladimir@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <safestack.h>

#include <machine/param.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <unistd.h>
#include <link.h>

#include "libc_private.h"

int __safestack_enabled = 0;

#ifndef SAFESTACK_USE_TCB
__thread void* __safestack_unsafe_stack_ptr;
#endif

int
__safestack_init(void)
{
	if (__safestack_enabled)
		return 1;

	if (__isthreaded)
		/* This is not supported for now, but can be implemented later
		 * by allocating unsafe stack for all existing threads. */
		return 0;

	size_t stacksize = sizeof (void *) * (1 << 20); // pointer size * 1MB
	size_t guardsize = PAGE_SIZE > 4096 ? PAGE_SIZE : 4096;
	struct rlimit rl;
	void *memory;

	/* determine the unsafe stack size according to RLIMIT_STACK, or use
	 * the default value if RLIMIT_STACK is infinity */
	if (syscall(SYS_getrlimit, RLIMIT_STACK, &rl) == 0 &&
	    rl.rlim_cur != RLIM_INFINITY)
		stacksize = rl.rlim_cur;

	/* allocate memory, using the same protection as the main stack */
	memory = ((void *(*)(int, void *, size_t, int, int, int, off_t))syscall)
	    (SYS_mmap, NULL, stacksize + guardsize,
	    _rtld_get_stack_prot(), MAP_STACK, -1, 0);
	if (memory == MAP_FAILED)
    return 0;

	/* setup the stack guard */
	syscall(SYS_mprotect, memory, guardsize, PROT_NONE);

	__set_unsafe_stack_ptr(((char *) memory) + guardsize + stacksize);
	__safestack_enabled = 1;
	return 1;
}
