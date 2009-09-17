#ifndef GENS_VDP_IO_H
#define GENS_VDP_IO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

// Needed for inline functions.
#include "gens_core/mem/mem_m68k.h"
#include "util/file/rom.hpp"
#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Constant data.
extern const uint32_t CD_Table[64];
extern const uint8_t  DMA_Timing_Table[16];
extern const uint32_t Size_V_Scroll[4];
extern const uint32_t H_Scroll_Mask_Table[4];

// System status.
extern int Genesis_Started;
extern int SegaCD_Started;
extern int _32X_Started;

struct Reg_VDP_Type
{
	unsigned int Set1;
	unsigned int Set2;
	unsigned int Pat_ScrA_Adr;
	unsigned int Pat_Win_Adr;
	unsigned int Pat_ScrB_Adr;
	unsigned int Spr_Att_Adr;
	unsigned int Reg6;
	unsigned int BG_Color;
	unsigned int Reg8;
	unsigned int Reg9;
	unsigned int H_Int;
	unsigned int Set3;
	unsigned int Set4;
	unsigned int H_Scr_Adr;
	unsigned int Reg14;
	unsigned int Auto_Inc;
	unsigned int Scr_Size;
	unsigned int Win_H_Pos;
	unsigned int Win_V_Pos;
	unsigned int DMA_Length_L;
	unsigned int DMA_Length_H;
	unsigned int DMA_Src_Adr_L;
	unsigned int DMA_Src_Adr_M;
	unsigned int DMA_Src_Adr_H;
	unsigned int DMA_Length;
	unsigned int DMA_Address;
};

extern uint8_t  VRam[64 * 1024];
extern uint16_t CRam[64];
extern uint8_t  VSRam[256];
extern uint32_t VSRam_Over[8];
extern uint8_t  H_Counter_Table[512][2];

extern struct Reg_VDP_Type VDP_Reg;
extern int VDP_Current_Line;
extern int VDP_Num_Lines;
extern int VDP_Num_Vis_Lines;
extern int CRam_Flag;
extern int VRam_Flag;
extern int VDP_Int;
extern int VDP_Status;
extern int DMAT_Length;
extern int DMAT_Type;
extern int DMAT_Tmp;
extern struct 
{
	int Flag;
	int Data;
	int Write;
	int Access;
	int Address;
	int DMA_Mode;
	int DMA;
} Ctrl;

// Set this to 1 to enable zero-length DMA requests.
// Default is 0. (hardware-accurate)
extern int Zero_Length_DMA;

void VDP_Reset(void);
unsigned int Update_DMA(void);
uint16_t Read_VDP_Data(void);
uint16_t Read_VDP_Status(void);
uint8_t  Read_VDP_H_Counter(void);
uint8_t  Read_VDP_V_Counter(void);
int Write_Byte_VDP_Data(uint8_t  Data);
int Write_Word_VDP_Data(uint16_t Data);
int Write_VDP_Ctrl(uint16_t Data);
int Set_VDP_Reg(int Num_Reg, uint8_t Val);

uint8_t VDP_Int_Ack(void);
void VDP_Update_IRQ_Line(void);


/** Inline VDP functions. **/

/**
 * vdp_isH40(): Determine if the current horiontal resolution is H40.
 * @return Zero if H32 (256); non-zero if H40 (320).
 */
static inline int vdp_isH40(void)
{
	// Default when no game is loaded is 1. (320x224)
	int rval = 0;
	if ((VDP_Reg.Set4 & 0x1) || !Game)
		rval = 1;
#ifdef GENS_DEBUGGER
	if (debug_mode != DEBUG_NONE)
		rval = 1;
#endif
#if 0
	if (!FrameCount)
		rval = 1;
#endif
	return rval;
}

/**
 * vdp_isH40(): Determine if the current horiontal resolution is H40.
 * @return Zero if V28 (224); non-zero if V30 (240).
 */
static inline int vdp_isV30(void)
{
	// Default when no game is loaded is 0. (320x224)
	int rval = 0;
	if (((CPU_Mode == 1) && (VDP_Reg.Set2 & 0x8)) && Game)
		rval = 1;
#ifdef GENS_DEBUGGER
	if (debug_mode != DEBUG_NONE)
		rval = 1;
#endif
#if 0
	if (!FrameCount)
		rval = 1;
#endif
	return rval;
}

#ifdef __cplusplus
}
#endif

/** VDP macros. **/

/**
 * VDP_SET_VISIBLE_LINES(): Sets the number of visible lines, depending on CPU mode and VDP setting.
 * Possible options:
 * - Normal: 224 lines. (V28)
 * - If PAL and Set2.M2 (bit 3) is set, 240 lines. (V30)
 */
#define VDP_SET_VISIBLE_LINES()				\
do {							\
	if ((CPU_Mode == 1) && (VDP_Reg.Set2 & 0x8))	\
		VDP_Num_Vis_Lines = 240;		\
	else						\
		VDP_Num_Vis_Lines = 224;		\
} while (0)

#endif /* GENS_VDP_IO_H */
