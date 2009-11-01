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
	.section .note.GNU-stack,"",%progbits
#endif

/* Symbol declaration. */
#if defined(__ELF__)
#define SYM(x) x
#else
#define SYM(x) _ ## x
#endif

/* Read-only data macro. */
#if defined(__APPLE__)
#define RODATA() \
	.const_data
#elif defined(_WIN32)
#define RODATA() \
	.section	.rdata
#else
#define RODATA() \
	.section	.rodata
#endif

/* Symbol types. */
#if defined(__ELF__)
	#define SYMTYPE(sym, symtype) \
	.type sym, ## symtype
#else
	#define SYMTYPE(sym, symtype)
#endif

/* Symbol sizes. */
/* SYMSIZE_DATA() can be used anywhere. */
/* SYMSIZE_FUNC() should be used directly after a function. */
#if defined(__ELF__)
	#define SYMSIZE_DATA(sym, sym_size) \
	.size	sym, sym_size
	#define SYMSIZE_FUNC(sym) \
	.size	sym, .-sym
#else
	#define SYMSIZE_DATA(sym, sym_size)
	#define SYMSIZE_FUNC(sym)
#endif

/* PIC macros from http://www.gentoo.org/proj/en/hardened/pic-fix-guide.xml */

#ifdef __PIC__
	/* Position-independent code. */
	#undef __i686			/* gcc builtin define gets in our way */
	
#ifndef __APPLE__
	/* Linux version */
	#define MUNG_LOCAL(sym)		sym ## @GOTOFF(%ebx)
	#define MUNG_EXTERN(sym)	sym ## @GOT(%ebx)
	#define DEREF_EXTERN(reg)	movl (reg), reg
	#define INIT_PIC() \
		call __i686.get_pc_thunk.bx ; \
		addl $_GLOBAL_OFFSET_TABLE_, %ebx
	
/* __i686.get_pc_thunk.bx - retrieves the current PC in %ebx */
.section .gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
.globl __i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx,@function
__i686.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
SYMSIZE_FUNC(__i686.get_pc_thunk.bx)

#else /* __APPLE__ */
	/* MacOS X version */
	#define MUNG_LOCAL(sym)		sym-9b(%ebx)
	#define MUNG_EXTERN(sym)	/* TODO */
	#define DEREF_EXTERN(reg)	movl (reg), reg
	#define INIT_PIC() \
		call	9f;	\
9:		popl	%ebx;

#endif /* __APPLE__ */

#else
	/* Non-PIC code. */
	#define MUNG_LOCAL(sym)   sym
	#define MUNG_EXTERN(sym)  sym
	#define DEREF_EXTERN(reg)
	#define INIT_PIC()
#endif

#endif /* _MDP_GNU_AS_X86_INC */
