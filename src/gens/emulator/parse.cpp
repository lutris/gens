/***************************************************************************
 * Gens: Command line parser.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

// New video layer.
#include "video/v_draw.hpp"

#include "gens.h"
#include "g_main.hpp"
#include "port/port.h"
#include "util/file/save.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"
#include "sdllayer/g_sdlsound.h"
#include "sdllayer/g_sdlinput.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"
#include "g_palette.h"
#include "util/file/rom.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "util/gfx/scrshot.h"
#include "util/file/ggenie.h"
#include "gens_core/io/io.h"
#include "gens_core/misc/misc.h"
#include "segacd/cd_sys.hpp"
#include "ui_proxy.hpp"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.h"
#endif

// Include this *last* to avoid naming conflicts.
#include "parse.hpp"

#define print_usage(option, helpmsg) fprintf(stderr, MM option " : " helpmsg "\n")
#define print_usage2(option, helpmsg) fprintf(stderr, MM ENABLE "-" option ", " MM DISABLE "-" option " : " helpmsg "\n")

static const struct option long_options[] =
{
	{GAME,			required_argument, 0, 0},
	{ROMPATH,		required_argument, 0, 0},
	{SAVEPATH,		required_argument, 0, 0},
	{SRAMPATH,		required_argument, 0, 0},
	{BRAMPATH,		required_argument, 0, 0},
	{DUMPPATH,		required_argument, 0, 0},
	{DUMPGYMPATH,		required_argument, 0, 0},
	{SCRSHTPATH,		required_argument, 0, 0},
	{PATPATH,		required_argument, 0, 0},
	{IPSPATH,		required_argument, 0, 0},
	{GCOFFPATH,		required_argument, 0, 0},
	{GENSMANPATH,		required_argument, 0, 0},
	{GENBIOS,		required_argument, 0, 0},
	{USABIOS,		required_argument, 0, 0},
	{EURBIOS,		required_argument, 0, 0},
	{JAPBIOS,		required_argument, 0, 0},
	{_32X68kBIOS,		required_argument, 0, 0},
	{_32XMBIOS,		required_argument, 0, 0},
	{_32XSBIOS,		required_argument, 0, 0},
	{FS,			no_argument, 0, 0},
	{WINDOW,		no_argument, 0, 0},
	{RENDERMODE,		required_argument, 0, 0},
	{STRETCH_ENABLE,	no_argument, 0, 0},
	{STRETCH_DISABLE,	no_argument, 0, 0},
	{SWBLIT_ENABLE,		no_argument, 0, 0},
	{SWBLIT_DISABLE,	no_argument, 0, 0},
	{CONTRAST,		required_argument, 0, 0},
	{BRIGHTNESS,		required_argument, 0, 0},
	{GREYSCALE_ENABLE,	no_argument, 0, 0},
	{GREYSCALE_DISABLE,	no_argument, 0, 0},
	{INVERT_ENABLE,		no_argument, 0, 0},
	{INVERT_DISABLE,	no_argument, 0, 0},
	{SPRITELIMIT_ENABLE,	no_argument, 0, 0},
	{SPRITELIMIT_DISABLE,	no_argument, 0, 0},
	{FRAMESKIP,		required_argument, 0, 0},
	{SOUND_ENABLE,		no_argument, 0, 0},
	{SOUND_DISABLE,		no_argument, 0, 0},
	{SOUNDRATE,		required_argument, 0, 0},
	{STEREO_ENABLE,		no_argument, 0, 0},
	{STEREO_DISABLE,	no_argument, 0, 0},
	{Z80_ENABLE,		no_argument, 0, 0},
	{Z80_DISABLE,		no_argument, 0, 0},
	{YM2612_ENABLE,		no_argument, 0, 0},
	{YM2612_DISABLE,	no_argument, 0, 0},
	{PSG_ENABLE,		no_argument, 0, 0},
	{PSG_DISABLE,		no_argument, 0, 0},
	{DAC_ENABLE,		no_argument, 0, 0},
	{DAC_DISABLE,		no_argument, 0, 0},
	{PCM_ENABLE,		no_argument, 0, 0},
	{PCM_DISABLE,		no_argument, 0, 0},
	{PWM_ENABLE,		no_argument, 0, 0},
	{PWM_DISABLE,		no_argument, 0, 0},
	{CDDA_ENABLE,		no_argument, 0, 0},
	{CDDA_DISABLE,		no_argument, 0, 0},
	{PSGIMPROVED_ENABLE,	no_argument, 0, 0},
	{PSGIMPROVED_DISABLE,	no_argument, 0, 0},
	{YMIMPROVED_ENABLE,	no_argument, 0, 0},
	{YMIMPROVED_DISABLE,	no_argument, 0, 0},
	{DACIMPROVED_ENABLE,	no_argument, 0, 0},
	{DACIMPROVED_DISABLE,	no_argument, 0, 0},
	{PERFECTSYNC_ENABLE,	no_argument, 0, 0},
	{PERFECTSYNC_DISABLE,	no_argument, 0, 0},
	{MSH2SPEED,		required_argument, 0, 0},
	{SSH2SPEED,		required_argument, 0, 0},
	{FASTBLUR_ENABLE,	no_argument, 0, 0},
	{FASTBLUR_DISABLE,	no_argument, 0, 0},
	{FPS_ENABLE,		no_argument, 0, 0},
	{FPS_DISABLE,		no_argument, 0, 0},
	{MSG_ENABLE,		no_argument, 0, 0},
	{MSG_DISABLE,		no_argument, 0, 0},
	{LED_ENABLE,		no_argument, 0, 0},
	{LED_DISABLE,		no_argument, 0, 0},
	{FIXCHKSUM_ENABLE,	no_argument, 0, 0},
	{FIXCHKSUM_DISABLE,	no_argument, 0, 0},
	{AUTOPAUSE_ENABLE,	no_argument, 0, 0},
	{AUTOPAUSE_DISABLE,	no_argument, 0, 0},
	{RAMCARTSIZE,		required_argument, 0, 0},
	{QUICKEXIT,		no_argument, 0, 0},
	{HELP,			no_argument, 0, 0},
	{0, 0, 0, 0}
};


static void _usage()
{
	fprintf (stderr, GENS_APPNAME " " GENS_VERSION "\n");
	fprintf (stderr, "Usage: gens [options] romfilename\n");
	print_usage(  HELP, "print this help");
	
	print_usage(GAME ,"ROM to load (from standard ROM directory)");
	print_usage(ROMPATH ,"Path where your roms are stored");
	print_usage(SAVEPATH ,"Path where to save your states files");
	print_usage(SRAMPATH ,"Path where to save your SRAM files");
	print_usage(BRAMPATH ,"Path where to save your BRAM files");	
	print_usage(DUMPPATH ,"unused");
	print_usage(DUMPGYMPATH ,"Path where to save your GYM files");
	print_usage(SCRSHTPATH ,"Path where to save your screenshot files");
	print_usage(PATPATH ,"Path where to save your patch files");	
	print_usage(IPSPATH ,"Path where to save your IPS files");
	print_usage(GCOFFPATH ,"unused");
	print_usage(GENSMANPATH ,"unused");
	print_usage(GENBIOS ,"Genesis bios");	
	print_usage(USABIOS ,"US cd bios");
	print_usage(EURBIOS ,"European cd bios");
	print_usage(JAPBIOS ,"Japan cd bios");
	print_usage(_32X68kBIOS ,"32X Genesis bios");
	print_usage(_32XMBIOS ,"32X Master SH2 bios");
	print_usage(_32XSBIOS ,"32X Slave SH2 bios");
	print_usage(CONTRAST ,"Contrast (-100 -> 100)");
	print_usage(BRIGHTNESS ,"Brightness (-100 -> 100)");	
	print_usage(FS, "Run in full screen mode");
	print_usage(WINDOW, "Run in window mode");
	print_usage(RENDERMODE ,"Render mode options\n"
				"   Normal : 1\n"
				"   Double : 2\n"
				"   Interpolated : 3\n"
				"   Full Scanline : 4\n"
				"   Scanline 50%% : 5\n"
				"   Scanline 25%% : 6\n"
				"   Interpolated Scanline : 7\n"
				"   Interpolated Scanline 50%% : 8\n"
				"   Interpolated Scanline 25%% : 9\n"
				"   2XSai Kreed : 10\n"
				"   AdvanceMAME Scale2x : 11\n"
				"   HQ2X : 12"
				);
	print_usage(FRAMESKIP ,"Frameskip (0 -> 9)");
	print_usage(SOUNDRATE ,"Soundrate (11025, 16000, 22050, 32000, 44100, 48000 kHz)");	
	print_usage(MSH2SPEED ,"Master SH2 speed");
	print_usage(SSH2SPEED ,"Slave SH2 speed");
	print_usage(RAMCARTSIZE,"Ram cart size");
	print_usage(QUICKEXIT, "Quick exit with ESC");

	print_usage2(STRETCH,"Stretch mode");
	print_usage2(SWBLIT,"Software blitting");
	print_usage2(GREYSCALE,"Greyscale");
	print_usage2(INVERT,"Invert color");
	print_usage2(SPRITELIMIT,"Sprite limit");
	print_usage2(SOUND,"Sound");
	print_usage2(STEREO,"Stereo");
	print_usage2(Z80,"Z80");
	print_usage2(YM2612,"Yamaha 2612");
	print_usage2(PSG,"psg");
	print_usage2(DAC,"Digital to analogic converter");
	print_usage2(PCM,"pcm");
	print_usage2(PWM,"pwm");
	print_usage2(CDDA,"cdda");
	print_usage2(PSGIMPROVED,"psg improved");
	print_usage2(YMIMPROVED,"ym2612 improved");
	print_usage2(DACIMPROVED,"dac improved");
	print_usage2(PERFECTSYNC,"Perfect Synchro");
	print_usage2(FASTBLUR,"Fast blur");
	print_usage2(FPS,"Frame per second");
	print_usage2(MSG,"Message");
	print_usage2(LED,"SegaCD led");
	print_usage2(FIXCHKSUM,"Fix checksum");
	print_usage2(AUTOPAUSE,"Auto-pause");
	
	exit(0);
}


#define TEST_OPTION_STRING(option, strbuf)		\
if (!strcmp(long_options[option_index].name, option))	\
{							\
	strcpy(strbuf, optarg);				\
	continue;					\
}


#define TEST_OPTION_ENABLE(option, enablevar)					\
if (!strcmp(long_options[option_index].name, option ## _ENABLE))		\
{										\
	enablevar = 1;								\
	continue;								\
}										\
else if (!strcmp(long_options[option_index].name, option ## _DISABLE))		\
{										\
	enablevar = 0;								\
	continue;								\
}


#define TEST_OPTION_NUMERIC(option, numvar)		\
if (!strcmp(long_options[option_index].name, option))	\
{							\
	numvar = strtol(optarg, (char**)NULL, 10);	\
	continue;					\
}


void parseArgs(int argc, char **argv)
{
	int c;
	int error = 0;
	
	while (1)
	{
		int option_index = 0;
		
		c = getopt_long(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;
		if (c == '?')
		{
			error = 1;
			continue;
		}
			
		if (!strcmp (long_options[option_index].name, GAME))
		{
			if (strcmp(optarg, "") != 0)
			{
				strcpy(PathNames.Start_Rom, Rom_Dir);
				strcat(PathNames.Start_Rom, optarg);
			}
		}
		
		// Test string options.
		TEST_OPTION_STRING(ROMPATH, Rom_Dir);
		TEST_OPTION_STRING(SAVEPATH, State_Dir);
		TEST_OPTION_STRING(SRAMPATH, SRAM_Dir);
		TEST_OPTION_STRING(BRAMPATH, BRAM_Dir);
		TEST_OPTION_STRING(DUMPPATH, Dump_Dir);
		TEST_OPTION_STRING(SCRSHTPATH, ScrShot_Dir);
		TEST_OPTION_STRING(PATPATH, Patch_Dir);
		TEST_OPTION_STRING(IPSPATH, IPS_Dir);
		TEST_OPTION_STRING(GCOFFPATH, Misc_Filenames.GCOffline);
		TEST_OPTION_STRING(GENSMANPATH, Misc_Filenames.Manual);
		TEST_OPTION_STRING(GENBIOS, BIOS_Filenames.MD_TMSS);
		TEST_OPTION_STRING(USABIOS, BIOS_Filenames.SegaCD_US);
		TEST_OPTION_STRING(EURBIOS, BIOS_Filenames.MegaCD_EU);
		TEST_OPTION_STRING(JAPBIOS, BIOS_Filenames.MegaCD_JP);
		TEST_OPTION_STRING(_32X68kBIOS, BIOS_Filenames._32X_MC68000);
		TEST_OPTION_STRING(_32XMBIOS, BIOS_Filenames._32X_MSH2);
		TEST_OPTION_STRING(_32XSBIOS, BIOS_Filenames._32X_SSH2);
		
		/* TODO: C++ support
		TEST_OPTION_ENABLE(STRETCH, Stretch);
		TEST_OPTION_ENABLE(SWBLIT, Blit_Soft);
		*/
		TEST_OPTION_NUMERIC(CONTRAST, Contrast_Level);
		TEST_OPTION_NUMERIC(BRIGHTNESS, Brightness_Level);
		TEST_OPTION_ENABLE(GREYSCALE, Greyscale);
		TEST_OPTION_ENABLE(INVERT, Invert_Color);
		TEST_OPTION_ENABLE(SPRITELIMIT, Sprite_Over);
		TEST_OPTION_NUMERIC(FRAMESKIP, Frame_Skip);
		TEST_OPTION_ENABLE(SOUND, Sound_Enable);
		TEST_OPTION_ENABLE(STEREO, Sound_Stereo);
		TEST_OPTION_ENABLE(Z80, Z80_State);
		TEST_OPTION_ENABLE(YM2612, YM2612_Enable);
		TEST_OPTION_ENABLE(PSG, PSG_Enable);
		TEST_OPTION_ENABLE(DAC, DAC_Enable);
		TEST_OPTION_ENABLE(PCM, PCM_Enable);
		TEST_OPTION_ENABLE(PWM, PWM_Enable);
		TEST_OPTION_ENABLE(CDDA, CDDA_Enable);
		TEST_OPTION_ENABLE(PSGIMPROVED, PSG_Improv);
		TEST_OPTION_ENABLE(YMIMPROVED, YM2612_Improv);
		TEST_OPTION_ENABLE(DACIMPROVED, DAC_Improv);
		TEST_OPTION_ENABLE(PERFECTSYNC, SegaCD_Accurate);
		TEST_OPTION_NUMERIC(MSH2SPEED, MSH2_Speed);
		TEST_OPTION_NUMERIC(SSH2SPEED, SSH2_Speed);
		TEST_OPTION_ENABLE(FASTBLUR, Video.Fast_Blur);
		TEST_OPTION_ENABLE(FPS, Show_FPS);
		//TEST_OPTION_ENABLE(MSG, Show_Message);
		TEST_OPTION_ENABLE(LED, Show_LED);
		TEST_OPTION_ENABLE(FIXCHKSUM, Auto_Fix_CS);
		TEST_OPTION_ENABLE(AUTOPAUSE, Auto_Pause);
		TEST_OPTION_NUMERIC(RAMCARTSIZE, BRAM_Ex_Size);
		
		// Other options that can't be handled by macros.
		if (!strcmp(long_options[option_index].name, MSG_ENABLE))
		{
			draw->setMsgEnabled(true);
		}
		else if (!strcmp(long_options[option_index].name, MSG_DISABLE))
		{
			draw->setMsgEnabled(false);
		}
		else if (!strcmp(long_options[option_index].name, FS))
		{
			Video.Full_Screen = 1;
		}
		else if (!strcmp(long_options[option_index].name, WINDOW))
		{
			Video.Full_Screen = 0;
		}
		else if (!strcmp(long_options[option_index].name, RENDERMODE))
		{
			int mode = strtol (optarg, (char **) NULL, 10);
			
			// TODO: NB_FILTER?
			if ((mode <0) || (mode>=NB_FILTER))
			{
				fprintf(stderr, "Invalid render mode : %d\n", mode);
				exit(1);
			}
			else
			{
				fprintf(stderr, "Render mode : %d\n", mode);
				Video.Render_Mode = mode;
			}
		}
		else if (!strcmp(long_options[option_index].name, SOUNDRATE))
		{
			int rate = strtol(optarg, (char**)NULL, 10);
			switch(rate)
			{
				case 11025:
				case 16000:
				case 22050:
				case 32000:
				case 44100:
				case 48000:
					Sound_Rate = rate;
					break;
				
				default:
					fprintf(stderr, "Invalid rate");
					break;
			}
		}
		else if (!strcmp(long_options[option_index].name, QUICKEXIT))
		{
			Quick_Exit = 1;
		}
		else if (!strcmp(long_options[option_index].name, HELP))
		{
			_usage();
		}
		else 
		{
			_usage();
		}
	}
	
	if (optind < argc - 1 || error)
	{
		fputs("Arguments not understood.\n", stderr);
		_usage();
	}
	else if (optind == argc - 1)
	{
		if (argv[optind][0] != '/')
		{
			GetCurrentDirectory(900, PathNames.Start_Rom);
			strcat(PathNames.Start_Rom, "/");
		}
		strcat (PathNames.Start_Rom, argv[optind]);
	}
}


