/***************************************************************************
 * MDP: GNU `as` (x86) macros.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef _MDP_GNU_AS_X86_INC
#define _MDP_GNU_AS_X86_INC

#if defined(__linux__) && defined(__ELF__)
	/* Mark the stack as non-executable on ELF. */
	.section .note.GNU-stack, "", %progbits
#endif

/* PIC macros from http://www.gentoo.org/proj/en/hardened/pic-fix-guide.xml */

#ifdef __PIC__
	/* Position-independent code. */
	#undef __i686			/* gcc builtin define gets in our way */
	#define MUNG_LOCAL(sym)		sym ## @GOTOFF(%ebx)
	#define MUNG_EXTERN(sym)	sym ## @GOT(%ebx)
	#define DEREF_EXTERN(reg)	movl (reg), reg
	#define INIT_PIC() \
		call __i686.get_pc_thunk.bx ; \
		addl $_GLOBAL_OFFSET_TABLE_, %ebx
#else
	/* Non-PIC code. */
	#define MUNG_LOCAL(sym)   sym
	#define MUNG_EXTERN(sym)  sym
	#define DEREF_EXTERN(reg)
	#define INIT_PIC()
#endif

/* Symbol types. */
#if defined(__ELF__)
	#define SYMTYPE(sym, symtype) .type sym, ## symtype
#else
	#define SYMTYPE(sym, symtype)
#endif

/* Win32: Read-only data section are named ".rdata", not ".rodata" */
#if defined(_WIN32)
	#define rodata rdata
#endif

#endif /* _MDP_GNU_AS_X86_INC */
