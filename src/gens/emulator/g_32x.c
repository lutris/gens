/**
 * GENS: Sega 32X initialization and main loop code.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "g_32x.h"
#include "g_md.h"
#include "gens.h"
#include "g_main.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "rom.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "ym2612.h"
#include "psg.h"
#include "cpu_68k.h"
#include "cpu_z80.h"
#include "cpu_sh2.h"
#include "z80.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"
#include "io.h"
#include "misc.h"
#include "save.h"
#include "ggenie.h"
#include "cd_sys.h"
#include "lc89510.h"
#include "gfx_cd.h"
#include "wave.h"
#include "pcm.h"
#include "pwm.h"
#include "cd_sys.h"
#include "cd_file.h"
#include "gym.h"
#include "support.h"

/**
 * Error_32X_BIOS(): Displays an error message if a BIOS file isn't found.
 * @param Str_BIOS Missing BIOS file.
 */
void Error_32X_BIOS(const char *Str_BIOS)
{
	char Str_Err[256];
	sprintf(Str_Err, "Your 32X BIOS files aren't correctly configured:\n%s\nGo to menu 'Options -> BIOS/Misc Files' to set them up.", Str_BIOS);
	open_msgbox(Str_Err);
}

/**
 * Init_32X(): Initialize the 32X with the specified ROM image.
 * @param MD_Rom ROM image struct.
 * @return 1 if successful; 0 if an error occurred.
 */
