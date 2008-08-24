/**
 * GENS: Byteswapping functions.
 * These functions were ported from x86 assembler to C,
 * since they don't really need assembly optimizations.
 */


/**
 * __byte_swap_16(): Swap bytes in 16-bit chunks.
 * @param ptr Pointer to bytes.
 * @param n Number of bytes to swap.
 */
void __byte_swap_16(void *ptr, int n)
{
	int i;
	unsigned char x;
	unsigned char *cptr;
	cptr = (unsigned char*)ptr;
	
	for (i = 0; i < n; i += 2)
	{
		x = cptr[i];
		cptr[i] = cptr[i + 1];
		cptr[i + 1] = x;
	}
}
