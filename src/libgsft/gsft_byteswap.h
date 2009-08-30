/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_byteswap.h: Byteswapping functions.                                *
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

#ifndef __GSFT_BYTESWAP_H
#define __GSFT_BYTESWAP_H

#include <stdint.h>

// Endianness defines ported from libsdl.
#define GSFT_LIL_ENDIAN 1234
#define GSFT_BIG_ENDIAN 4321
#ifndef GSFT_BYTEORDER
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__SPARC__)
#define GSFT_BYTEORDER GSFT_BIG_ENDIAN
#else
#define GSFT_BYTEORDER GSFT_LIL_ENDIAN
#endif
#endif

// TODO: Optimize out ?e??_to_cpu_from_ptr on appropriate architectures.
#define be16_to_cpu_from_ptr(ptr) \
	(((ptr)[0] << 8) | ((ptr)[1]))
#define le16_to_cpu_from_ptr(ptr) \
	(((ptr)[1] << 8) | ((ptr)[0]))
#define be32_to_cpu_from_ptr(ptr) \
	(((ptr)[0] << 24) | ((ptr)[1] << 16) | ((ptr)[2] << 8) | ((ptr)[3]))
#define le32_to_cpu_from_ptr(ptr) \
	(((ptr)[3] << 24) | ((ptr)[2] << 16) | ((ptr)[1] << 8) | ((ptr)[0]))
#define cpu_to_le16_ucptr(ptr, val) \
	(ptr)[0] = ((val) & 0xFF);  \
	(ptr)[1] = (((val) >> 8) & 0xFF);
#define cpu_to_be16_ucptr(ptr, val) \
	(ptr)[0] = (((val) >> 8) & 0xFF);  \
	(ptr)[1] = ((val) & 0xFF);
#define cpu_to_le32_ucptr(ptr, val) \
	(ptr)[0] = ((val) & 0xFF);  \
	(ptr)[1] = (((val) >> 8) & 0xFF);  \
	(ptr)[2] = (((val) >> 16) & 0xFF); \
	(ptr)[3] = (((val) >> 24) & 0xFF);
#define cpu_to_be32_ucptr(ptr, val) \
	(ptr)[0] = (((val) >> 24) & 0xFF); \
	(ptr)[1] = (((val) >> 16) & 0xFF); \
	(ptr)[2] = (((val) >> 8) & 0xFF);  \
	(ptr)[3] = ((val) & 0xFF);

#define __swab16(x) (((x) << 8) | ((x) >> 8))

#define __swab32(x) \
	(((x) << 24) | ((x) >> 24) | \
		((x & 0x0000FF00UL) << 8) | \
		((x & 0x00FF0000UL) >> 8))

#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
	#define be16_to_cpu_array(ptr, n) __byte_swap_16_array((ptr), (n));
	#define le16_to_cpu_array(ptr, n)
	#define cpu_to_be16_array(ptr, n) __byte_swap_16_array((ptr), (n));
	#define cpu_to_le16_array(ptr, n)
	
	#define be16_to_cpu(x) __swab16(x)
	#define be32_to_cpu(x) __swab32(x)
	#define le16_to_cpu(x) (x)
	#define le32_to_cpu(x) (x)
	
	#define cpu_to_be16(x) __swab16(x)
	#define cpu_to_be32(x) __swab32(x)
	#define cpu_to_le16(x) (x)
	#define cpu_to_le32(x) (x)
#else /* GSFT_BYTEORDER == GSFT_BIG_ENDIAN */
	#define be16_to_cpu_array(ptr, n)
	#define le16_to_cpu_array(ptr, n) __byte_swap_16_array((ptr), (n));
	#define cpu_to_be16_array(ptr, n)
	#define cpu_to_le16_array(ptr, n) __byte_swap_16_array((ptr), (n));
	
	#define be16_to_cpu(x) (x)
	#define be32_to_cpu(x) (x)
	#define le16_to_cpu(x) __swab16(x)
	#define le32_to_cpu(x) __swab32(x)
	
	#define cpu_to_be16(x) (x)
	#define cpu_to_be32(x) (x)
	#define cpu_to_le16(x) __swab16(x)
	#define cpu_to_le32(x) __swab32(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 16-bit byteswap function.
void __byte_swap_16_array(void *ptr, unsigned int n);

#ifdef __cplusplus
}
#endif

#endif /* __GSFT_BYTESWAP_H */