int Init_32X (struct Rom *MD_Rom)
{
	char Str_Err[256];
	FILE *f;
	int i;
	
	/*
	strcpy(_32X_Genesis_Bios, "32X_G_BIOS.BIN");
	strcpy(_32X_Master_Bios, "32X_M_BIOS.BIN");
	strcpy(_32X_Slave_Bios, "32X_S_BIOS.BIN");
	*/
	
	// Read the Genesis 32X BIOS (usually "32X_G_BIOS.BIN")
	if ((f = fopen (_32X_Genesis_Bios, "rb")))
	{
		fread (&_32X_Genesis_Rom[0], 1, 256, f);
		Byte_Swap (&_32X_Genesis_Rom[0], 256);
		fclose (f);
	}
	else
	{
		Error_32X_BIOS("Genesis 32X");
		return 0;
	}
	
	// Read the Master SH2 BIOS (usually "32X_M_BIOS.BIN")	
	if ((f = fopen (_32X_Master_Bios, "rb")))
	{
		fread (&_32X_MSH2_Rom[0], 1, 2 * 1024, f);
		fclose (f);
	}
	else
	{
		Error_32X_BIOS("Master SH2");
		return 0;
	}
	
	// Read the Slave SH2 BIOS (usually "32X_S_BIOS.BIN")
	if ((f = fopen (_32X_Slave_Bios, "rb")))
	{
		fread (&_32X_SSH2_Rom[0], 1, 1 * 1024, f);
		fclose (f);
	}
	else
	{
		Error_32X_BIOS("Slave SH2");
		return 0;
	}
	
	Flag_Clr_Scr = 1;
	Debug = Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	Controller_1_COM = Controller_2_COM = 0;
	
	if (!Kaillera_Client_Running)
	{
		if ((MD_Rom->Ram_Infos[8] == 'R') && (MD_Rom->Ram_Infos[9] == 'A')
			&& (MD_Rom->Ram_Infos[10] & 0x40))
		{
			// SRAM specified in the ROM header. Use this address.
			SRAM_Start = MD_Rom->Ram_Start_Adress & 0x0F80000;	// multiple de 0x080000
			SRAM_End = MD_Rom->Ram_End_Adress;
		}
		else
		{
			SRAM_Start = 0x200000;
			SRAM_End = 0x200000 + (64 * 1024) - 1;
		}
		
		if ((SRAM_Start > SRAM_End) || ((SRAM_End - SRAM_Start) >= (64 * 1024)))
			SRAM_End = SRAM_Start + (64 * 1024) - 1;
		
		// If the ROM is smaller than the SRAM starting address, always enable SRAM.
		// TODO: Instead of hardcoding 2MB, use SRAM_Start.
		if (Rom_Size <= (2 * 1024 * 1024))
		{
			SRAM_ON = 1;
			SRAM_Write = 1;
		}
		
		// Make sure SRAM starts on an even byte and ends on an odd byte.
		SRAM_Start &= 0xFFFFFFFE;
		SRAM_End |= 0x00000001;
		
//		sprintf(Str_Err, "deb = %.8X end = %.8X", SRAM_Start, SRAM_End);
//		MessageBox(NULL, Str_Err, "", MB_OK);
		
		if ((SRAM_End - SRAM_Start) <= 2)
			SRAM_Custom = 1;
		else
			SRAM_Custom = 0;
		
		Load_SRAM ();
	}
	
	// Check what country code should be used.
	// TODO: Get rid of magic numbers.
	switch (Country)
	{
		default:
		case -1: // Autodetection.
			Detect_Country_Genesis ();
			break;
		
		case 0: // Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			break;
		
		case 1: // US (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			break;
		
		case 2: // Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			break;
		
		case 3: // Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			break;
	}
	
	// Set the window title to the localized console name and the game name.
	if (CPU_Mode == 1)
		sprintf (Str_Err, "Gens - 32X (PAL) : %s", MD_Rom->Rom_Name_W);
	else
		sprintf (Str_Err, "Gens - 32X (NTSC) : %s", MD_Rom->Rom_Name_W);
	SetWindowText (Str_Err);
	
	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)
	
	// Two copies of the ROM are needed:
	// - Non-byteswapped for SH2
	// - Byteswapped for 68000
	memcpy (_32X_Rom, Rom_Data, 4 * 1024 * 1024);	// no byteswapped image (for SH2)
	Byte_Swap (Rom_Data, Rom_Size);	// byteswapped image (for 68000)
	
	// Reset all CPUs and other components.
	MSH2_Reset ();
	SSH2_Reset ();
	M68K_Reset (1);
	Z80_Reset ();
	Reset_VDP ();
	_32X_VDP_Reset ();
	_32X_Set_FB ();
	PWM_Init ();
	
	// Set clock rates depending on the CPU mode (NTSC / PAL).
	if (CPU_Mode)
	{
		// PAL
		CPL_Z80 = Round_Double ((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double ((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 =
			Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
				(double) MSH2_Speed) / 100.0);
		CPL_SSH2 =
			Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
				(double) SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
		
		YM2612_Init (CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init (CLOCK_PAL / 15, Sound_Rate);
	}
	else
	{
		// NTSC
		CPL_Z80 = Round_Double ((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = Round_Double ((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 =
			Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
				(double) MSH2_Speed) / 100.0);
		CPL_SSH2 =
			Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
				(double) SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		_32X_VDP.Mode |= 0x8000;
		
		YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init (CLOCK_NTSC / 15, Sound_Rate);
	}
	
	_32X_VDP.State |= 0x2000;
	
	// If auto-fix checksum is enabled, fix the ROM checksum.
	if (Auto_Fix_CS)
		Fix_Checksum ();
	
	// Initialize sound.
	if (Sound_Enable)
	{
		End_Sound ();
		
		if (!Init_Sound ())
			Sound_Enable = 0;
		else
			Play_Sound ();
	}
	
	Load_Patch_File ();
	
	Last_Time = GetTickCount ();
	New_Time = 0;
	Used_Time = 0;
	
	// Set the appropriate frame update function pointers.
	Update_Frame = Do_32X_Frame;
	Update_Frame_Fast = Do_32X_Frame_No_VDP;
	
	// We patch the Master SH2 bios with ROM bios
	// this permit 32X games with older BIOS version to run correctly
	// Ecco 32X demo needs it
	for (i = 0; i < 0x400; i++)
		_32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];
	
	return 1;
}


/**
 * Reset_32X(): Resets the 32X.
 */
void Reset_32X (void)
{
	int i;
	
	Paused = 0;
	Controller_1_COM = Controller_2_COM = 0;
	_32X_ADEN = _32X_RES = _32X_FM = _32X_RV = 0;
	
	// If the ROM size is smaller than 2MB, enable SRAM.
	// Otherwise, disable SRAM.
	// TODO: Instead of hardcoding 2MB, use SRAM_Start.
	if (Rom_Size <= (2 * 1024 * 1024))
	{
		SRAM_ON = 1;
		SRAM_Write = 1;
	}
	else
	{
		SRAM_ON = 0;
		SRAM_Write = 0;
	}
	
	// Reset all CPUs and other components.
	MSH2_Reset ();
	SSH2_Reset ();
	M68K_Reset (1);
	Z80_Reset ();
	Reset_VDP ();
	_32X_VDP_Reset ();
	_32X_Set_FB ();
	YM2612_Reset ();
	PWM_Init ();
	
	if (CPU_Mode)
	{
		VDP_Status |= 1;
		_32X_VDP.Mode &= ~0x8000;
	}
	else
	{
		VDP_Status &= ~1;
		_32X_VDP.Mode |= 0x8000;
	}
	
	_32X_VDP.State |= 0x2000;
	
	if (Auto_Fix_CS)
		Fix_Checksum ();
	
	// We patch the Master SH2 bios with ROM bios
	// this permit 32X games with older BIOS version to run correctly
	// Ecco 32X demo needs it
	
	for (i = 0; i < 0x400; i++)
		_32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];
}


/**
 * Do_32X_Frame_No_VDP(): Runs a 32X frame without updating the VDP.
 * @return 1 if successful.
 */
int Do_32X_Frame_No_VDP (void)
{
	int i, j, k, l, p_i, p_j, p_k, p_l, *buf[2];
	int HInt_Counter, HInt_Counter_32X;
	int CPL_PWM;
	
	// If PAL and some VDP register are set, use 240 lines.
	// Otherwise, only 224 lines are visible.
	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
		VDP_Num_Vis_Lines = 240;
	else
		VDP_Num_Vis_Lines = 224;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	CPL_PWM = CPL_M68K * 3;
	
	PWM_Cycles = Cycles_SSH2 = Cycles_MSH2 = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	
	main68k_tripOdometer ();
	z80_Clear_Odo (&M_Z80);
#ifdef __RESULT__
	asm volatile ("call SH2_Clear_Odo"::"c" (&M_SH2));
	asm volatile ("call SH2_Clear_Odo"::"c" (&S_SH2));
#else
	SH2_Clear_Odo (&M_SH2);
	SH2_Clear_Odo (&S_SH2);
#endif
	PWM_Clear_Timer ();
	
	Patch_Codes ();
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	_32X_VDP.State &= ~0x8000;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H
	HInt_Counter_32X = _32X_HIC;
	
	p_i = 84;
	p_j = (p_i * CPL_MSH2) / CPL_M68K;
	p_k = (p_i * CPL_SSH2) / CPL_M68K;
	p_l = p_i * 3;
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		_32X_VDP.State |= 0x6000;
		
		main68k_exec (i - p_i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j - p_j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k - p_k));
#else
		SH2_Exec (&M_SH2, j - p_j);
		SH2_Exec (&S_SH2, k - p_k);
#endif
		PWM_Update_Timer (l - p_l);
		
		VDP_Status &= ~0x0004;	// HBlank = 0
		_32X_VDP.State &= ~0x6000;
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}
		
		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
#ifdef __RESULT__
			if (_32X_MINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (10));
			if (_32X_SINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (10));
#else
			if (_32X_MINT & 0x04)
				SH2_Interrupt (&M_SH2, 10);
			if (_32X_SINT & 0x04)
				SH2_Interrupt (&S_SH2, 10);
#endif
		}
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
#ifdef __RESULT__
			asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
			asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
			SH2_Exec (&M_SH2, j);
			SH2_Exec (&S_SH2, k);
