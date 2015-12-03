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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <machine/elf.h>
#include <string.h>
#include <safestack.h>

#include "notes.h"

extern char **environ;

/*
 * Find and parse ELF headers to check whether the safestack note is
 * present in this program.
 */
static inline int
has_safestack_note(void) {
	Elf_Addr *sp;
	Elf_Auxinfo *aux, *auxp;
	Elf_Phdr *phdr;
	size_t phent, phnum;
	Elf_Addr note_start, note_end;
	const Elf_Note *note;
	const char *note_name;
	int i;

	sp = (Elf_Addr *) environ;
	while (*sp++ != 0)
		;
	aux = (Elf_Auxinfo *) sp;
	phdr = 0;
	phent = phnum = 0;
	for (auxp = aux; auxp->a_type != AT_NULL; auxp++) {
		if (auxp->a_type == AT_PHDR)
			phdr = auxp->a_un.a_ptr;
		else if (auxp->a_type == AT_PHENT)
			phent = auxp->a_un.a_val;
		else if (auxp->a_type == AT_PHNUM)
			phnum = auxp->a_un.a_val;
	}
	if (phdr == 0 || phent != sizeof(Elf_Phdr) || phnum == 0)
		return 0;

	for (i = 0; (unsigned) i < phnum; i++) {
		if (phdr[i].p_type != PT_NOTE)
			continue;

		note_start = (Elf_Addr) phdr[i].p_vaddr;
		note_end = note_start + phdr[i].p_filesz;

		for (note = (const Elf_Note *)note_start;
		    (Elf_Addr)note < note_end;
		    note = (const Elf_Note *)(const void *)(
		    (const char *)(note + 1) +
		    roundup2(note->n_namesz, sizeof(Elf32_Addr)) +
		    roundup2(note->n_descsz, sizeof(Elf32_Addr)))) {
			note_name = (const char *)(note + 1);
			if (note->n_namesz == sizeof(NOTE_SAFESTACK_VENDOR) &&
			    note->n_descsz == sizeof(int32_t) &&
			    strncmp(NOTE_SAFESTACK_VENDOR, note_name,
			    sizeof(NOTE_SAFESTACK_VENDOR)) == 0 &&
			    note->n_type == SAFESTACK_NOTETYPE) {
				return 1;
			}
		}
	}

	return 0;
}

/*
 * Initialize safestack for the main thread if required for this program.
 */
static inline void
init_safestack(void)
{
	if (has_safestack_note())
		__safestack_init();
}
