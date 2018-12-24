#ifndef GENS_VDP_32X_H
#define GENS_VDP_32X_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VDP_32X_t
{
	unsigned int Mode;
	unsigned int State;
	unsigned int AF_Data;
	unsigned int AF_St;
	unsigned int AF_Len;
	unsigned int AF_Line;
} VDP_32X_t;

// VDP RAM and CRam.
extern uint8_t  _32X_VDP_Ram[0x100 * 1024];
extern uint16_t  _32X_VDP_CRam[0x100];

// Full 32X palettes.
extern uint16_t _32X_Palette_16B[0x10000];
extern uint32_t _32X_Palette_32B[0x10000];

// Adjusted CRam.
extern uint16_t _32X_VDP_CRam_Adjusted[0x100];
extern uint32_t _32X_VDP_CRam_Adjusted32[0x100];

extern VDP_32X_t _32X_VDP;

void _32X_VDP_Reset(void);
void _32X_VDP_Draw(int FB_Num);

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDP_32X_H */
