/*-
 * Copyright (c) 2015 Volodymyr Kuznetsov <ks.vladimir@gmail.com>
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
 * $FreeBSD$
 */
#ifndef _SAFESTACK_H_
#define _SAFESTACK_H_

/* Safestack enabled flag, set to a non-null value if safestack is being used
 * by the current process. The variable is defined in crt1 and set by either
 * rtld or crt1 based on safestack note in the ELF headers. */
extern int __safestack_enabled;

/* Initialize safestack. Return 1 on success, 0 on error */
int __safestack_init(void);

extern __thread void* __safestack_unsafe_stack_ptr;

static inline void *__get_unsafe_stack_ptr() {
	return __safestack_unsafe_stack_ptr;
}

static inline void __set_unsafe_stack_ptr(void* ptr) {
	__safestack_unsafe_stack_ptr = ptr;
}

#endif // _SAFESTACK_H_
