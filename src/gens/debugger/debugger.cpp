#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "emulator/g_main.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_32x.hpp"
#include "ui/gens_ui.hpp"

// Print_Text() function.
#include "gens_core/misc/print_text.hpp"

#include "debugger.hpp"

// MC68000
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "m68kd.h"

// Z80
#include "gens_core/cpu/z80/cpu_z80.h"
#include "mdZ80/mdZ80.h"
#include "gens_core/mem/mem_z80.h"
#include "z80dis.h"

// SH2
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/mem/mem_sh2.h"
#include "sh2d.h"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// SegaCD
#include "segacd/lc89510.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"
#include "mcd_cell_dump.h"

// Input functions.
#include "input/input.h"


static int Current_32X_FB = 0;
static int adr_mem = 0, pattern_pal = 0;
static int Current_PC;

// MD VDP pattern address.
#define MD_VDP_PATTERN_MAX		0xD000
#define MD_VDP_PATTERN_NORMAL_INC	0x200
#define MD_VDP_PATTERN_NORMAL_MASK	0x1FF
#define MD_VDP_PATTERN_INTERLACED_INC	0x400
#define MD_VDP_PATTERN_INTERLACED_MASK	0x3FF
static int pattern_adr = 0;
static int pattern_inc;
static int pattern_mask;

// SegaCD Word RAM pattern address.
#define SEGACD_WRAM_PATTERN_MAX		0x3D000
#define SEGACD_WRAM_PATTERN_16x16_INC	0x400
#define SEGACD_WRAM_PATTERN_16x16_MASK	0x3FF
#define SEGACD_WRAM_PATTERN_32x32_INC	0x800
#define SEGACD_WRAM_PATTERN_32x32_MASK	0x7FF
static int cd_pattern_adr = 0;
static int cd_pattern_inc;
static int cd_pattern_mask;

static char Dbg_Out_Str[GENS_PATH_MAX];

// Debug mode settings.
DEBUG_MODE debug_mode = DEBUG_NONE;
int debug_show_vdp = 0;


/**
 * Debug_Event(): Key pressed while debugging.
 * @param key Keycode.
 * @param mod Modifiers.
 */
