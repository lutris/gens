/**
 * Gens: Genesis (Mega Drive) initialization and main loop code.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include "gens.hpp"
#include "g_md.hpp"
#include "g_main.hpp"
#include "g_update.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "mdZ80/mdZ80.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/io/io.h"
#include "util/file/save.hpp"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "libgsft/gsft_byteswap.h"

#include "gens_ui.hpp"

// Video, Audio.
#include "video/vdraw.h"
#include "audio/audio.h"

// MDP Event Manager.
#include "plugins/eventmgr.hpp"


int congratulations = 0;


/**
 * Detect_Country_Genesis(): Detect the country code of a Genesis game.
 * @param MD_ROM Pointer to the MD_ROM struct.
 */
void Detect_Country_Genesis(ROM_t* MD_ROM)
{
	static const int c_tab[3]  = { 4, 1, 8 };
	static const int gm_tab[3] = { 1, 0, 1 };
	static const int cm_tab[3] = { 0, 0, 1 };
	
	int i, coun = 0;
	char c;
	
	if (!MD_ROM)
		return;
	
	if (!strncasecmp(MD_ROM->Country_Codes, "eur", 3))
		coun |= 8;
	else if (!strncasecmp(MD_ROM->Country_Codes, "usa", 3))
		coun |= 4;
	else if (!strncasecmp(MD_ROM->Country_Codes, "jap", 3))
		coun |= 1;
	else
	{
		for (i = 0; i < 4; i++)
		{
			c = toupper(MD_ROM->Country_Codes[i]);
			
			if (c == 'U')
				coun |= 4;
			else if (c == 'J')
				coun |= 1;
			else if (c == 'E')
				coun |= 8;
			else if (c < 16)
				coun |= c;
			else if ((c >= '0') && (c <= '9'))
				coun |= c - '0';
			else if ((c >= 'A') && (c <= 'F'))
				coun |= c - 'A' + 10;
		}
	}
	
	// Check what country should be set.
	if (coun & c_tab[Country_Order[0]])
	{
		Game_Mode = gm_tab[Country_Order[0]];
		CPU_Mode = cm_tab[Country_Order[0]];
	}
	else if (coun & c_tab[Country_Order[1]])
	{
		Game_Mode = gm_tab[Country_Order[1]];
		CPU_Mode = cm_tab[Country_Order[1]];
	}
	else if (coun & c_tab[Country_Order[2]])
	{
		Game_Mode = gm_tab[Country_Order[2]];
		CPU_Mode = cm_tab[Country_Order[2]];
	}
	else if (coun & 2)
	{
		Game_Mode = 0;
		CPU_Mode = 1;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
	}
	
	if (Game_Mode)
	{
		if (CPU_Mode)
			vdraw_text_write("Europe system (50 FPS)", 1500);
		else
			vdraw_text_write("USA system (60 FPS)", 1500);
	}
	else
	{
		if (CPU_Mode)
			vdraw_text_write("Japan system (50 FPS)", 1500);
		else
			vdraw_text_write("Japan system (60 FPS)", 1500);
	}
	
	if (CPU_Mode)
	{
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
	}
	else
	{
		_32X_VDP.Mode |= 0x8000;
		VDP_Status &= 0xFFFE;
	}
}


/**
 * Init_Genesis_Bios(): Initialize the Genesis BIOS.
 * TODO: Fix this. It doesn't seem to work properly.
 */