#endif
			PWM_Update_Timer (l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}
		
		main68k_exec (Cycles_M68K);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (Cycles_MSH2));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (Cycles_SSH2));
#else
		SH2_Exec (&M_SH2, Cycles_MSH2);
		SH2_Exec (&S_SH2, Cycles_SSH2);
#endif
		PWM_Update_Timer (PWM_Cycles);
		
#ifdef __RESULT__
		if (Z80_State == 3)
			asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
		if (Z80_State == 3)
			z80_Exec (&M_Z80, Cycles_Z80);
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	
	i = Cycles_M68K + p_i;
	j = Cycles_MSH2 + p_j;
	k = Cycles_SSH2 + p_k;
	l = PWM_Cycles + p_l;
	
	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_MSH2 += CPL_MSH2;
	Cycles_SSH2 += CPL_SSH2;
	Cycles_Z80 += CPL_Z80;
	PWM_Cycles += CPL_PWM;
	if (DMAT_Lenght)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line ();
	}
	
	if (--HInt_Counter_32X < 0)
	{
		HInt_Counter_32X = _32X_HIC;
#ifdef __RESULT__
		if (_32X_MINT & 0x04)
			asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (10));
		if (_32X_SINT & 0x04)
			asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (10));
#else
		if (_32X_MINT & 0x04)
			SH2_Interrupt (&M_SH2, 10);
		if (_32X_SINT & 0x04)
			SH2_Interrupt (&S_SH2, 10);
