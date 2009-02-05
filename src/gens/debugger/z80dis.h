/***************************************************************************
 * Gens: Z80 disassembler.                                                 *
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

#ifndef GENS_Z80DIS_H
#define GENS_Z80DIS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUGGER

int z80dis(unsigned char *buf, unsigned int *Counter, char str[128]);


/* The Z80 logging function uses the FASTCALL calling convention. */

#ifndef __WIN32__

#ifndef __GNUC__

#define FASTCALL				__fastcall
#define DECL_FASTCALL(type, name)	type FASTCALL name

#else //__GNUC__
#define __fastcall __attribute__ ((regparm(2)))
#define FASTCALL __attribute__ ((regparm(2)))
#define DECL_FASTCALL(type, name)	type name __attribute__ ((regparm(2)))

#endif //!__GNUC__

#else /* __WIN32__ */

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif /* FASTCALL */

#ifndef DECL_FASTCALL
#define DECL_FASTCALL(type, name)	type FASTCALL name
#endif /* DECL_FASTCALL */

#endif /* __WIN32__ */

int FASTCALL z80log(unsigned int PC);

#endif /* GENS_DEBUG */

#ifdef __cplusplus
}
#endif /* GENS_DEBUGGER */

#endif /* GENS_Z80DIS_H */
