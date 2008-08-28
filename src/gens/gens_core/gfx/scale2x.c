#include "vdp_rend.h"
#include "blit.h"
#include "byteswap.h"


extern unsigned char Bits32;


static void Blit_Scale2x_16(unsigned char *Dest, int pitch, int x, int y, int offset);
static void Blit_Scale2x_32(unsigned char *Dest, int pitch, int x, int y, int offset);


/**
 * Blit_Scale2x(): Blits the image to the screen, scaled to 2x its normal size (without filtering).
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
void Blit_Scale2x(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	if (Bits32)
		Blit_Scale2x_32(Dest, pitch, x, y, offset);
	else
		Blit_Scale2x_16(Dest, pitch, x, y, offset);
}


/**
 * Blit_Scale2x_16(): (16-bit) Blits the image to the screen, scaled to 2x its normal size (without filtering).
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_Scale2x_16(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	int i, j;
	unsigned short B, D, E, F, H;
	unsigned short E0, E1, E2, E3;
	
	// A B C
	// D E F
	// G H I
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		E = MD_Screen[SrcOffs - 1];
		F = MD_Screen[SrcOffs];
		
		DstOffs = i * pitch * 2;
		
		for (j = 0; j < x; j++)
		{
			D = E; E = F;
			B = MD_Screen[SrcOffs - ScrWdth];
			H = MD_Screen[SrcOffs + ScrWdth];
			F = MD_Screen[++SrcOffs];
			E0 = D == B && B != F && D != H ? D : E;
			E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;
			
			// E0
			Dest[DstOffs + 0] = (unsigned char)(E0 & 0xFF);
			Dest[DstOffs + 1] = (unsigned char)(E0 >> 8);
			
			// E1
			Dest[DstOffs + 2] = (unsigned char)(E1 & 0xFF);
			Dest[DstOffs + 3] = (unsigned char)(E1 >> 8);
			
			// E2
			Dest[DstOffs + pitch + 0] = (unsigned char)(E2 & 0xFF);
			Dest[DstOffs + pitch + 1] = (unsigned char)(E2 >> 8);
			
			// E3
			Dest[DstOffs + pitch + 2] = (unsigned char)(E3 & 0xFF);
			Dest[DstOffs + pitch + 3] = (unsigned char)(E3 >> 8);
			
			DstOffs += 4;
		}
		
		SrcOffs += ScrAdd;
	}
}


/**
 * Blit_Scale2x_32(): (32-bit) Blits the image to the screen, scaled to 2x its normal size (without filtering).
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
static void Blit_Scale2x_32(unsigned char *Dest, int pitch, int x, int y, int offset)
{
	int i, j;
	unsigned int B, D, E, F, H;
	unsigned int E0, E1, E2, E3;
	
	// A B C
	// D E F
	// G H I
	
	int ScrWdth, ScrAdd;
	int SrcOffs, DstOffs;
	
	ScrAdd = offset >> 1;
	ScrWdth = x + ScrAdd;
	
	SrcOffs = 8;
	DstOffs = 0;
	
	for (i = 0; i < y; i++)
	{
		E = MD_Screen32[SrcOffs - 1];
		F = MD_Screen32[SrcOffs];
		
		DstOffs = i * pitch * 2;
		
		for (j = 0; j < x; j++)
		{
			D = E; E = F;
			B = MD_Screen32[SrcOffs - ScrWdth];
			H = MD_Screen32[SrcOffs + ScrWdth];
			F = MD_Screen32[++SrcOffs];
			E0 = D == B && B != F && D != H ? D : E;
			E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;
			
			cpu_to_le32_ucptr(&Dest[DstOffs], E0);
			cpu_to_le32_ucptr(&Dest[DstOffs + 4], E1);
			cpu_to_le32_ucptr(&Dest[DstOffs + pitch + 0], E2);
			cpu_to_le32_ucptr(&Dest[DstOffs + pitch + 4], E3);
			
			DstOffs += 8;
		}
		
		SrcOffs += ScrAdd;
	}
}
