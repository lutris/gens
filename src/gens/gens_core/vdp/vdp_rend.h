#ifndef GENS_VDP_REND_H
#define GENS_VDP_REND_H

#ifdef __cplusplus
extern "C" {
#endif

// 15/16-bit color
extern unsigned short MD_Screen[336 * 240];
extern unsigned short Palette[0x1000];
extern unsigned short MD_Palette[256];

// 32-bit color
extern unsigned int MD_Screen32[336 * 240];
extern unsigned int Palette32[0x1000];
extern unsigned int MD_Palette32[256];

// _32X_Rend_Mode is used for the 32X 32-bit color C functions.
// See g_32x_32bit.h
extern unsigned char _32X_Rend_Mode;

// Debugging stuff
// TODO: Port all debugging stuff from Gens Rerecording
extern char PalLock;

extern unsigned long TAB336[336];

extern struct
{
	int Pos_X;
	int Pos_Y;
	unsigned int Size_X;
	unsigned int Size_Y;
	int Pos_X_Max;
	int Pos_Y_Max;
	unsigned int Num_Tile;
	int dirt;
} Sprite_Struct[256];

extern int Sprite_Over;

// Color depth
extern unsigned char bpp;

void Render_Line();
void Render_Line_32X();

#ifdef __cplusplus
}
#endif

#endif