void Init_Genesis_Bios(void)
{
	// TODO: Compressor support.
	FILE *f;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Clear the TMSS firmware buffer.
	memset(&Genesis_Rom[0], 0x00, sizeof(Genesis_Rom));
	
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_MD_TMSS;
	if ((f = fopen(BIOS_Filenames.MD_TMSS, "rb")))
	{
		size_t n = fread(&Genesis_Rom[0], 1, sizeof(Genesis_Rom), f);
		fclose(f);
		be16_to_cpu_array(&Genesis_Rom[0], n);
		Rom_Size = n;
	}
	else
	{
		// No Genesis TMSS ROM.
		Rom_Size = sizeof(Genesis_Rom);
	}
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_TMSS;
	
	memcpy(Rom_Data, Genesis_Rom, Rom_Size);
	Game_Mode = 0;
	CPU_Mode = 0;
	VDP_Num_Vis_Lines = 224;
	
	// Set the clock frequencies.
	Set_Clock_Freq(0);
	
	// Reset the CPUs.
	M68K_Reset(0);
	Z80_Reset();
	VDP_Reset();
}


/**
 * Init_Genesis_SRAM(): Initialize the Genesis SRAM.
 * @param MD_ROM Pointer to the MD_ROM struct.
 */
void Init_Genesis_SRAM(ROM_t* MD_ROM)
{
	if (MD_ROM->SRAM_Info[0] == 'R' &&
	    MD_ROM->SRAM_Info[1] == 'A' &&
	    (MD_ROM->SRAM_Info[2] & 0x40))
	{
		// SRAM specified in the ROM header. Use this address.
		// SRAM starting position must be a multiple of 0x080000.
		SRAM_Start = MD_ROM->SRAM_Start_Address & 0xF80000;
		SRAM_End = MD_ROM->SRAM_End_Address;
	}
	else
	{
		// Default SRAM address.
		SRAM_Start = 0x200000;
		SRAM_End = 0x200000 + (64 * 1024) - 1;
	}
	
	// Check for invalid SRAM addresses.
	if ((SRAM_Start > SRAM_End) ||
	    ((SRAM_End - SRAM_Start) >= (64 * 1024)))
	{
		// Invalid SRAM information. Use the default SRAM end value.
		SRAM_End = SRAM_Start + (64 * 1024) - 1;
	}
	
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
	
	//sprintf(Str_Err, "deb = %.8X end = %.8X", SRAM_Start, SRAM_End);
	//MessageBox(NULL, Str_Err, "", MB_OK);
	
	// Check for custom SRAM mode.
	// TODO: Document what "custom" mode is for.
	if ((SRAM_End - SRAM_Start) <= 2)
		SRAM_Custom = 1;
	else
		SRAM_Custom = 0;
	
	// Load the SRAM file.
	Savestate::LoadSRAM();
}


/**
 * Init_Genesis(): Initialize the Genesis with the specified ROM image.
 * @param MD_ROM ROM image struct.
 * @return 1 if successful.
 */
int Init_Genesis(ROM_t* MD_ROM)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	Flag_Clr_Scr = 1;
	Settings.Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	Controller_1_COM = Controller_2_COM = 0;
#ifdef GENS_DEBUGGER
	STOP_DEBUGGING();
#endif

	
#if 0	// TODO: Replace with MDP "exclusive mode" later.
	if (!Kaillera_Client_Running)
#endif
	Init_Genesis_SRAM(MD_ROM);
	
	// Check what country code should be used.
	// TODO: Get rid of magic numbers.
	switch (Country)
	{
		default:
		case -1: // Autodetection.
			Detect_Country_Genesis(MD_ROM);
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
	
	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)
	
	// Byteswap the ROM data.
	be16_to_cpu_array(Rom_Data, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_ROM;
	
	// Reset all CPUs and other components.
	M68K_Reset(0);
	Z80_Reset();
	VDP_Reset();
	
	// Initialize the controller state.
	Init_Controllers();
	
	// Set clock rates depending on the CPU mode (NTSC / PAL).
	Set_Clock_Freq(0);
	
	// If auto-fix checksum is enabled, fix the ROM checksum.
	if (Auto_Fix_CS)
		ROM::fixChecksum();
	
	// Initialize sound.
	if (audio_get_enabled())
	{
		audio_end();
		
		if (audio_init(AUDIO_BACKEND_DEFAULT))
			audio_set_enabled(false);
		else
		{
			if (audio_play_sound)
				audio_play_sound();
		}
	}
	
	Reset_Update_Timers();
	
	// Set the appropriate frame update function pointers.
	Update_Frame = Do_Genesis_Frame;
	Update_Frame_Fast = Do_Genesis_Frame_No_VDP;
	
	return 1;
}