#endif
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	_32X_VDP.State |= 0xE000;	// VBlank = 1, HBlank = 1, PEN = 1
	
	if (_32X_VDP.State & 0x10000)
		_32X_VDP.State |= 1;
	else
		_32X_VDP.State &= ~1;
	
	_32X_Set_FB ();
	
	while (i < (Cycles_M68K - 360))
	{
		main68k_exec (i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
		SH2_Exec (&M_SH2, j);
		SH2_Exec (&S_SH2, k);
#endif
		PWM_Update_Timer (l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}
	
	main68k_exec (Cycles_M68K - 360);
#ifdef __RESULT__
	if (Z80_State == 3)
		asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
	if (Z80_State == 3)
		z80_Exec (&M_Z80, Cycles_Z80 - 168);
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80 - 168);
#endif
	
	VDP_Status &= ~0x0004;		// HBlank = 0
	_32X_VDP.State &= ~0x4000;
	VDP_Status |= 0x0080;		// V Int happened
	
	VDP_Int |= 0x8;
	Update_IRQ_Line ();
	
#ifdef __RESULT__
	if (_32X_MINT & 0x08)
		asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (12));
	if (_32X_SINT & 0x08)
		asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (12));
#else
	if (_32X_MINT & 0x08)
		SH2_Interrupt (&M_SH2, 12);
	if (_32X_SINT & 0x08)
		SH2_Interrupt (&S_SH2, 12);
#endif
	
	z80_Interrupt (&M_Z80, 0xFF);
	
	while (i < Cycles_M68K)
	{
		main68k_exec (i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
		SH2_Exec (&M_SH2, j);
		SH2_Exec (&S_SH2, k);
#endif
		PWM_Update_Timer (l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}
	
	main68k_exec (Cycles_M68K);
	
#ifdef __RESULT__
	asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (Cycles_MSH2));
	asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (Cycles_SSH2));
#else
	SH2_Exec (&M_SH2, Cycles_MSH2);
	SH2_Exec (&S_SH2, Cycles_SSH2);
#endif
	
	PWM_Update_Timer (PWM_Cycles);
	
#ifdef __RESULT__
	if (Z80_State == 3)
		asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
	if (Z80_State == 3)
		z80_Exec (&M_Z80, Cycles_Z80);
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		_32X_VDP.State |= 0x6000;
		
		main68k_exec (i - p_i);
		
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j - p_j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k - p_k));
#else
		SH2_Exec (&M_SH2, j - p_j);
		SH2_Exec (&S_SH2, k - p_k);
#endif
		PWM_Update_Timer (l - p_l);
		
		VDP_Status &= ~0x0004;	// HBlank = 0
		_32X_VDP.State &= ~0x6000;
		
		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
#ifdef __RESULT__
			if (_32X_MINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (10));
			if (_32X_SINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (10));
#else
			if ((_32X_MINT & 0x04) && (_32X_MINT & 0x80))
				SH2_Interrupt (&M_SH2, 10);
			if ((_32X_SINT & 0x04) && (_32X_SINT & 0x80))
				SH2_Interrupt (&S_SH2, 10);
#endif
		}
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
#ifdef __RESULT__
			asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
			asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
			SH2_Exec (&M_SH2, j);
			SH2_Exec (&S_SH2, k);
#endif
		
			PWM_Update_Timer (l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}
		
		main68k_exec (Cycles_M68K);
		
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (Cycles_MSH2));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (Cycles_SSH2));
#else
		SH2_Exec (&M_SH2, Cycles_MSH2);
		SH2_Exec (&S_SH2, Cycles_SSH2);
#endif
		
		PWM_Update_Timer (PWM_Cycles);
		
#ifdef __RESULT__
		if (Z80_State == 3)
			asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
		if (Z80_State == 3)
			z80_Exec (&M_Z80, Cycles_Z80);
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
	}
	
	PSG_Special_Update ();
	YM2612_Special_Update ();
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	return 1;
}


/**
 * Do_32X_Frame(): Runs a 32X frame.
 * @return 1 if successful.
 */
