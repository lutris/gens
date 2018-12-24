/**
 * Gens: Sega CD (Mega CD) initialization and main loop code.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C includes.
#include <stdint.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

#include "gens.hpp"
#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_main.hpp"
#include "g_update.hpp"
#include "options.hpp"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "mdZ80/mdZ80.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/io/io.h"
#include "util/file/save.hpp"
#include "segacd/cd_sys.hpp"
#include "segacd/lc89510.h"
#include "gens_core/gfx/gfx_cd.h"
#include "gens_core/sound/pcm.h"
#include "segacd/cd_sys.hpp"
#include "segacd/cd_file.h"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "libgsft/gsft_byteswap.h"

#include "ui/gens_ui.hpp"

// Audio Handler.
#include "audio/audio.h"

// MDP Event Manager.
#include "plugins/eventmgr.hpp"


// SegaCD disc header.
// Used for hard reset to know the game name.
static unsigned char SegaCD_Header[512];


/**
 * Detect_Country_SegaCD(): Detect the country code of a SegaCD game.
 * @return BIOS filename for the required country.
 */
const char* Detect_Country_SegaCD(void)
{
	if (SegaCD_Header[0x10B] == 0x64)
	{
		Game_Mode = 1;
		CPU_Mode = 1;
		return BIOS_Filenames.MegaCD_EU;
	}
	else if (SegaCD_Header[0x10B] == 0xA1)
	{
		Game_Mode = 0;
		CPU_Mode = 0;
		return BIOS_Filenames.MegaCD_JP;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
		return BIOS_Filenames.SegaCD_US;
	}
}


/**
 * Init_SegaCD(): Initialize the Sega CD with the specified ISO image.
 * @param iso_name Filename of the ISO, or NULL for physical CD-ROM.
 * @return 1 if successful; 0 if an error occurred.
 */
int Init_SegaCD(const char* iso_name)
{
	const char* BIOS_To_Use;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	GensUI::setWindowTitle_Init("SegaCD", false);
	
	// Clear the SCD struct.
	memset(&SCD, 0x00, sizeof(SCD));
	
	if (Reset_CD((char*)SegaCD_Header, iso_name))
	{
		// Error occured while setting up Sega CD emulation.
		// TODO: Show a message box.
		GensUI::setWindowTitle_Idle();
		return 0;
	}
	
	// Check what BIOS should be used.
	// TODO: Get rid of magic numbers.
	switch (Country)
	{
		default:
		case -1: // Autodetection.
			BIOS_To_Use = Detect_Country_SegaCD();
			break;
		
		case 0: // Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			BIOS_To_Use = BIOS_Filenames.MegaCD_JP;
			break;
		
		case 1: // US (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			BIOS_To_Use = BIOS_Filenames.SegaCD_US;
			break;
		
		case 2: // Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			BIOS_To_Use = BIOS_Filenames.MegaCD_EU;
			break;
		
		case 3: // Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			BIOS_To_Use = BIOS_Filenames.MegaCD_JP;
			break;
	}
	
	// Attempt to load the Sega CD BIOS.
	if (ROM::loadSegaCD_BIOS(BIOS_To_Use) == NULL)
	{
		GensUI::msgBox(
			"Your Sega CD BIOS files aren't configured correctly.\n"
			"Go to menu 'Options -> BIOS/Misc Files' to set them up.",
			"BIOS Configuration Error"
			);
		GensUI::setWindowTitle_Idle();
		return 0;
	}
	
	// Update the CD-ROM name.
	ROM::updateCDROMName(SegaCD_Header, Game_Mode);
	
	// Update the window title with the game name.
	Options::setGameName(1);
	
	Flag_Clr_Scr = 1;
	Settings.Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	BRAM_Ex_State &= 0x100;
	Controller_1_COM = Controller_2_COM = 0;
#ifdef GENS_DEBUGGING
	STOP_DEBUGGING();
#endif
	
	// Set clock rates depending on the CPU mode (NTSC / PAL).
	Set_Clock_Freq(1);
	
	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)
	
	// TODO: Why are these two bytes set to 0xFF?
	Rom_Data[0x72] = 0xFF;
	Rom_Data[0x73] = 0xFF;
	be16_to_cpu_array(Rom_Data, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_ROM;
	
	// Reset all CPUs and other components.
	M68K_Reset(2);
	S68K_Reset();
	Z80_Reset();
	VDP_Reset();
	LC89510_Reset();
	Init_RS_GFX();
	
	PCM_Init(audio_get_sound_rate());
	
	// Initialize the controller state.
	Init_Controllers();
	
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
	
	// Initialize BRAM.
	Savestate::LoadBRAM();
	
	Reset_Update_Timers();
	
	if (SegaCD_Accurate)
	{
		Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
	}
	else
	{
		Update_Frame = Do_SegaCD_Frame;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
	}
	
	return 1;
}


/**
 * Reload_SegaCD(): Reloads the Sega CD.
 * @param iso_name Filename of the ISO, or NULL for physical CD-ROM.
 * @return 1 if successful.
 */