/**
 * Reset_Genesis(): Resets the Genesis.
 */
void Reset_Genesis(void)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	Controller_1_COM = Controller_2_COM = 0;
	Settings.Paused = 0;
	
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
	M68K_Reset(0);
	Z80_Reset();
	VDP_Reset();
	YM2612_Reset();
	
	// Initialize the controller state.
	Init_Controllers();
	
	if (CPU_Mode)
		VDP_Status |= 1;
	else
		VDP_Status &= ~1;
	
	if (Auto_Fix_CS)
		ROM::fixChecksum();
}


/**
 * Do_VDP_Only(): Updates the Genesis VDP.
 * @return 0 if successful. TODO: Remove unnecessary return values.
 */
int Do_VDP_Only(void)
{
	// Set the number of visible lines.
	VDP_SET_VISIBLE_LINES();
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		Render_Line();
	}

	return 0;
}


#define CONGRATULATIONS_PRECHECK				\
unsigned int old_pc = main68k_context.pc;			\
do {								\
	if (congratulations == 1 && old_pc < Rom_Size)		\
	{							\
		congratulations = 2;				\
		Rom_Data[old_pc] = ~Rom_Data[old_pc];		\
		Rom_Data[old_pc + 1] = ~Rom_Data[old_pc + 1];	\
	}							\
} while (0)

#define CONGRATULATIONS_POSTCHECK				\
do {								\
	if (congratulations == 2)				\
	{							\
		congratulations = 3;				\
		Rom_Data[old_pc] = ~Rom_Data[old_pc];		\
		Rom_Data[old_pc + 1] = ~Rom_Data[old_pc + 1];	\
	}							\
	congratulations = 0;					\
} while (0)


/**
 * T_gens_do_MD_frame(): Do an MD frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
static inline int T_gens_do_MD_frame(void)
{
	int *buf[2];
	int HInt_Counter;
	
	// Set the number of visible lines.
	VDP_SET_VISIBLE_LINES();
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	mdZ80_clear_odo(&M_Z80);
	
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec (Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			VDP_Update_IRQ_Line();
		}
		
		if (VDP)
		{
			// VDP needs to be updated.
			Render_Line();
		}
		
		main68k_exec(Cycles_M68K);
		Z80_EXEC(0);
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	
	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (DMAT_Length)
		main68k_addCycles(Update_DMA());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		VDP_Update_IRQ_Line();
	}
	
	CONGRATULATIONS_PRECHECK;
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec(Cycles_M68K - 360);
	Z80_EXEC(168);
	CONGRATULATIONS_POSTCHECK;
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	
	VDP_Int |= 0x8;
	VDP_Update_IRQ_Line();
	mdZ80_interrupt(&M_Z80, 0xFF);
	
	main68k_exec(Cycles_M68K);
	Z80_EXEC(0);
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		main68k_exec(Cycles_M68K);
		Z80_EXEC(0);
	}
	
	PSG_Special_Update();
	YM2612_Special_Update();
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
	
	// Raise the MDP_EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	if (bppMD == 32)
		post_frame.md_screen = &MD_Screen32[8];
	else
		post_frame.md_screen = &MD_Screen[8];
	post_frame.width = (vdp_isH40() ? 320 : 256);
	post_frame.height = VDP_Num_Vis_Lines;
	post_frame.pitch = 336;
	post_frame.bpp = bppMD;
	
	EventMgr::RaiseEvent(MDP_EVENT_POST_FRAME, &post_frame);
	
	return 1;
}


int Do_Genesis_Frame_No_VDP(void)
{
	return T_gens_do_MD_frame<false>();
}
int Do_Genesis_Frame(void)
{
	return T_gens_do_MD_frame<true>();
}