int Do_32X_Frame (void)
{
	int i, j, k, l, p_i, p_j, p_k, p_l, *buf[2];
	int HInt_Counter, HInt_Counter_32X;
	int CPL_PWM;
	
  	// If PAL and some VDP register are set, use 240 lines.
	// Otherwise, only 224 lines are visible.
	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))
		VDP_Num_Vis_Lines = 240;
	else
		VDP_Num_Vis_Lines = 224;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	CPL_PWM = CPL_M68K * 3;
	
	PWM_Cycles = Cycles_SSH2 = Cycles_MSH2 = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	
	main68k_tripOdometer ();
	z80_Clear_Odo (&M_Z80);
#ifdef __RESULT__
	asm volatile ("call SH2_Clear_Odo"::"c" (&M_SH2));
	asm volatile ("call SH2_Clear_Odo"::"c" (&S_SH2));
#else
	SH2_Clear_Odo (&M_SH2);
	SH2_Clear_Odo (&S_SH2);
#endif
	PWM_Clear_Timer ();
	
	Patch_Codes ();
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	_32X_VDP.State &= ~0x8000;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H
	HInt_Counter_32X = _32X_HIC;
	
	p_i = 84;
	p_j = (p_i * CPL_MSH2) / CPL_M68K;
	p_k = (p_i * CPL_SSH2) / CPL_M68K;
	p_l = p_i * 3;
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		_32X_VDP.State |= 0x6000;
		
		main68k_exec (i - p_i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j - p_j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k - p_k));
#else
		SH2_Exec (&M_SH2, j - p_j);
		SH2_Exec (&S_SH2, k - p_k);
#endif
		PWM_Update_Timer (l - p_l);
		
		VDP_Status &= ~0x0004;	// HBlank = 0
		_32X_VDP.State &= ~0x6000;
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line ();
		}
		
		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
#ifdef __RESULT__
			if (_32X_MINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (10));
			if (_32X_SINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (10));
#else
			if (_32X_MINT & 0x04)
				SH2_Interrupt (&M_SH2, 10);
			if (_32X_SINT & 0x04)
				SH2_Interrupt (&S_SH2, 10);
#endif
		}
		
		Render_Line_32X ();
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
#ifdef __RESULT__
			asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
			asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
			SH2_Exec (&M_SH2, j);
			SH2_Exec (&S_SH2, k);
#endif
			PWM_Update_Timer (l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}
		
		main68k_exec (Cycles_M68K);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (Cycles_MSH2));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (Cycles_SSH2));
#else
		SH2_Exec (&M_SH2, Cycles_MSH2);
		SH2_Exec (&S_SH2, Cycles_SSH2);
#endif
		PWM_Update_Timer (PWM_Cycles);
		
#ifdef __RESULT__
		if (Z80_State == 3)
			asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
		if (Z80_State == 3)
			z80_Exec (&M_Z80, Cycles_Z80);
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	
	i = Cycles_M68K + p_i;
	j = Cycles_MSH2 + p_j;
	k = Cycles_SSH2 + p_k;
	l = PWM_Cycles + p_l;
	
	Fix_Controllers ();
	Cycles_M68K += CPL_M68K;
	Cycles_MSH2 += CPL_MSH2;
	Cycles_SSH2 += CPL_SSH2;
	Cycles_Z80 += CPL_Z80;
	PWM_Cycles += CPL_PWM;
	if (DMAT_Lenght)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line ();
	}
	
	if (--HInt_Counter_32X < 0)
	{
		HInt_Counter_32X = _32X_HIC;
#ifdef __RESULT__
		if (_32X_MINT & 0x04)
			asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (10));
		if (_32X_SINT & 0x04)
			asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (10));
#else
		if (_32X_MINT & 0x04)
			SH2_Interrupt (&M_SH2, 10);
		if (_32X_SINT & 0x04)
			SH2_Interrupt (&S_SH2, 10);
#endif
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	_32X_VDP.State |= 0xE000;	// VBlank = 1, HBlank = 1, PEN = 1
	
	if (_32X_VDP.State & 0x10000)
		_32X_VDP.State |= 1;
	else
		_32X_VDP.State &= ~1;
	
	_32X_Set_FB ();
	
	while (i < (Cycles_M68K - 360))
	{
		main68k_exec (i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
		SH2_Exec (&M_SH2, j);
		SH2_Exec (&S_SH2, k);
#endif
		PWM_Update_Timer (l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}
	
	main68k_exec (Cycles_M68K - 360);
#ifdef __RESULT__
	if (Z80_State == 3)
		asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80 - 168));
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80 - 168);
#else
	if (Z80_State == 3)
		z80_Exec (&M_Z80, Cycles_Z80 - 168);
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80 - 168);
#endif
	
	//if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	//else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);
	
	VDP_Status &= ~0x0004;		// HBlank = 0
	_32X_VDP.State &= ~0x4000;
	VDP_Status |= 0x0080;		// V Int happened
	
	VDP_Int |= 0x8;
	Update_IRQ_Line ();
	