int Reload_SegaCD(const char* iso_name)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Save the current BRAM.
	Savestate::SaveBRAM();
	
	// Set the window title to "Reinitializing..."
	GensUI::setWindowTitle_Init(((CPU_Mode == 0 && Game_Mode == 1) ? "SegaCD" : "MegaCD"), true);
	
	// Update the CD-ROM name.
	Reset_CD((char*)SegaCD_Header, iso_name);
	ROM::updateCDROMName(SegaCD_Header, Game_Mode);
	
	// Update the window title with the game name.
	Options::setGameName(1);
	
	// Load the new BRAM.
	Savestate::LoadBRAM();
	
	return 1;
}


/**
 * Reset_SegaCD(): Resets the Sega CD.
 */
void Reset_SegaCD(void)
{
	char *BIOS_To_Use;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	if (Game_Mode == 0)
		BIOS_To_Use = BIOS_Filenames.MegaCD_JP;
	else if (CPU_Mode)
		BIOS_To_Use = BIOS_Filenames.MegaCD_EU;
	else
		BIOS_To_Use = BIOS_Filenames.SegaCD_US;
	
	// Load the BIOS file.
	string z_filename;
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_MD_ROM;
	if (ROM::loadROM(BIOS_To_Use, z_filename, Game) <= 0)
	{
		GensUI::msgBox(
			"Your Sega CD BIOS files aren't configured correctly.\n"
			"Go to menu 'Options -> BIOS/Misc Files' to set them up.",
			"BIOS Configuration Error"
			);
		return;
	}
	
	Controller_1_COM = Controller_2_COM = 0;
	SRAM_ON = 0;
	SRAM_Write = 0;
	Settings.Paused = 0;
	BRAM_Ex_State &= 0x100;
	
	// Update the CD-ROM name.
	ROM::updateCDROMName(SegaCD_Header, Game_Mode);
	
	// Update the window title with the game name.
	Options::setGameName(1);
	
	// TODO: Why are these two bytes set to 0xFF?
	Rom_Data[0x72] = 0xFF;
	Rom_Data[0x73] = 0xFF;
	be16_to_cpu_array(Rom_Data, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_ROM;
	
	// Reset all CPUs and other components.
	M68K_Reset(2);
	S68K_Reset();
	Z80_Reset();
	LC89510_Reset();
	VDP_Reset();
	Init_RS_GFX();
	PCM_Reset();
	YM2612_Reset();
	
	// Initialize the controller state.
	Init_Controllers();
	
	if (CPU_Mode)
		VDP_Status |= 1;
	else
		VDP_Status &= ~1;
}


/**
 * draw_SegaCD_LED(): Draw a SegaCD LED.
 * @param screen Screen buffer.
 * @param color LED color.
 * @param startCol Left-most column of the LED.
 */
template<typename Pixel>
static inline void T_Draw_SegaCD_LED(Pixel* screen, Pixel color, unsigned short startCol)
{
	screen[(336*220) + 12 + startCol] = color;
	screen[(336*220) + 13 + startCol] = color;
	screen[(336*220) + 14 + startCol] = color;
	screen[(336*220) + 15 + startCol] = color;
	
	screen[(336*222) + 12 + startCol] = color;
	screen[(336*222) + 13 + startCol] = color;
	screen[(336*222) + 14 + startCol] = color;
	screen[(336*222) + 15 + startCol] = color;
}


/**
 * SegaCD_Display_LED(): Display the LEDs on the Sega CD interface.
 */
static void SegaCD_Display_LED(void)
{
	// TODO: Optimize this function.
	// TODO: Draw the LEDs on the screen buffer, not the MD screen.
	
	// Ready LED (green)
	static const uint16_t ledReady_15 = 0x03E0;
	static const uint16_t ledReady_16 = 0x07C0;
	static const uint32_t ledReady_32 = 0x00F800;
	
	// Access LED (red)
	static const uint16_t ledAccess_15 = 0x7C00;
	static const uint16_t ledAccess_16 = 0xF800;
	static const uint32_t ledAccess_32 = 0xF80000;
	
	// Ready LED
	if (LED_Status & 2)
	{
		if (bppMD == 15)
			T_Draw_SegaCD_LED(MD_Screen, ledReady_15, 0);
		else if (bppMD == 16)
			T_Draw_SegaCD_LED(MD_Screen, ledReady_16, 0);
		else //if (bppMD == 32)
			T_Draw_SegaCD_LED(MD_Screen32, ledReady_32, 0);
	}
	
	// Access LED
	if (LED_Status & 1)
	{
		if (bppMD == 15)
			T_Draw_SegaCD_LED(MD_Screen, ledAccess_15, 8);
		else if (bppMD == 16)
			T_Draw_SegaCD_LED(MD_Screen, ledAccess_16, 8);
		else //if (bppMD == 32)
			T_Draw_SegaCD_LED(MD_Screen32, ledAccess_32, 8);
	}
}


/**
 * T_gens_do_MCD_frame(): Do an MCD frame.
 * @param VDP If true, VDP is updated.
 * @param perfect_sync If true, use perfect synchronization.
 */
template<bool VDP, bool perfect_sync>
static inline int T_gens_do_MCD_frame(void)
{
	int *buf[2], i, j;
	int HInt_Counter;
	
	// Set the number of visible lines.
	VDP_SET_VISIBLE_LINES();

	CPL_S68K = 795;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	sub68k_tripOdometer();
	mdZ80_clear_odo(&M_Z80);
	
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			PCM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();
		
		if (perfect_sync)
		{
			i = Cycles_M68K + 24;
			j = Cycles_S68K + 39;
		}
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		
		if (!perfect_sync)
		{
			// Perfect Sync is disabled.
			main68k_exec(Cycles_M68K - 404);
		}
		else
		{
			// Perfect sync is enabled.
			// Use instruction by instruction execution.
			while (i < (Cycles_M68K - 404))
			{
				main68k_exec(i);
				i += 24;
				
				if (j < (Cycles_S68K - 658))
				{
					sub68k_exec(j);
					j += 39;
				}
			}
			
			main68k_exec(Cycles_M68K - 404);
			sub68k_exec(Cycles_S68K - 658);
		}
		
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
		
		if (perfect_sync)
		{
			// Perfect sync is enabled.
			// Use instruction by instruction execution.
			while (i < Cycles_M68K)
			{
				main68k_exec(i);
				i += 24;
				
				if (j < Cycles_S68K)
				{
					sub68k_exec(j);
					j += 39;
				}
			}
		}
		
		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);
		Z80_EXEC(0);
		
		Update_SegaCD_Timer();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable)
		PCM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert();
	
	if (perfect_sync)
	{
		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;
	}
	
	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1)
		Cycles_S68K += CPL_S68K;
	if (DMAT_Length)
		main68k_addCycles (Update_DMA ());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		VDP_Update_IRQ_Line();
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	
	if (perfect_sync)
	{
		// Perfect sync is enabled.
		// Use instruction by instruction execution.
		while (i < (Cycles_M68K - 360))
		{
			main68k_exec(i);
			i += 24;
			
			if (j < (Cycles_S68K - 586))
			{
				sub68k_exec(j);
				j += 39;
			}
		}
	}
	
	main68k_exec(Cycles_M68K - 360);
	sub68k_exec(Cycles_S68K - 586);
	Z80_EXEC(168);
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	VDP_Int |= 0x8;
	VDP_Update_IRQ_Line();
	mdZ80_interrupt(&M_Z80, 0xFF);
	
	if (perfect_sync)
	{
		// Perfect sync is enabled.
		// Use instruction by instruction execution.
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			i += 24;
			
			if (j < Cycles_S68K)
			{
				sub68k_exec(j);
				j += 39;
			}
		}
	}
	
	main68k_exec(Cycles_M68K);
	sub68k_exec(Cycles_S68K);
	Z80_EXEC(0);
	
	Update_SegaCD_Timer();
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable)
			PCM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();
		
		if (perfect_sync)
		{
			i = Cycles_M68K + 24;
			j = Cycles_S68K + 39;
		}
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1)
			Cycles_S68K += CPL_S68K;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA ());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		
		if (!perfect_sync)
		{
			// Perfect sync is disabled.
			main68k_exec(Cycles_M68K - 404);
			VDP_Status &= 0xFFFB;	// HBlank = 0
		}
		else
		{
			// Perfect sync is enabled.
			// Use instruction by instruction execution.
			while (i < (Cycles_M68K - 404))
			{
				main68k_exec(i);
				i += 24;
				
				if (j < (Cycles_S68K - 658))
				{
					sub68k_exec(j);
					j += 39;
				}
			}
			
			main68k_exec(Cycles_M68K - 404);
			sub68k_exec(Cycles_S68K - 658);
			
			VDP_Status &= 0xFFFB;	// HBlank = 0
			
			while (i < Cycles_M68K)
			{
				main68k_exec(i);
				i += 24;
				
				if (j < Cycles_S68K)
				{
					sub68k_exec(j);
					j += 39;
				}
			}
		}
		
		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);
		Z80_EXEC(0);
		
		Update_SegaCD_Timer();
	}
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	PSG_Special_Update();
	YM2612_Special_Update();
	Update_CD_Audio(buf, audio_seg_length);
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
	
	if (VDP && Show_LED)
		SegaCD_Display_LED();
	
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


int Do_SegaCD_Frame_No_VDP(void)
{
	return T_gens_do_MCD_frame<false, false>();
}
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void)
{
	return T_gens_do_MCD_frame<false, true>();
}
int Do_SegaCD_Frame(void)
{
	return T_gens_do_MCD_frame<true, false>();
}
int Do_SegaCD_Frame_Cycle_Accurate(void)
{
	return T_gens_do_MCD_frame<true, true>();
}
