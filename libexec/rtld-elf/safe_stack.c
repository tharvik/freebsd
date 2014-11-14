/*-
 * Copyright (c) 2014 Alexandre BIQUE <bique.alexandre@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD$
 */

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <machine/param.h>
#include <unistd.h>
#include <stdlib.h>

#include "rtld.h"
#include "safe_stack.h"

void *
_rtld_allocate_unsafe_stack(void)
{
	struct rlimit  rl;
        size_t	       stack_size = sizeof (void *) * (1 << 20); // pointer size * 1MB
	size_t	       guard_size = PAGE_SIZE > 4096 ? PAGE_SIZE : 4096;
	void	      *memory;

	/* getrlimit to know the stack size */
	if (!syscall(SYS_getrlimit, RLIMIT_STACK, &rl) &&
            rl.rlim_cur != RLIM_INFINITY)
		stack_size = rl.rlim_cur;

	/* Allocate memory.
         * About the stack protection, it should use rtld_get_stack_prot for
         * dynamicly linked binaries. But how to know when it is the case? */
	memory = ((void *(*)(int, void *, size_t, int, int, int, off_t))syscall)(
                SYS_mmap, NULL, rl.rlim_cur + guard_size,
                _rtld_get_stack_prot(), MAP_STACK, -1, 0);
	if (memory == MAP_FAILED)
		exit(1);

	/* setup the stack guard */
	syscall(SYS_mprotect, memory, guard_size, PROT_NONE);

	return ((char *)memory) + guard_size + stack_size;
}