#ifdef __RESULT__
	if (_32X_MINT & 0x08)
		asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (12));
	if (_32X_SINT & 0x08)
		asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (12));
#else
	if (_32X_MINT & 0x08)
		SH2_Interrupt (&M_SH2, 12);
	if (_32X_SINT & 0x08)
		SH2_Interrupt (&S_SH2, 12);
#endif
	
	z80_Interrupt (&M_Z80, 0xFF);
	
	while (i < Cycles_M68K)
	{
		main68k_exec (i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
		SH2_Exec (&M_SH2, j);
		SH2_Exec (&S_SH2, k);
#endif
		
		PWM_Update_Timer (l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}
	
	main68k_exec (Cycles_M68K);
#ifdef __RESULT__
	asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (Cycles_MSH2));
	asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (Cycles_MSH2));
#else
	SH2_Exec (&M_SH2, Cycles_MSH2);
	SH2_Exec (&S_SH2, Cycles_MSH2);
#endif
	//SH2_Exec(&M_SH2, Cycles_MSH2);
	//SH2_Exec(&S_SH2, Cycles_SSH2);
	
	PWM_Update_Timer (PWM_Cycles);

#ifdef __RESULT__
	if (Z80_State == 3)
		asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
	if (Z80_State == 3)
		z80_Exec (&M_Z80, Cycles_Z80);
	else
		z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
	//if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	//else z80_Set_Odo(&M_Z80, Cycles_Z80);

	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);
		
		Fix_Controllers ();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght)
			main68k_addCycles (Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		_32X_VDP.State |= 0x6000;
		
		main68k_exec (i - p_i);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j - p_j));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k - p_k));
#else
		SH2_Exec (&M_SH2, j - p_j);
		SH2_Exec (&S_SH2, k - p_k);
#endif
		PWM_Update_Timer (l - p_l);
		
		VDP_Status &= ~0x0004;	// HBlank = 0
		_32X_VDP.State &= ~0x6000;
		
		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
#ifdef __RESULT__
			if (_32X_MINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&M_SH2), "d" (10));
			if (_32X_SINT & 0x04)
				asm volatile ("call SH2_Interrupt"::"c" (&S_SH2), "d" (10));
#else
			if ((_32X_MINT & 0x04) && (_32X_MINT & 0x80))
				SH2_Interrupt (&M_SH2, 10);
			if ((_32X_SINT & 0x04) && (_32X_SINT & 0x80))
				SH2_Interrupt (&S_SH2, 10);
#endif
		}
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec (i);
#ifdef __RESULT__
			asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (j));
			asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (k));
#else
			SH2_Exec (&M_SH2, j);
			SH2_Exec (&S_SH2, k);
#endif
			//SH2_Exec(&M_SH2, j);
			//SH2_Exec(&S_SH2, k);
			PWM_Update_Timer (l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}
		
		main68k_exec (Cycles_M68K);
#ifdef __RESULT__
		asm volatile ("call SH2_Exec"::"c" (&M_SH2), "d" (Cycles_MSH2));
		asm volatile ("call SH2_Exec"::"c" (&S_SH2), "d" (Cycles_SSH2));
#else
		SH2_Exec (&M_SH2, Cycles_MSH2);
		SH2_Exec (&S_SH2, Cycles_SSH2);
#endif
		//SH2_Exec(&M_SH2, Cycles_MSH2);
		//SH2_Exec(&S_SH2, Cycles_SSH2);
		PWM_Update_Timer (PWM_Cycles);
		
#ifdef __RESULT__
		if (Z80_State == 3)
			asm volatile ("call z80_Exec"::"c" (&M_Z80), "d" (Cycles_Z80));
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#else
		if (Z80_State == 3)
			z80_Exec (&M_Z80, Cycles_Z80);
		else
			z80_Set_Odo (&M_Z80, Cycles_Z80);
#endif
		//if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		//else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}
	
	PSG_Special_Update ();
	YM2612_Special_Update ();
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		Update_WAV_Dump ();
	if (GYM_Dumping)
		Update_GYM_Dump ((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	return 1;
}
