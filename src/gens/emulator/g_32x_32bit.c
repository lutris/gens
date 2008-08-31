/**
 * GENS: Sega 32X VDP - 32-bit color functions.
 * Provided by Upth, ported from Gens Rerecording
 */


// Post_Line_32X() was originally a series of macros.
// I converted it to a function so it can be debugged easier.


// TODO: Clean up these functions.

// TODO: Maybe split up Post_Line_32X() using inline functions?


#include "g_32x_32bit.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"


/**
 * Post_Line_32X(): Process a 32X video line.
 */
void Post_Line_32X(void)
{
	unsigned short *VRam_32X = (unsigned short *) &_32X_VDP_Ram;
	int VRam_Ind = ((_32X_VDP.State & 1) << 16);
	VRam_Ind += VRam_32X[VRam_Ind + VDP_Current_Line];
	
	unsigned int Pixel;
	unsigned char pixC; 
	unsigned short pixS; 
	
	// Used for POST_LINE_32X_M11
	unsigned char Num;
	int EndPixel, CurPixel, PixMax;
	
	switch (_32X_Rend_Mode)
	{
		case 0:
		case 4:
		case 8:
		case 12:
			//POST_LINE_32X_M00;
			for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < TAB336[VDP_Current_Line] + 336; Pixel++)
			{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
			}
			
			break;
		
		case 1:
			//POST_LINE_32X_M01;
			VRam_Ind *= 2;
			for (Pixel = TAB336[VDP_Current_Line] + 8;
			     Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) || !(MD_Screen[Pixel] & 0x0F))
				{
					MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];
					MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC];
				}
				else
				{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
				}
			}
			
			break;
		
		case 2:
		case 10:
			//POST_LINE_32X_M10;
			for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < TAB336[VDP_Current_Line] + 336; Pixel++)
			{
				pixS = VRam_32X[VRam_Ind++];
				if ((pixS & 0x8000) || !(MD_Screen[Pixel] & 0x0F))
				{
					MD_Screen32[Pixel] = _32X_Palette_32B[pixS];
					MD_Screen[Pixel] = _32X_Palette_16B[pixS];
				}
				else
				{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
				}
			}
			break;
		
		case 3:
		case 7:
		case 11:
		case 15:
			//POST_LINE_32X_M11;
			CurPixel = TAB336[VDP_Current_Line] + 8;
			PixMax = TAB336[VDP_Current_Line] + 336;
			while (CurPixel < PixMax)
			{
				pixC = VRam_32X[VRam_Ind] & 0xFF;
				Num = VRam_32X[VRam_Ind++] >> 8;
				EndPixel = CurPixel + Num;
				while (CurPixel <= EndPixel)
				{
					MD_Screen[CurPixel] = _32X_VDP_CRam_Ajusted[pixC];
					MD_Screen32[CurPixel++] = _32X_VDP_CRam_Ajusted32[pixC];
				}
			}
			break;
		
		case 5:
			//POST_LINE_32X_M01_P;
			VRam_Ind *= 2;
			for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) && (MD_Screen[Pixel] & 0x0F))
				{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
				}
				else
				{
					MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];
					MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC];
				}
			}
			break;
		
		case 6:
		case 14:
			//POST_LINE_32X_M10_P;
			for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < TAB336[VDP_Current_Line] + 336; Pixel++)
			{
				pixS = VRam_32X[VRam_Ind++];
				if (!(pixS & 0x8000) && (MD_Screen[Pixel] & 0x0F))
				{
					MD_Screen32[Pixel] = _32X_Palette_32B[pixS];
					MD_Screen[Pixel] = _32X_Palette_16B[pixS];
				}
				else
				{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
				}
			}
			break;
		
		case 9:
			//POST_LINE_32X_SM01;
			VRam_Ind *= 2;
			for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) || !(MD_Screen[Pixel] & 0x0F))
				{
					MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];
					MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC];
				}
				else
				{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
				}
			}
			break;
		
		case 13:
			//POST_LINE_32X_SM01_P;
			VRam_Ind *= 2;
			for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++)
			{
				pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1];
				pixS = _32X_VDP_CRam[pixC];
				if ((pixS & 0x8000) && (MD_Screen[Pixel] & 0x0F))
				{
					MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF];
					MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];
				}
				else
				{
					MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];
					MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC];
				}
			}
			break;
	}
}
