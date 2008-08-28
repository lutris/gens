#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vdp_rend.h"
#include "blit.h"
#include "byteswap.h"
#include <string.h>


extern unsigned char Bits32;


static void Blit_1x_16(unsigned char *Dest, int pitch, int x, int y, int offset);
static void Blit_1x_32(unsigned char *Dest, int pitch, int x, int y, int offset);


/**
 * Blit_1x(): Blits the image to the screen, with no scaling.
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
// TODO: Return a function pointer so this can be set in ui_proxy.c:Set_Render().
// That will reduce function call overhead.
void Blit_1x(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	if (Bits32)
		Blit_1x_32(Dest, pitch, x, y, offset);
	else
	{
#ifdef GENS_X86_ASM
		if (Have_MMX)
			Blit_1x_16_asm_MMX(Dest, pitch, x, y, offset);
		else
			Blit_1x_16_asm(Dest, pitch, x, y, offset);
#else
		Blit_1x_16(Dest, pitch, x, y, offset);
#endif
	}
}


/**
 * Blit_1x_16(): (16-bit) Blits the image to the screen, with no scaling.
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_1x_16(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	int i;
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		//DstOffs = i * pitch * 2;
		DstOffs = i * pitch;
		memcpy(&Dest[DstOffs], &MD_Screen[SrcOffs], x * 2);
		
		// Next line.
		// TODO: Make this a constant somewhere.
		SrcOffs += 336;
	}
}


/**
 * Blit_1x_32(): (32-bit) Blits the image to the screen, with no scaling.
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_1x_32(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	int i;
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		//DstOffs = i * pitch * 2;
		DstOffs = i * pitch;
		memcpy(&Dest[DstOffs], &MD_Screen32[SrcOffs], x * 4);
		
		// Next line.
		// TODO: Make this a constant somewhere.
		SrcOffs += 336;
	}
}
