/***************************************************************************
 * MDP: Mega Drive Plugins - Function Call Definitions.                    *
 *                                                                         *
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

#ifndef __MDP_FNCALL_H
#define __MDP_FNCALL_H

/* Packed struct define. */
#ifndef PACKED
	#if defined(__GNUC__)
		#define PACKED __attribute__ ((packed))
	#else
		#define PACKED
	#endif
#endif

/**
 * Function pointer calling conventions from SDL's begin_code.h
 */
#ifndef MDP_FNCALL
	#if defined(__WIN32__) && !defined(__GNUC__)
		#define MDP_FNCALL __cdecl
	#elif defined(__OS2__)
		#define MDP_FNCALL _System
	#else
		#define MDP_FNCALL
	#endif
#endif

/**
 * DLL import/export.
 * See http://gcc.gnu.org/wiki/Visibility
 */
#if defined(_WIN32) || defined(__CYGWIN__)
	#if defined(BUILDING_DLL) || defined(DLL_EXPORT)
		#ifdef __GNUC__
			#define DLL_PUBLIC __attribute__ ((dllexport))
		#else
			#define DLL_PUBLIC __declspec(dllexport)
		#endif
	#else
		#ifdef __GNUC__
			#define DLL_PUBLIC __attribute__ ((dllimport))
		#else
			#define DLL_PUBLIC __declspec(dllimport)
		#endif
	#endif
	#define DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define DLL_PUBLIC __attribute__ ((visibility("default")))
		#define DLL_LOCAL  __attribute__ ((visibility("hidden")))
	#else
		#define DLL_PUBLIC
		#define DLL_LOCAL
	#endif
#endif

/* Endianness defines ported from libsdl. */
#define MDP_LIL_ENDIAN 1234
#define MDP_BIG_ENDIAN 4321
#ifndef MDP_BYTEORDER
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__SPARC__)
#define MDP_BYTEORDER MDP_BIG_ENDIAN
#else
#define MDP_BYTEORDER MDP_LIL_ENDIAN
#endif
#endif

/**
 * A macro to suppress some compilers' "Parameter is not used" warnings.
 * Copied from Subversion.
 */
#define MDP_UNUSED_PARAMETER(x) ((void)x)
/* Possible alternative:
 *
 * #define MDP_UNUSED_PARAMETER(x) ((x) = (x))
 */

#endif /* __MDP_FNCALL_H */
