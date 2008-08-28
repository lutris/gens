#include "vdp_rend.h"
#include "blit.h"

/**
 * Blit_Scale2x(): Blits the image to the screen, scaled to 2x its normal size (without filtering).
 * @param Dest Destination buffer.
 * @param pitch Number of bytes per line.
 * @param x X coordinate for the image.
 * @param y Y coordinate for the image.
 * @param offset ???
 */
void Blit_Scale2x (unsigned char *Dest, int pitch, int x, int y, int offset) 
{
	int i, j;
	int B, D, E, F, H;
	int E0, E1, E2, E3;
	
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
			Dest[DstOffs] = E0;
			Dest[DstOffs + 1] = E0 >> 8;
			
			// E1
			Dest[DstOffs + 2] = E1;
			Dest[DstOffs + 3] = E1 >> 8;
			
			// E2
			Dest[DstOffs + pitch] = E2;
			Dest[DstOffs + pitch + 1] = E2 >> 8;
			
			// E3
			Dest[DstOffs + pitch + 2] = E3;
			Dest[DstOffs + pitch + 3] = E3 >> 8;
			
			DstOffs += 4;
		}
		
		SrcOffs += ScrAdd;
	}
}
