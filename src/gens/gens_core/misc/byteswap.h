/**
 * GENS: Byteswapping functions.
 * These functions were ported from x86 assembler to C,
 * since they don't really need assembly optimizations.
 */


#ifndef BYTESWAP_H
#define BYTESWAP_H

#ifdef __cplusplus
extern "C" {
#endif


// Endianness defines ported from libsdl.
#define GENS_LIL_ENDIAN 1234
#define GENS_BIG_ENDIAN 4321
#ifndef GENS_BYTEORDER
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MIPSEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__SPARC__)
#define GENS_BYTEORDER GENS_BIG_ENDIAN
#else
#define GENS_BYTEORDER GENS_LIL_ENDIAN
#endif
#endif


// 16-bit byteswap function.
void __byte_swap_16(void *ptr, int n);
void __byte_swap_16_deprecated(void *ptr, int n);

#if GENS_BYTEORDER == GENS_LIL_ENDIAN
#define be16_to_cpu_array(ptr, n) __byte_swap_16((ptr), (n));
#define le16_to_cpu_array(ptr, n)
#define cpu_to_be16_array(ptr, n) __byte_swap_16((ptr), (n));
#define cpu_to_le16_array(ptr, n)
#else
#define be16_to_cpu_array(ptr, n)
#define le16_to_cpu_array(ptr, n) __byte_swap_16((ptr), (n));
#define cpu_to_be16_array(ptr, n)
#define cpu_to_le16_array(ptr, n) __byte_swap_16((ptr), (n));
#endif


#ifdef __cplusplus
}
#endif

#endif
