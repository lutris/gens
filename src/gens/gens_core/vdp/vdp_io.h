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

// System status.
extern int Genesis_Started;
extern int SegaCD_Started;
extern int _32X_Started;

typedef struct
{
	union
	{
		uint8_t reg[24];
		struct
		{
			uint8_t Set1;
			uint8_t Set2;
			uint8_t Pat_ScrA_Adr;
			uint8_t Pat_Win_Adr;
			uint8_t Pat_ScrB_Adr;
			uint8_t Spr_Att_Adr;
			uint8_t Reg6;
			uint8_t BG_Color;
			uint8_t Reg8;
			uint8_t Reg9;
			uint8_t H_Int;
			uint8_t Set3;
			uint8_t Set4;
			uint8_t H_Scr_Adr;
			uint8_t Reg14;
			uint8_t Auto_Inc;
			uint8_t Scr_Size;
			uint8_t Win_H_Pos;
			uint8_t Win_V_Pos;
			uint8_t DMA_Length_L;
			uint8_t DMA_Length_H;
			uint8_t DMA_Src_Adr_L;
			uint8_t DMA_Src_Adr_M;
			uint8_t DMA_Src_Adr_H;
		};
	};
	
	// These two variables are internal to Gens.
	// They don't map to any actual VDP registers.
	unsigned int DMA_Length;
	unsigned int DMA_Address;
} Reg_VDP_Type;

extern union
{
	uint8_t  u8[64*1024];
	uint16_t u16[(64*1024)>>1];
	uint32_t u32[(64*1024)>>2];
} VRam;
extern union
{
	uint8_t  u8[64<<1];
	uint16_t u16[64];
	uint32_t u32[64>>1];
} CRam;
extern uint8_t  VSRam[256];
extern uint32_t VSRam_Over[8];
extern uint8_t  H_Counter_Table[512][2];

extern Reg_VDP_Type VDP_Reg;
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

typedef union
{
	int reg[7];
	uint32_t ureg[7];
	struct
	{
		int Flag;
		int Data;
		int Write;
		int Access;
		int Address;
		int DMA_Mode;
		int DMA;
	};
} VDP_Ctrl_t;
extern VDP_Ctrl_t Ctrl;

// Set this to 1 to enable zero-length DMA requests.
// Default is 0. (hardware-accurate)
extern int Zero_Length_DMA;

void     VDP_Reset(void);
uint8_t  VDP_Read_H_Counter(void);
uint8_t  VDP_Read_V_Counter(void);
uint16_t VDP_Read_Status(void);

/* Functions that need to be ported to C. */
uint16_t Read_VDP_Data(void);
int Write_Byte_VDP_Data(uint8_t  Data);
int Write_Word_VDP_Data(uint16_t Data);
int Write_VDP_Ctrl(uint16_t Data);
void VDP_Set_Reg(int reg_num, uint8_t val);
unsigned int Update_DMA(void);

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