void Debug_Event(int key, int mod)
{
	int i, steps;
	SH2_CONTEXT *sh;
	// Determine the appropriate SH2 context.
	if (debug_mode == DEBUG_MAIN_SH2 || debug_mode == DEBUG_32X_VDP)
		sh = &M_SH2;
	else if (debug_mode == DEBUG_SUB_SH2)
		sh = &S_SH2;
	else
		sh = NULL;
	
	switch (key)
	{
		case GENS_KEY_t:
		case GENS_KEY_y:
		case GENS_KEY_u:
		case GENS_KEY_i:
		case GENS_KEY_o:
		case GENS_KEY_p:
			// Trip Odometer (run code?)
			// T = 1; Y = 10; U = 100; I = 1,000; O = 10,000; P = 100,000
			steps = 0;
			switch (key)
			{
				case GENS_KEY_t:
					steps = 1;
					break;
				case GENS_KEY_y:
					steps = 10;
					break;
				case GENS_KEY_u:
					steps = 100;
					break;
				case GENS_KEY_i:
					steps = 1000;
					break;
				case GENS_KEY_o:
					steps = 10000;
					break;
				case GENS_KEY_p:
					steps = 100000;
					break;
			}
			
			for (i = 0; i < steps; i++)
			{
				if ((debug_mode == DEBUG_MAIN_68000) || (debug_mode == DEBUG_GENESIS_VDP))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (debug_mode == DEBUG_Z80)
				{
					mdZ80_clear_odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if (debug_mode == DEBUG_SUB_68000_REG ||
					 debug_mode == DEBUG_SUB_68000_CDC ||
					 debug_mode == DEBUG_WORD_RAM_PATTERN)
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if (debug_mode == DEBUG_MAIN_SH2 ||
					 debug_mode == DEBUG_SUB_SH2 ||
					 debug_mode == DEBUG_32X_VDP)
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			
			break;
		
		case GENS_KEY_z:
			adr_mem -= 0xC * 0xC * 0xC;
			break;
		
		case GENS_KEY_s:
			adr_mem += 0xC * 0xC * 0xC;
			break;
		
		case GENS_KEY_e:
			adr_mem -= 0xC * 0xC;
			break;
		
		case GENS_KEY_d:
			adr_mem += 0xC * 0xC;
			break;
		
		case GENS_KEY_r:
			adr_mem -= 0xC;
			break;
		
		case GENS_KEY_f:
			adr_mem += 0xC;
			break;
		
		case GENS_KEY_h:
			if (debug_mode == DEBUG_MAIN_68000)
			{
				main68k_interrupt(4, -1);
			}
			else if (debug_mode == DEBUG_Z80)
			{
				mdZ80_interrupt(&M_Z80, 0xFF);
			}
			else if (debug_mode == DEBUG_SUB_68000_REG ||
				 debug_mode == DEBUG_SUB_68000_CDC ||
				 debug_mode == DEBUG_WORD_RAM_PATTERN)
			{
				sub68k_interrupt(5, -1);
			}
			else if (debug_mode == DEBUG_MAIN_SH2 ||
				 debug_mode == DEBUG_SUB_SH2 ||
				 debug_mode == DEBUG_32X_VDP)
			{
				SH2_Interrupt(sh, 8);
			}
			
			break;
		
		case GENS_KEY_j:
			if (debug_mode == DEBUG_MAIN_68000)
			{
				main68k_interrupt(6, -1);
			}
			else if (debug_mode == DEBUG_Z80)
			{
				mdZ80_interrupt(&M_Z80, 0xFF);
			}
			else if (debug_mode == DEBUG_SUB_68000_REG ||
				 debug_mode == DEBUG_SUB_68000_CDC ||
				 debug_mode == DEBUG_WORD_RAM_PATTERN)
			{
				sub68k_interrupt(4, -1);
			}
			else if (debug_mode == DEBUG_MAIN_SH2 ||
				 debug_mode == DEBUG_SUB_SH2 ||
				 debug_mode == DEBUG_32X_VDP)
			{
				SH2_Interrupt(sh, 12);
			}
			
			break;
		
		case GENS_KEY_l:
			// Go up one VDP line.
			if (VDP_Lines.Display.Current > 0)
				VDP_Lines.Display.Current--;
			break;
		
		case GENS_KEY_m:
			// Go down one VDP line.
			if (VDP_Lines.Display.Current < (VDP_Lines.Display.Total - 1))
				VDP_Lines.Display.Current++;
			break;
		
		case GENS_KEY_x:
			debug_show_vdp = !debug_show_vdp;
			break;
		
		case GENS_KEY_c:
			if (debug_mode == DEBUG_SUB_68000_REG || debug_mode == DEBUG_SUB_68000_CDC)
				SCD.Cur_LBA++;
			VDP_Status ^= 0x8;
			break;
		
		case GENS_KEY_v:
			VDP_Status ^= 0x4;
			Current_32X_FB ^= 1;
			break;
		
		case GENS_KEY_n:
			if (debug_mode == DEBUG_MAIN_SH2 || debug_mode == DEBUG_SUB_SH2)
			{
				sh->PC += 2;
				sh->Status &= 0xFFFFFFF0;
			}
			else if (debug_mode == DEBUG_SUB_68000_REG || debug_mode == DEBUG_SUB_68000_CDC)
			{
				sub68k_context.pc += 2;
			}
			else if (debug_mode == DEBUG_MAIN_68000)
			{
				main68k_context.pc += 2;
			}
			else if (debug_mode == DEBUG_Z80)
			{
				mdZ80_set_PC(&M_Z80, mdZ80_get_PC(&M_Z80) + 1);
			}
			
			break;
		
		case GENS_KEY_w:
			if (debug_mode == DEBUG_SUB_68000_REG || debug_mode == DEBUG_SUB_68000_CDC)
				Check_CD_Command();
			
			break;
		
		case GENS_KEY_SPACE:
			if (debug_mode)
			{
				int tmp_debug = (int)debug_mode;
				tmp_debug++;
				
				if (SegaCD_Started)
				{
					if (tmp_debug > 6)
						tmp_debug = 1;
				}
				else if (_32X_Started)
				{
					if ((tmp_debug > 3) && (tmp_debug < 7))
						tmp_debug = 7;
					if (tmp_debug > 9)
						tmp_debug = 1;
				}
				else if (tmp_debug > 3)
					tmp_debug = 1;
				
				debug_mode = (DEBUG_MODE)tmp_debug;
			}
			
			break;
		
		case GENS_KEY_NUM_MULTIPLY:
			if (mod & GENS_KMOD_SHIFT)
				pattern_pal--;
			else
				pattern_pal++;
			
			pattern_pal &= 0xF;
			break;
		
		case GENS_KEY_NUM_PLUS:
			// Scroll down in pattern debugging.
			if (debug_mode == DEBUG_MAIN_68000 ||
			    debug_mode == DEBUG_Z80 ||
			    debug_mode == DEBUG_GENESIS_VDP)
			{
				if (pattern_adr < MD_VDP_PATTERN_MAX)
				{
					pattern_adr += pattern_inc;
					if (pattern_adr >= MD_VDP_PATTERN_MAX) // Make sure it doesn't go out of bounds.
						pattern_adr = MD_VDP_PATTERN_MAX;
				}
			}
			else if (debug_mode == DEBUG_WORD_RAM_PATTERN)
			{
				if (cd_pattern_adr < SEGACD_WRAM_PATTERN_MAX)
				{
					cd_pattern_adr += cd_pattern_inc;
					if (cd_pattern_adr >= (SEGACD_WRAM_PATTERN_MAX)) // Make sure it doesn't go out of bounds.
						cd_pattern_adr = SEGACD_WRAM_PATTERN_MAX;
				}
			}
			
			break;
		
		case GENS_KEY_NUM_MINUS:
			// Scroll up in pattern debugging.
			if (debug_mode == DEBUG_MAIN_68000 ||
			    debug_mode == DEBUG_Z80 ||
			    debug_mode == DEBUG_GENESIS_VDP)
			{
				if (pattern_adr > 0)
				{
					pattern_adr -= pattern_inc;
					if (pattern_adr < 0) // Make sure it doesn't go out of bounds.
						pattern_adr = 0;
				}
			}
			else if (debug_mode == DEBUG_WORD_RAM_PATTERN)
			{
				if (cd_pattern_adr > 0)
				{
					cd_pattern_adr -= cd_pattern_inc;
					if (cd_pattern_adr < 0)	// Make sure it doesn't go out of bounds.
						cd_pattern_adr = 0;
				}
			}
			
			break;
	}
}


/**
 * Next_Short(): Get the next short (16-bit) value.
 * @return Next short (16-bit) value.
 */
unsigned short Next_Word(void)
{
	unsigned short val = 0;
	
	if (debug_mode == DEBUG_MAIN_68000)
		val = M68K_RW(Current_PC);
	else if (debug_mode >= DEBUG_Z80)
		val = S68K_RW(Current_PC);
	
	Current_PC += 2;
	return val;
}


/**
 * Next_Long(): Get the next long (32-bit) value.
 * @return Next long (32-bit) value.
 */
unsigned int Next_Long(void)
{
	unsigned int val = 0;
	
	if (debug_mode == DEBUG_MAIN_68000)
	{
		val = M68K_RW(Current_PC);
		val <<= 16;
		val |= M68K_RW(Current_PC + 2);
	}
	else if (debug_mode >= DEBUG_Z80)
	{
		val = S68K_RW(Current_PC);
		val <<= 16;
		val |= S68K_RW(Current_PC + 2);
	}
	
	Current_PC += 4;
	return val;
}


/**
 * Refresh_M68k_Inst(): Refresh the Main 68000 instance.
 */
static void Refresh_M68k_Inst(void)
{
	Current_PC = main68k_context.pc;
	Print_Text("** MAIN 68000 DEBUG **", 24, 1, TEXT_GREEN);
	
	for (unsigned int i = 1; i < 14; i++)
	{
		unsigned int PC = Current_PC;
		PrintF_Text(1, (i << 3) + 5, (i == 1 ? TEXT_RED : TEXT_WHITE),
				"%04X   %-33s", PC, M68KDisasm(Next_Word, Next_Long));
	}
}


/**
 * Refresh_M68k_Inst(): Refresh the Sub 68000 instance.
 */
static void Refresh_S68k_Inst(void)
{
	Current_PC = sub68k_context.pc;
	Print_Text("** SUB 68000 DEBUG **", 24, 1, TEXT_GREEN);
	
	for (unsigned int i = 1; i < 14; i++)
	{
		unsigned int PC = Current_PC;
		PrintF_Text(1, (i << 3) + 5, (i == 1 ? TEXT_RED : TEXT_WHITE),
				"%04X   %-33s", PC, M68KDisasm(Next_Word, Next_Long));
	}
}


/**
 * Refresh_Z80_Inst(): Refresh the Z80 instance.
 */
static void Refresh_Z80_Inst(void)
{
	unsigned int PC = mdZ80_get_PC(&M_Z80);
	Print_Text("***** Z80 DEBUG *****", 24, 1, TEXT_GREEN);
	
	for (unsigned int i = 1; i < 14; i++)
	{
		z80dis((unsigned char*)Ram_Z80, (unsigned int*)&PC, Dbg_Out_Str, sizeof(Dbg_Out_Str));
		Print_Text(Dbg_Out_Str, 1, (i << 3) + 5, (i == 1 ? TEXT_RED : TEXT_WHITE));
	}
}


/**
 * Refresh_SH2_Inst(): Refresh an SH2 instance.
 * @param num ID of SH2 CPU (0 or 1).
 */
static void Refresh_SH2_Inst(int num)
{
	SH2_CONTEXT *sh;
	
	if (num)
	{
		Print_Text("** SLAVE SH2 DEBUG **", 24, 1, TEXT_GREEN);
		sh = &S_SH2;
	}
	else
	{
		Print_Text("** MASTER SH2 DEBUG **", 24, 1, TEXT_GREEN);
		sh = &M_SH2;
	}
	
	unsigned int PC = (sh->PC - sh->Base_PC) - 4;
	
	for (unsigned int i = 1; i < 14; i++, PC += 2)
	{
		SH2Disasm(Dbg_Out_Str, PC, SH2_Read_Word (sh, PC), 0);
		Print_Text(Dbg_Out_Str, 1, (i << 3) + 5, (i == 1 ? TEXT_RED : TEXT_WHITE));
	}
}


/**
 * Refresh_M68k_Mem(): Refresh the Main 68000 memory dump.
 */
static void Refresh_M68k_Mem(void)
{
	unsigned int Adr = adr_mem >> 1;
	Print_Text("** MAIN 68000 MEM **", 24, 130, TEXT_GREEN);
	
	for (unsigned int k = 0, j = Adr; k < 7; k++, j += 6)
	{
		// TODO: Optimize this to use Ram_68k.u16[].
		unsigned int i = (j & 0x7FFF) << 1;
		PrintF_Text(1, 146 + (k << 3), TEXT_WHITE,
				"%04X: %04X %04X %04X %04X %04X %04X",
				i, // current address
				Ram_68k.u8[i] | (Ram_68k.u8[i + 1] << 8),
				Ram_68k.u8[i + 2] | (Ram_68k.u8[i + 3] << 8),
				Ram_68k.u8[i + 4] | (Ram_68k.u8[i + 5] << 8),
				Ram_68k.u8[i + 6] | (Ram_68k.u8[i + 7] << 8),
				Ram_68k.u8[i + 8] | (Ram_68k.u8[i + 9] << 8),
				Ram_68k.u8[i + 10] | (Ram_68k.u8[i + 11] << 8));
    }
}


/**
 * Refresh_M68k_Mem(): Refresh the Sub 68000 memory dump.
 */
static void Refresh_S68k_Mem(void)
{
	unsigned int Adr = adr_mem >> 1;
	Print_Text("** SUB 68000 MEM **", 24, 130, TEXT_GREEN);
	
	for (unsigned int k = 0, j = Adr; k < 7; k++, j += 6)
	{
		unsigned int i = (j & 0x1FFFF) << 1;
		PrintF_Text(1, 146 + (k << 3), TEXT_WHITE,
				"%05X: %04X %04X %04X %04X %04X %04X",
				i, // current address
				Ram_Word_1M[i] + (Ram_Word_1M[i + 1] << 8),
				Ram_Word_1M[i + 2] + (Ram_Word_1M[i + 3] << 8),
				Ram_Word_1M[i + 4] + (Ram_Word_1M[i + 5] << 8),
				Ram_Word_1M[i + 6] + (Ram_Word_1M[i + 7] << 8),
				Ram_Word_1M[i + 8] + (Ram_Word_1M[i + 9] << 8),
				Ram_Word_1M[i + 10] + (Ram_Word_1M[i + 11] << 8));
	}
}


/**
 * Refresh_Z80_Mem(): Refresh the Z80 memory dump.
  */
static void Refresh_Z80_Mem(void)
{
	Print_Text("***** Z80 MEM *****", 24, 130, TEXT_GREEN);
	
	for (unsigned int k = 0, j = adr_mem & 0xFFFF; k < 7; k++, j = (j + 12) & 0xFFFF)
	{
		PrintF_Text(1, 146 + (k << 3), TEXT_WHITE,
				"%04X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
				j, // current address
				Z80_ReadB(j + 0), Z80_ReadB(j + 1), Z80_ReadB(j + 2),
				Z80_ReadB(j + 3), Z80_ReadB(j + 4), Z80_ReadB(j + 5),
				Z80_ReadB(j + 6), Z80_ReadB(j + 7), Z80_ReadB(j + 8),
				Z80_ReadB(j + 9), Z80_ReadB(j + 10), Z80_ReadB(j + 11));
	}
}


/**
 * Refresh_SH2_Mem(): Refresh the SH2 memory dump.
 */
static void Refresh_SH2_Mem(void)
{
	unsigned int i, j, k, Adr;
	Adr = adr_mem >> 1;
	Print_Text("** SH2 CPU MEM **", 24, 130, TEXT_GREEN);
	
	for (k = 0, j = Adr; k < 7; k++, j += 6)
	{
		// TODO: Optimize this to use _32X_Ram.u16[].
		i = (j & 0x1FFFF) << 1;
		PrintF_Text(1, 146 + (k << 3), TEXT_WHITE,
				"%05X: %04X %04X %04X %04X %04X %04X",
				i, // current address
				_32X_Ram.u8[i] + (_32X_Ram.u8[i + 1] << 8),
				_32X_Ram.u8[i + 2] + (_32X_Ram.u8[i + 3] << 8),
				_32X_Ram.u8[i + 4] + (_32X_Ram.u8[i + 5] << 8),
				_32X_Ram.u8[i + 6] + (_32X_Ram.u8[i + 7] << 8),
				_32X_Ram.u8[i + 8] + (_32X_Ram.u8[i + 9] << 8),
				_32X_Ram.u8[i + 10] + (_32X_Ram.u8[i + 11] << 8));
	}
}


/**
 * Refresh_M68k_State(): Refresh the Main 68000 status display.
 */
static void Refresh_M68k_State(void)
{
	Print_Text("** MAIN 68000 STATUS **", 196, 130, TEXT_GREEN);
	
	PrintF_Text(162, 146, TEXT_WHITE,
			"A0=%08X A1=%08X A2=%08X X=%d",
			main68k_context.areg[0], main68k_context.areg[1],
			main68k_context.areg[2], (main68k_context.sr & 0x10) ? 1 : 0);
	PrintF_Text(162, 154, TEXT_WHITE,
			"A3=%08X A4=%08X A5=%08X N=%d",
			main68k_context.areg[3], main68k_context.areg[4],
			main68k_context.areg[5], (main68k_context.sr & 0x8) ? 1 : 0);
	PrintF_Text(162, 162, TEXT_WHITE,
			"A6=%08X A7=%08X D0=%08X Z=%d",
			main68k_context.areg[6], main68k_context.areg[7],
			main68k_context.dreg[0], (main68k_context.sr & 0x4) ? 1 : 0);
	PrintF_Text(162, 170, TEXT_WHITE,
			"D1=%08X D2=%08X D3=%08X V=%d",
			main68k_context.dreg[1], main68k_context.dreg[2],
			main68k_context.dreg[3], (main68k_context.sr & 0x2) ? 1 : 0);
	PrintF_Text(162, 178, TEXT_WHITE,
			"D4=%08X D5=%08X D6=%08X C=%d",
			main68k_context.dreg[4], main68k_context.dreg[5],
			main68k_context.dreg[6], (main68k_context.sr & 0x1) ? 1 : 0);
	PrintF_Text(162, 186, TEXT_WHITE,
			"D7=%08X PC=%08X SR=%04X",
			main68k_context.dreg[7],
			main68k_context.pc, main68k_context.sr);
	PrintF_Text(162, 194, TEXT_WHITE,
			"Cycles = %010d ", main68k_context.odometer);
	PrintF_Text(162, 202, TEXT_WHITE,
			"Bank for Z80 = %06X", Bank_Z80);
}


/**
 * Refresh_S68k_State(): Refresh the Sub 68000 status display.
 */
static void Refresh_S68k_State(void)
{
	Print_Text("** SUB 68000 STATUS **", 196, 130, TEXT_GREEN);
	
	PrintF_Text(162, 146, TEXT_WHITE,
			"A0=%08X A1=%08X A2=%08X X=%d",
			sub68k_context.areg[0], sub68k_context.areg[1],
			sub68k_context.areg[2], (sub68k_context.sr & 0x10) ? 1 : 0);
	PrintF_Text(162, 154, TEXT_WHITE,
			"A3=%08X A4=%08X A5=%08X N=%d",
			sub68k_context.areg[3], sub68k_context.areg[4],
			sub68k_context.areg[5], (sub68k_context.sr & 0x8) ? 1 : 0);
	PrintF_Text(162, 162, TEXT_WHITE,
			"A6=%08X A7=%08X D0=%08X Z=%d",
			sub68k_context.areg[6], sub68k_context.areg[7],
			sub68k_context.dreg[0], (sub68k_context.sr & 0x4) ? 1 : 0);
	PrintF_Text(162, 170, TEXT_WHITE,
			"D1=%08X D2=%08X D3=%08X V=%d",
			sub68k_context.dreg[1], sub68k_context.dreg[2],
			sub68k_context.dreg[3], (sub68k_context.sr & 0x2) ? 1 : 0);
	PrintF_Text(162, 178, TEXT_WHITE,
			"D4=%08X D5=%08X D6=%08X C=%d",
			sub68k_context.dreg[4], sub68k_context.dreg[5],
			sub68k_context.dreg[6], (sub68k_context.sr & 0x1) ? 1 : 0);
	PrintF_Text(162, 186, TEXT_WHITE,
			"D7=%08X PC=%08X SR=%04X",
			sub68k_context.dreg[7],
			sub68k_context.pc, sub68k_context.sr);
	PrintF_Text(162, 194, TEXT_WHITE,
			"Cycles = %010d ", sub68k_context.odometer);
	PrintF_Text(162, 202, TEXT_WHITE,
			"Bank for main 68K = %06X", Bank_M68K);
}


/**
 * Refresh_Z80_State(): Refresh the Z80 status display.
 */
static void Refresh_Z80_State(void)
{
	Print_Text("***** Z80 STATUS *****", 196, 130, TEXT_GREEN);
	
	unsigned int AF = mdZ80_get_AF(&M_Z80);
	
	PrintF_Text(176, 146, TEXT_WHITE,
			"AF =%04X BC =%04X DE =%04X HL =%04X",
			AF, M_Z80.BC.w.BC, M_Z80.DE.w.DE, M_Z80.HL.w.HL);
	PrintF_Text(176, 154, TEXT_WHITE,
			"AF2=%04X BC2=%04X DE2=%04X HL2=%04X",
			mdZ80_get_AF2(&M_Z80), M_Z80.BC2.w.BC2,
			M_Z80.DE2.w.DE2, M_Z80.HL2.w.HL2);
	PrintF_Text(176, 162, TEXT_WHITE,
			"IX =%04X IY =%04X SP =%04X PC =%04X",
			M_Z80.IX.w.IX, M_Z80.IY.w.IY,
			M_Z80.SP.w.SP, mdZ80_get_PC(&M_Z80));
	PrintF_Text(176, 170, TEXT_WHITE,
			"IFF1=%d IFF2=%d I=%02X R=%02X IM=%02X",
			M_Z80.IFF.b.IFF1, M_Z80.IFF.b.IFF2, M_Z80.I,
			M_Z80.R.b.R1, M_Z80.IM);
	PrintF_Text(176, 178, TEXT_WHITE,
			"S=%d Z=%d Y=%d H=%d X=%d P=%d N=%d C=%d",
			(AF & 0x80) >> 7, (AF & 0x40) >> 6,
			(AF & 0x20) >> 5, (AF & 0x10) >> 4,
			(AF & 0x08) >> 3, (AF & 0x04) >> 2,
			(AF & 0x02) >> 1, (AF & 0x01) >> 0);
	PrintF_Text(176, 186, TEXT_WHITE,
			"Status=%02X ILine=%02X IVect=%02X\n",
			M_Z80.Status & 0xFF, M_Z80.IntLine, M_Z80.IntVect);
	PrintF_Text(176, 194, TEXT_WHITE,
			"Bank for main 68K = %06X", Bank_Z80);
	PrintF_Text(176, 202, TEXT_WHITE,
			"Z80 State = %02X", Z80_State);
}


/**
 * Refresh_Z80_State(): Refresh the Z80 status display.
 */
static void Refresh_SH2_State(int num)
{
	SH2_CONTEXT *sh;
	
	if (num)
	{
		Print_Text("** SLAVE SH2 STATUS **", 196, 130, TEXT_GREEN);
		sh = &S_SH2;
	}
	else
	{
		Print_Text("** MASTER SH2 STATUS **", 196, 130, TEXT_GREEN);
		sh = &M_SH2;
	}
	
	PrintF_Text(162, 146, TEXT_WHITE,
			"R0=%08X R1=%08X R2=%08X T=%d",
			SH2_Get_R(sh, 0), SH2_Get_R(sh, 1),
			SH2_Get_R(sh, 2), SH2_Get_SR(sh) & 1);
	PrintF_Text(162, 154, TEXT_WHITE,
			"R3=%08X R4=%08X R5=%08X S=%d",
			SH2_Get_R(sh, 3), SH2_Get_R(sh, 4),
			SH2_Get_R(sh, 5), (SH2_Get_SR(sh) >> 1) & 1);
	PrintF_Text(162, 162, TEXT_WHITE,
			"R6=%08X R7=%08X R8=%08X Q=%d",
			SH2_Get_R(sh, 6), SH2_Get_R(sh, 7),
			SH2_Get_R(sh, 8), (SH2_Get_SR(sh) >> 8) & 1);
	PrintF_Text(162, 170, TEXT_WHITE,
			"R9=%08X RA=%08X RB=%08X M=%d",
			SH2_Get_R(sh, 9), SH2_Get_R(sh, 0xA),
			SH2_Get_R(sh, 0xB), (SH2_Get_SR(sh) >> 9) & 1);
	PrintF_Text(162, 178, TEXT_WHITE,
			"RC=%08X RD=%08X RE=%08X I=%01X",
			SH2_Get_R(sh, 0xC), SH2_Get_R(sh, 0xD),
			SH2_Get_R(sh, 0xE), (SH2_Get_SR(sh) >> 4) & 0xF);
	PrintF_Text(162, 186, TEXT_WHITE,
			"RF=%08X PC=%08X SR=%04X St=%04X",
			SH2_Get_R(sh, 0xF), SH2_Get_PC(sh),
			SH2_Get_SR(sh), sh->Status & 0xFFFF);
	PrintF_Text(162, 194, TEXT_WHITE,
			"GBR=%08X VBR=%08X PR=%08X",
			SH2_Get_GBR(sh), SH2_Get_VBR(sh), SH2_Get_PR(sh));
	PrintF_Text(162, 202, TEXT_WHITE,
			"MACH=%08X MACL=%08X IL=%02X IV=%02X",
			SH2_Get_MACH(sh), SH2_Get_MACL(sh), sh->INT.Prio, sh->INT.Vect);
}


/**
 * Refresh_VDP_State(): Refresh the VDP status display.
 */
static void Refresh_VDP_State(void)
{
	Print_Text("**** VDP STATUS ****", 200, 1, TEXT_GREEN);
	
	PrintF_Text(162, 14, TEXT_WHITE,
			"Setting register: 1=%02X 2=%02X 3=%02X 4=%02X",
			VDP_Reg.m5.Set1, VDP_Reg.m5.Set2,
			VDP_Reg.m5.Set3, VDP_Reg.m5.Set4);
	PrintF_Text(162, 22, TEXT_WHITE,
			"Pattern Adr: ScrA=%02X ScrB=%02X Win=%02X",
			VDP_Reg.m5.Pat_ScrA_Adr, VDP_Reg.m5.Pat_ScrB_Adr,
			VDP_Reg.m5.Pat_Win_Adr);
	PrintF_Text(162, 30, TEXT_WHITE,
			"Sprite Attribute Adr: Low=%02X High=%02X",
			VDP_Reg.m5.Spr_Att_Adr, VDP_Reg.m5.Reg6);
	PrintF_Text(162, 38, TEXT_WHITE,
			"H Scroll Adr: Low=%02X High=%02X",
			VDP_Reg.m5.H_Scr_Adr, VDP_Reg.m5.Reg14);
	PrintF_Text(162, 46, TEXT_WHITE,
			"H Interrupt=%02X    Auto Inc=%02X",
			VDP_Reg.m5.H_Int, VDP_Reg.m5.Auto_Inc);
	PrintF_Text(162, 54, TEXT_WHITE,
			"BG Color: Low=%02X Med=%02X High=%02X",
			VDP_Reg.m5.BG_Color, VDP_Reg.m5.Reg8, VDP_Reg.m5.Reg9);
	PrintF_Text(162, 62, TEXT_WHITE,
			"Scroll Size=%02X    Window Pos: H=%02X V=%02X",
			VDP_Reg.m5.Scr_Size, VDP_Reg.m5.Win_H_Pos, VDP_Reg.m5.Win_V_Pos);
	PrintF_Text(162, 70, TEXT_WHITE,
			"DMA Length: Low=%.2X High=%.2X",
			VDP_Reg.m5.DMA_Length_L, VDP_Reg.m5.DMA_Length_H);
	PrintF_Text(162, 78, TEXT_WHITE,
			"DMA Source Adr: Low=%.2X Med=%.2X High=%.2X",
			VDP_Reg.m5.DMA_Src_Adr_L, VDP_Reg.m5.DMA_Src_Adr_M,
			VDP_Reg.m5.DMA_Src_Adr_H);
	
	int tmp = VDP_Read_Status();
	
	PrintF_Text(162, 86, TEXT_WHITE,
			"V Int Happened %d  Sprite overflow %d",
			(tmp >> 7) & 1, (tmp >> 6) & 1);
	PrintF_Text(162, 94, TEXT_WHITE,
			"Collision Spr  %d  Odd Frame in IM %d",
			(tmp >> 5) & 1, (tmp >> 4) & 1);
	PrintF_Text(162, 102, TEXT_WHITE,
			"During V Blank %d  During H Blank  %d",
			(tmp >> 3) & 1, (tmp >> 2) & 1);
	PrintF_Text(162, 110, TEXT_WHITE,
			"DMA Busy %d  PAL Mode %d Line Num %d",
			(tmp >> 1) & 1, tmp & 1, VDP_Lines.Display.Current);
	PrintF_Text(162, 118, TEXT_WHITE,
			"VDP Int =%02X DMA_Length=%04X",
			VDP_Int, VDP_Reg.DMAT_Length);
}


/**
 * Refresh_VDP_Pattern(): Refresh the VDP pattern display.
 */
static void Refresh_VDP_Pattern(void)
{
	Print_Text("******** VDP PATTERN ********", 28, 0, TEXT_GREEN);
	
	// VDP cells can be either 8x8 (Normal) or 8x16 (Interlaced).
	// TODO: This checks LSM1 only. Check both LSM1 and LSM0!
	int VRam_Inc;
	if (VDP_Reg.m5.Set4 & 0x04)
	{
		// Interlaced mode. (8x16 cells)
		VRam_Inc = 2;
		pattern_inc  = MD_VDP_PATTERN_INTERLACED_INC;
		pattern_mask = MD_VDP_PATTERN_INTERLACED_MASK;
	}
	else
	{
		// Normal mode. (8x8 cells)
		VRam_Inc = 1;
		pattern_inc  = MD_VDP_PATTERN_NORMAL_INC;
		pattern_mask = MD_VDP_PATTERN_NORMAL_MASK;
	}
	
	// Make sure the pattern address is on a cell boundary.
	pattern_adr &= (~pattern_mask & 0xFFFF);
	if (pattern_adr > MD_VDP_PATTERN_MAX)
		pattern_adr = MD_VDP_PATTERN_MAX;
	
	// TODO: This checks LSM1 only. Check both LSM1 and LSM0!
	for (unsigned int i = 0; i < 24; i += VRam_Inc)
	{
		PrintF_Text(2, (i << 3) + 11, TEXT_WHITE,
				"%04X", (pattern_adr + (0x200 * i)));
	}
	
	// TODO: This checks LSM1 only. Check both LSM1 and LSM0!
	if (VDP_Reg.m5.Set4 & 0x04)
		Cell_8x16_Dump(&VRam.u8[pattern_adr], pattern_pal);
	else
		Cell_8x8_Dump(&VRam.u8[pattern_adr], pattern_pal);
}


template<typename pixel>
static inline void T_Refresh_VDP_Palette_Colors(pixel *screen, pixel *palette, unsigned int numPalettes)
{
	// Check if the palettes need to be updated.
	if (VDP_Flags.CRam)
	{
		// Palettes need to be updated.
		if (VDP_Reg.m5.Set4 & 0x10)
			VDP_Update_Palette_HS();
		else
			VDP_Update_Palette();
	}
	
	const unsigned int line_number = (VDP_Lines.Visible.Border_Size + 10);
	pixel *pLine = &screen[(336 * line_number) + 180];
	
	// Palette number.
	int pal_num = -1;
	
	for (unsigned int line = (numPalettes * 8); line != 0; line--, pLine += (336-(16*8)))
	{
		// Check if we're starting a new palette.
		if (!(line & 7))
			pal_num++;
		
		// Draw the palette line.
		int color_num = (pal_num * 16) - 1;
		for (unsigned int col = (16*8); col != 0; col -= 2, pLine += 2)
		{
			if (!(col & 7))
				color_num++;
			
			*pLine		= palette[color_num];
			*(pLine + 1)	= palette[color_num];
		}
	}
}

template<typename pixel>
static inline void T_Refresh_VDP_Palette_Outline(pixel *screen, unsigned int paletteMask, pixel outlineColor)
{
	// Outline the selected palette. Ported from Gens Rerecording.
	const unsigned int line_number = (VDP_Lines.Visible.Border_Size + 9);
	pixel *line1 = &screen[(336 * (line_number + ((pattern_pal & paletteMask) * 8))) + 180];
	pixel *line2 = line1 + (336 * 9);
	
	for (unsigned int i = 16 * 8; i != 0; i -= 2, line1 += 2, line2 += 2)
	{
		*line1		= outlineColor;
		*line2		= outlineColor;
		*(line1 + 1)	= outlineColor;
		*(line2 + 1)	= outlineColor;
	}
}

/**
 * Refresh_VDP_Palette(): Refresh the VDP palette display.
 */
static void Refresh_VDP_Palette(void)
{
	Print_Text("******** VDP PALETTE ********", 180, 0, TEXT_RED);
	
	if (bppMD == 15)
	{
		// 15-bit color palette update.
		T_Refresh_VDP_Palette_Colors<uint16_t>(MD_Screen.u16, MD_Palette.u16, 4);
		T_Refresh_VDP_Palette_Outline<uint16_t>(MD_Screen.u16, 0x03, 0x7FFF);
	}
	else if (bppMD == 16)
	{
		// 16-bit color palette update.
		T_Refresh_VDP_Palette_Colors<uint16_t>(MD_Screen.u16, MD_Palette.u16, 4);
		T_Refresh_VDP_Palette_Outline<uint16_t>(MD_Screen.u16, 0x03, 0xFFFF);
	}
	else //if (bppMD == 32)
	{
		// 32-bit color palette update.
		T_Refresh_VDP_Palette_Colors<uint32_t>(MD_Screen.u32, MD_Palette.u32, 4);
		T_Refresh_VDP_Palette_Outline<uint32_t>(MD_Screen.u32, 0x03, 0xFFFFFF);
	}
	
	Print_Text("******** VDP CONTROL ********", 180, 60, TEXT_WHITE);
	
	PrintF_Text(176, 70, TEXT_WHITE,
			"Status : %04X", VDP_Read_Status());
	PrintF_Text(176, 78, TEXT_WHITE,
			"Flag : %02X       Data : %08X",
			VDP_Ctrl.Flag, VDP_Ctrl.Data);
	PrintF_Text(176, 86, TEXT_WHITE,
			"Write : %02X      Access : %02X",
			VDP_Ctrl.Write, VDP_Ctrl.Access);
	PrintF_Text(176, 94, TEXT_WHITE,
			"Address : %04X  DMA_Mode : %02X",
			VDP_Ctrl.Address, VDP_Ctrl.DMA_Mode);
	PrintF_Text(176, 102, TEXT_WHITE,
			"DMA adr: %08X  DMA len: %04X",
			VDP_Reg.DMA_Address, VDP_Reg.DMA_Length);
	PrintF_Text(176, 110, TEXT_WHITE,
			"DMA : %02X", VDP_Ctrl.DMA);
	
	Print_Text("Sprite List:", 176, 126, TEXT_WHITE);
	Print_Text("   X    Y W H  Addr HF VF Pal Pri", 176, 134, TEXT_WHITE);
	for (unsigned int i = 0; i < 10; i++)
	{
		PrintF_Text(176, 142 + (i * 8), TEXT_WHITE,
				"%4d %4d %d %d $%04X  %d  %d  %d   %d",
				Sprite_Struct[i].Pos_X, Sprite_Struct[i].Pos_Y,
				Sprite_Struct[i].Size_X-1, Sprite_Struct[i].Size_Y,
				(Sprite_Struct[i].Num_Tile & 0x7FF) << 5,
				(Sprite_Struct[i].Num_Tile & 0x0400 ? 1 : 0),
				(Sprite_Struct[i].Num_Tile & 0x0800 ? 1 : 0),
				(Sprite_Struct[i].Num_Tile & 0x6000) >> 13,
				(Sprite_Struct[i].Num_Tile & 0x8000) ? 1 : 0);
	}
}


/**
 * Refresh_SegaCD_State(): Refresh the Sega CD status display.
 */
static void Refresh_SegaCD_State(void)
{
	Print_Text("** SEGACD STATUS **", 200, 1, TEXT_GREEN);
	
	PrintF_Text(162, 14, TEXT_WHITE,
			"GE00=%04X GE02=%04X CD00=%04X CD02=%04X",
			M68K_RW(0xA12000), M68K_RW(0xA12002),
			S68K_RW(0xFF8000), S68K_RW(0xFF8002));
	PrintF_Text(162, 22, TEXT_WHITE,
			"GE04=%04X GE06=%04X CD04=%04X CD06=%04X",
			M68K_RW(0xA12004), M68K_RW(0xA12006),
			S68K_RW(0xFF8004), 0x0000);	// TODO: 0x0000? Should this be S68K_RW(0xFF8006)?
	PrintF_Text(162, 30, TEXT_WHITE,
			"GE0A=%04X GE0C=%04X CD0A=%04X CD0C=%04X",
			M68K_RW(0xA1200A), M68K_RW(0xA1200C),
			S68K_RW(0xFF800A), S68K_RW(0xFF800C));
	PrintF_Text(162, 38, TEXT_WHITE,
			"GD0E=%04X", S68K_RW(0xFF800E));
	PrintF_Text(162, 46, TEXT_WHITE,
			"GD10=%04X GD12=%04X GD14=%04X GD16=%04X",
			S68K_RW(0xFF8010), S68K_RW(0xFF8012),
			S68K_RW(0xFF8014), S68K_RW(0xFF8016));
	PrintF_Text(162, 54, TEXT_WHITE,
			"GD18=%04X GD1A=%04X GD1C=%04X GD1E=%04X",
			S68K_RW(0xFF8018), S68K_RW(0xFF801A),
			S68K_RW(0xFF801C), S68K_RW(0xFF801E));
	PrintF_Text(162, 62, TEXT_WHITE,
			"GD20=%04X GD22=%04X GD24=%04X GD26=%04X",
			S68K_RW(0xFF8020), S68K_RW(0xFF8022),
			S68K_RW(0xFF8024), S68K_RW(0xFF8026));
	PrintF_Text(162, 70, TEXT_WHITE,
			"GD28=%04X GD2A=%04X GD2C=%04X GD2E=%04X",
			S68K_RW(0xFF8028), S68K_RW(0xFF802A),
			S68K_RW(0xFF802C), S68K_RW(0xFF802E));
	PrintF_Text(162, 78, TEXT_WHITE,
			"CD30=%04X CD32=%04X CD34=%04X CD36=%04X",
			S68K_RW(0xFF8030), S68K_RW(0xFF8032),
			S68K_RW(0xFF8034), S68K_RW(0xFF8036));
	PrintF_Text(162, 86, TEXT_WHITE,
			"CD38=%04X CD3A=%04X CD3E=%04X CD40=%04X",
			S68K_RW(0xFF8038), S68K_RW(0xFF803A),
			S68K_RW(0xFF803E), S68K_RW(0xFF8040));
	PrintF_Text(162, 94, TEXT_WHITE,
			"CD42=%04X CD44=%04X CD48=%04X CD4A=%04X",
			S68K_RW(0xFF8042), S68K_RW(0xFF8044),
			S68K_RW(0xFF8048), S68K_RW(0xFF804A));
	PrintF_Text(162, 102, TEXT_WHITE,
			"CD4C=%04X CD4E=%04X CD50=%04X CD52=%04X",
			S68K_RW(0xFF804C), S68K_RW(0xFF804E),
			S68K_RW(0xFF8050), S68K_RW(0xFF8052));
	PrintF_Text(162, 110, TEXT_WHITE,
			"CD58=%04X CD5A=%04X CD5C=%04X CD5E=%04X",
			S68K_RW(0xFF8058), S68K_RW(0xFF805A),
			S68K_RW(0xFF805C), S68K_RW(0xFF805E));
	PrintF_Text(162, 118, TEXT_WHITE,
			"CD60=%04X CD62=%04X CD64=%04X CD66=%04X",
			S68K_RW(0xFF8060), S68K_RW(0xFF8062),
			S68K_RW(0xFF8064), S68K_RW(0xFF8066));
}


/**
 * Refresh_32X_State(): Refresh the 32X status display.
 */
static void Refresh_32X_State(void)
{
	Print_Text("** 32X STATUS **", 200, 1, TEXT_GREEN);
	
	PrintF_Text(162, 14, TEXT_WHITE,
			"M000=%04X S000=%04X M004=%04X M006=%04X",
			SH2_Read_Word(&M_SH2, 0x4000), SH2_Read_Word(&S_SH2, 0x4000),
			SH2_Read_Word(&M_SH2, 0x4004), SH2_Read_Word(&M_SH2, 0x4006));
	PrintF_Text(162, 22, TEXT_WHITE,
			"M008=%04X M00A=%04X M00C=%04X M00E=%04X",
			SH2_Read_Word(&M_SH2, 0x4008), SH2_Read_Word(&M_SH2, 0x400A),
			SH2_Read_Word(&M_SH2, 0x400C), SH2_Read_Word(&M_SH2, 0x400E));
	PrintF_Text(162, 30, TEXT_WHITE,
			"M010=%04X M012=%04X M014=%04X M016=%04X",
			SH2_Read_Word(&M_SH2, 0x4010), SH2_Read_Word(&M_SH2, 0x4012),
			SH2_Read_Word(&M_SH2, 0x4014), SH2_Read_Word(&M_SH2, 0x4016));
	PrintF_Text(162, 38, TEXT_WHITE,
			"M020=%04X M022=%04X M024=%04X M026=%04X",
			SH2_Read_Word(&M_SH2, 0x4020), SH2_Read_Word(&M_SH2, 0x4022),
			SH2_Read_Word(&M_SH2, 0x4024), SH2_Read_Word(&M_SH2, 0x4026));
	PrintF_Text(162, 46, TEXT_WHITE,
			"M028=%04X M02A=%04X M02C=%04X M02E=%04X",
			SH2_Read_Word(&M_SH2, 0x4028), SH2_Read_Word(&M_SH2, 0x402A),
			SH2_Read_Word(&M_SH2, 0x402C), SH2_Read_Word(&M_SH2, 0x402E));
	PrintF_Text(162, 54, TEXT_WHITE,
			"M030=%04X M032=%04X M034=%04X M036=%04X",
			SH2_Read_Word(&M_SH2, 0x4030), SH2_Read_Word(&M_SH2, 0x4032),
			SH2_Read_Word(&M_SH2, 0x4034), SH2_Read_Word(&M_SH2, 0x4036));
	PrintF_Text(162, 62, TEXT_WHITE,
			"M100=%04X M102=%04X M104=%04X M106=%04X",
			SH2_Read_Word(&M_SH2, 0x4100), SH2_Read_Word(&M_SH2, 0x4102),
			SH2_Read_Word(&M_SH2, 0x4104), SH2_Read_Word(&M_SH2, 0x4106));
	PrintF_Text(162, 70, TEXT_WHITE,
			"M108=%04X M10A=%04X M10C=%04X M10E=%04X",
			SH2_Read_Word(&M_SH2, 0x4108), SH2_Read_Word(&M_SH2, 0x410A),
			SH2_Read_Word(&M_SH2, 0x410C), SH2_Read_Word(&M_SH2, 0x410E));
}


/**
 * Refresh_CDC_State(): Refresh the CDC status display.
 */
static void Refresh_CDC_State(void)
{
	Print_Text("** CDC STATUS **", 200, 1, TEXT_GREEN);
	
	PrintF_Text(162, 14, TEXT_WHITE,
			"COMIN=%02X IFSTAT=%02X DBC=%04X",
			CDC.COMIN, CDC.IFSTAT, CDC.DBC.N);
	PrintF_Text(162, 22, TEXT_WHITE,
			"HEAD=%08X PT=%04X WA=%04X",
			CDC.HEAD.N, CDC.PT.N, CDC.WA.N);
	PrintF_Text(162, 30, TEXT_WHITE,
			"STAT=%08X CTRL=%08X",
			CDC.STAT.N, CDC.CTRL.N);
	PrintF_Text(162, 38, TEXT_WHITE,
			"DAC=%04X IFCTRL=%02X",
			CDC.DAC.N, CDC.IFCTRL);
}


/**
 * Refresh_Word_RAM_Pattern(): Refresh the Word RAM pattern status display.
 */
static void Refresh_Word_RAM_Pattern(void)
{
	// Improved Word RAM pattern display function ported from Gens Rerecording.
	
	Print_Text("******** VDP PALETTE ********", 180, 0, TEXT_RED);
	
	if (bppMD == 15)
	{
		// 15-bit color palette update.
		T_Refresh_VDP_Palette_Colors<uint16_t>(MD_Screen.u16, MD_Palette.u16, 16);
		T_Refresh_VDP_Palette_Outline<uint16_t>(MD_Screen.u16, 0x0F, 0x7FFF);
	}
	else if (bppMD == 16)
	{
		// 16-bit color palette update.
		T_Refresh_VDP_Palette_Colors<uint16_t>(MD_Screen.u16, MD_Palette.u16, 16);
		T_Refresh_VDP_Palette_Outline<uint16_t>(MD_Screen.u16, 0x0F, 0xFFFF);
	}
	else //if (bppMD == 32)
	{
		// 32-bit color palette update.
		T_Refresh_VDP_Palette_Colors<uint32_t>(MD_Screen.u32, MD_Palette.u32, 16);
		T_Refresh_VDP_Palette_Outline<uint32_t>(MD_Screen.u32, 0x0F, 0xFFFFFF);
	}
	
	Print_Text("****** WORD RAM PATTERN ******", 28, 0, TEXT_GREEN);
	
	// Word RAM patterns can be either 16x16 or 32x32.
	int WRam_Inc;
	if (Rot_Comp.Stamp_Size & 2)
	{
		// 32x32.
		WRam_Inc = 4;
		cd_pattern_inc  = SEGACD_WRAM_PATTERN_32x32_INC;
		cd_pattern_mask = SEGACD_WRAM_PATTERN_32x32_MASK;
	}
	else
	{
		// 16x16.
		WRam_Inc = 2;
		cd_pattern_inc  = SEGACD_WRAM_PATTERN_16x16_INC;
		cd_pattern_mask = SEGACD_WRAM_PATTERN_16x16_MASK;
	}
	
	// Make sure the pattern address is on a cell boundary.
	cd_pattern_adr &= (~cd_pattern_mask & 0x3FFFF);
	if (cd_pattern_adr > SEGACD_WRAM_PATTERN_MAX)
		cd_pattern_adr = SEGACD_WRAM_PATTERN_MAX;
	
	for (unsigned int i = 0; i < 24; i += WRam_Inc)
	{
		PrintF_Text(2, (i << 3) + 11, TEXT_WHITE,
				"%04X", (cd_pattern_adr + (0x200 * i)));
	}
	
	if (Rot_Comp.Stamp_Size & 2)
		Cell_32x32_Dump(&Ram_Word_2M[cd_pattern_adr], pattern_pal);
	else
		Cell_16x16_Dump(&Ram_Word_2M[cd_pattern_adr], pattern_pal);
}


/**
 * Update_Debug_Screen(): Update the debug screen.
 */
void Update_Debug_Screen(void)
{
	// Clear the MD screen.
	if (bppMD == 32)
		memset(&MD_Screen.u32, 0x00, sizeof(MD_Screen.u32));
	else
		memset(&MD_Screen.u16, 0x00, sizeof(MD_Screen.u16));
	
	if (debug_show_vdp)
	{
		if (_32X_Started)
			Do_32X_VDP_Only();
		else
			Do_VDP_Only();
	}
	else
	{
		switch (debug_mode)
		{
			default:
			case DEBUG_MAIN_68000:
				// Main 68000
				Refresh_M68k_Mem();
				Refresh_M68k_Inst();
				Refresh_M68k_State();
				Refresh_VDP_State();
				break;
			
			case DEBUG_Z80:
				// Z80
				Refresh_Z80_Mem();
				Refresh_Z80_Inst();
				Refresh_Z80_State();
				break;
			
			case DEBUG_GENESIS_VDP:
				// Genesis VDP
				Refresh_VDP_Palette();
				Refresh_VDP_Pattern();
				break;
			
			case DEBUG_SUB_68000_REG:
				// Sub 68000 reg
				Refresh_S68k_Mem();
				Refresh_S68k_Inst();
				Refresh_S68k_State();
				Refresh_SegaCD_State();
				break;
			
			case DEBUG_SUB_68000_CDC:
				// Sub 68000 CDC
				Refresh_S68k_Mem();
				Refresh_S68k_Inst();
				Refresh_S68k_State();
				Refresh_CDC_State();
				break;
			
			case DEBUG_WORD_RAM_PATTERN:
				// Word RAM pattern
				Refresh_Word_RAM_Pattern();
				break;
			
			case DEBUG_MAIN_SH2:
				// Main SH2
				Refresh_SH2_Mem();
				Refresh_SH2_Inst(0);
				Refresh_SH2_State(0);
				Refresh_32X_State();
				break;
			
			case DEBUG_SUB_SH2:
				// Sub SH2
				Refresh_SH2_Mem();
				Refresh_SH2_Inst(1);
				Refresh_SH2_State(1);
				Refresh_32X_State();
				break;
			
			case DEBUG_32X_VDP:
				// 32X VDP
				_32X_VDP_Draw(Current_32X_FB);
				break;
		}
	}
	
	GensUI::sleep(10);
}
