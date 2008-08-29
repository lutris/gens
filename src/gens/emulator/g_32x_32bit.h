/**
 * GENS: Sega 32X VDP - 32-bit color macros
 * Provided by Upth, ported from Gens Rerecording
 */


// TODO: Clean up these macros.

// NOTE: The MD_Screen lines are commented out because they mess with 15/16-bit mode.
// TODO: 32-bit mode is still broken.


#define POST_LINE_32X_M00 \
{ \
	unsigned int Pixel; \
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < TAB336[VDP_Current_Line] + 336; Pixel++) \
	{ \
		MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
		/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
	} \
}


#define POST_LINE_32X_M01 \
{ \
	unsigned int Pixel; \
	unsigned char pixC; \
	unsigned short pixS; \
	VRam_Ind *= 2; \
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++) \
	{ \
		pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1]; \
		pixS = _32X_VDP_CRam[pixC]; \
		if ((pixS & 0x8000) || !(MD_Screen[Pixel] & 0x0F)) \
		{ \
			/*MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];*/ \
			MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC]; \
		} \
		else \
		{ \
			MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
			/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
		} \
	} \
}


#define POST_LINE_32X_M10 \
{ \
	unsigned int Pixel; \
	unsigned short pix; \
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < TAB336[VDP_Current_Line] + 336; Pixel++) \
	{ \
		pix = VRam_32X[VRam_Ind++]; \
		if ((pix & 0x8000) || !(MD_Screen[Pixel] & 0x0F)) \
		{ \
			MD_Screen32[Pixel] = _32X_Palette_32B[pix]; \
			/*MD_Screen[Pixel] = _32X_Palette_16B[pix];*/ \
		} \
		else \
		{ \
			MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
			/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
		} \
	} \
}


#define POST_LINE_32X_M11 \
{ \
	unsigned char Pix; \
	unsigned char Num; \
	int EndPixel; \
	int CurPixel = TAB336[VDP_Current_Line] + 8; \
	int PixMax = TAB336[VDP_Current_Line] + 336; \
	while (CurPixel < PixMax) \
	{ \
		Pix = VRam_32X[VRam_Ind] & 0xFF; \
		Num = VRam_32X[VRam_Ind++] >> 8; \
		EndPixel = CurPixel + Num; \
		while (CurPixel <= EndPixel) \
		{ \
			/*MD_Screen[CurPixel] = _32X_VDP_CRam_Ajusted[Pix];*/ \
			MD_Screen32[CurPixel++] = _32X_VDP_CRam_Ajusted32[Pix]; \
		} \
	} \
}


#define POST_LINE_32X_SM01 \
{ \
	unsigned int Pixel; \
	unsigned char pixC; \
	unsigned short pixS; \
	VRam_Ind *= 2; \
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++) \
	{ \
		pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1]; \
		pixS = _32X_VDP_CRam[pixC]; \
		if ((pixS & 0x8000) || !(MD_Screen[Pixel] & 0x0F)) \
		{ \
			/*MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];*/ \
			MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC]; \
		} \
		else \
		{ \
			MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
			/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
		} \
	} \
}


#define POST_LINE_32X_M01_P \
{ \
	unsigned int Pixel; \
	unsigned char pixC; \
	unsigned short pixS; \
	VRam_Ind *= 2;\
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++) \
	{ \
		pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1]; \
		pixS = _32X_VDP_CRam[pixC]; \
		if ((pixS & 0x8000) && (MD_Screen[Pixel] & 0x0F)) \
		{ \
			MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
			/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
		} \
		else \
		{ \
			/*MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];*/ \
			MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC]; \
		} \
	} \
}


#define POST_LINE_32X_M10_P \
{ \
	unsigned int Pixel; \
	unsigned short pix; \
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < TAB336[VDP_Current_Line] + 336; Pixel++) \
	{ \
		pix = VRam_32X[VRam_Ind++]; \
		if (!(pix & 0x8000) && (MD_Screen[Pixel] & 0x0F)) \
		{ \
			MD_Screen32[Pixel] = _32X_Palette_32B[pix]; \
			/*MD_Screen[Pixel] = _32X_Palette_16B[pix];*/ \
		} \
		else \
		{ \
			MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
			/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
		} \
	} \
}


#define POST_LINE_32X_SM01_P \
{ \
	unsigned int Pixel; \
	unsigned char pixC; \
	unsigned short pixS; \
	VRam_Ind *= 2; \
	for (Pixel = TAB336[VDP_Current_Line] + 8; Pixel < (TAB336[VDP_Current_Line] + 336); Pixel++) \
	{ \
		pixC = _32X_VDP_Ram[VRam_Ind++ ^ 1]; \
		pixS = _32X_VDP_CRam[pixC]; \
		if ((pixS & 0x8000) && (MD_Screen[Pixel] & 0x0F)) \
		{ \
			MD_Screen32[Pixel] = MD_Palette32[MD_Screen[Pixel] & 0xFF]; \
			/*MD_Screen[Pixel] = MD_Palette[MD_Screen[Pixel] & 0xFF];*/ \
		} \
		else \
		{ \
			/*MD_Screen[Pixel] = _32X_VDP_CRam_Ajusted[pixC];*/ \
			MD_Screen32[Pixel] = _32X_VDP_CRam_Ajusted32[pixC]; \
		} \
	} \
}


#define POST_LINE_32X \
{ \
	unsigned short *VRam_32X = (unsigned short *) &_32X_VDP_Ram; \
	int VRam_Ind = ((_32X_VDP.State & 1) << 16); \
	VRam_Ind += VRam_32X[VRam_Ind + VDP_Current_Line]; \
	\
	switch (_32X_Rend_Mode) \
	{ \
		case 0: \
		case 4: \
		case 8: \
		case 12: \
			POST_LINE_32X_M00; \
			break; \
		case 1: \
			POST_LINE_32X_M01; \
			break; \
		case 2: \
		case 10: \
			POST_LINE_32X_M10; \
			break; \
		case 3: \
		case 7: \
		case 11: \
		case 15: \
			POST_LINE_32X_M11; \
			break; \
		case 5: \
			POST_LINE_32X_M01_P; \
			break; \
		case 6: \
		case 14: \
			POST_LINE_32X_M10_P; \
			break; \
		case 9: \
			POST_LINE_32X_SM01; \
			break; \
		case 13: \
			POST_LINE_32X_SM01_P; \
			break; \
	} \
}
