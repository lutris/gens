/***************************************************************************
 * Gens: Command line parser.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C includes.
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "g_main.hpp"
#include "util/file/save.hpp"
#include "md_palette.hpp"
#include "util/file/rom.hpp"

#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "util/gfx/imageutil.hpp"
#include "gens_core/io/io.h"
#include "segacd/cd_sys.hpp"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif

// File management functions.
#include "util/file/file.hpp"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "audio/audio.h"
#include "input/input.h"

// libgsft includes.
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_strlcpy.h"

// Include this *last* to avoid naming conflicts.
#include "parse.hpp"


// 1-argument parameter struct.
struct opt1arg_str_t
{
	const char* option;
	const char* argument;
	const char* description;
};

// 1-argument parameters.
static const opt1arg_str_t opt1arg_str[] =
{
	{"game",		"filename",	"ROM to load (from standard ROM directory)"},
	{"rompath",		"pathname",	"Path where your ROMs are stored"},
	{"savepath",		"pathname",	"Path where to save your states files"},
	{"srampath",		"pathname",	"Path where to save your SRAM files"},
	{"brampath",		"pathname",	"Path where to save your BRAM files"},
	{"dumpwavpath",		"pathname",	"Path where to save your WAV files"},
	{"dumpgympath",		"pathname",	"Path where to save your GYM files"},
	{"screenshotpath",	"pathname",	"Path where to save your screenshot files"},
	{"patchpath",		"pathname",	"Path where to save your patch files"},
	{"genesisbios",		"filename",	"Genesis BIOS"},
	{"usacdbios",		"filename",	"USA SegaCD BIOS"},
	{"europecdbios",	"filename",	"European MegaCD BIOS"},
	{"japancdbios",		"filename",	"Japanese MegaCD BIOS"},
	{"32x68kbios",		"filename",	"32X MC68000 BIOS"},
	{"32xmsh2bios",		"filename",	"32X Main SH2 BIOS"},
	{"32xssh2bios",		"filename",	"32X Slave SH2 BIOS"},
	{"contrast",		"number",	"Contrast (-100 -> 100)"},
	{"brightness",		"number",	"Brightness (-100 -> 100)"},
	{"frameskip",		"number",	"Frameskip (-1 [Auto] -> 9)"},
	{"soundrate",		"rate",		"Sound Rate (11025, 22050, 44100 Hz)"},
	{"msh2-speed",		"percentage",	"Master SH2 Speed"},
	{"ssh2-speed",		"percentage",	"Slave SH2 Speed"},
	{"ramcart-size",	"number",	"SegaCD RAM cart size"},
	{NULL, NULL, NULL}
};

enum opt1arg_enum
{
	OPT1_GAME = 0,
	OPT1_ROMPATH,
	OPT1_SAVEPATH,
	OPT1_SRAMPATH,
	OPT1_BRAMPATH,
	OPT1_DUMPWAVPATH,
	OPT1_DUMPGYMPATH,
	OPT1_SCREENSHOTPATH,
	OPT1_GENESISBIOS,
	OPT1_USACDBIOS,
	OPT1_EUROPECDBIOS,
	OPT1_JAPANCDBIOS,
	OPT1_32X68KBIOS,
	OPT1_32XMSH2BIOS,
	OPT1_32XSSH2BIOS,
	OPT1_CONTRAST,
	OPT1_BRIGHTNESS,
	OPT1_FRAMESKIP,
	OPT1_SOUNDRATE,
	OPT1_MSH2_SPEED,
	OPT1_SSH2_SPEED,
	OPT1_RAMCART_SIZE,
	OPT1_TOTAL
};

// 0-argument parameter struct.
struct opt0arg_str_t
{
	const char* option;
	const char* description;
};

// 0-argument parameters.
// Index: 0 = parameter; 1 = description
static const opt0arg_str_t opt0arg_str[] =
{
	{"help",	"Help"},
	{"fs",		"Run in full screen mode"},
	{"window",	"Run in windowed mode"},
	{"quickexit",	"Quick exit with ESC"},
#ifdef GENS_CDROM
	{"boot-cd",	"Boot SegaCD"},
#endif
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
	{"debug",	"Enable debug console"},
#endif
	{NULL, NULL}
};

enum opt0arg_enum
{
	OPT0_HELP = 0,
	OPT0_FS,
	OPT0_WINDOW,
	OPT0_QUICKEXIT,
#ifdef GENS_CDROM
	OPT0_BOOT_CD,
#endif
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
	OPT0_DEBUG_CONSOLE,
#endif
	OPT0_TOTAL
};

// Boolean argument parameter struct.
struct optBarg_str_t
{
	const char* enable;
	const char* disable;
	const char* description;
};

// Boolean parameters.
// Index: 0 = enable parameter; 1 = disable parameter; 2 = description
#define OPTBARG_STR(parameter, description) \
	{"enable-" parameter, "disable-" parameter, description}

static const optBarg_str_t optBarg_str[] =
{
	OPTBARG_STR("stretch",		"Stretch mode"),
	OPTBARG_STR("swblit",		"Software blitting"),
	OPTBARG_STR("greyscale",	"Greyscale"),
	OPTBARG_STR("invert",		"Invert color"),
	OPTBARG_STR("scale-colors",	"Scale colors to full RGB"),
	OPTBARG_STR("spritelimit",	"Sprite limit"),
	OPTBARG_STR("sound",		"Sound"),
	OPTBARG_STR("stereo",		"Stereo"),
	OPTBARG_STR("z80",		"Z80"),
	OPTBARG_STR("ym2612",		"YM2612"),
	OPTBARG_STR("ym2612-improved",	"YM2612 Improved"),
	OPTBARG_STR("dac",		"DAC"),
	OPTBARG_STR("psg",		"PSG"),
	OPTBARG_STR("psg-sine",		"PSG (Sine Wave)"),
	OPTBARG_STR("pcm",		"PCM"),
	OPTBARG_STR("pwm",		"PWM"),
	OPTBARG_STR("cdda",		"CDDA"),
	OPTBARG_STR("perfect-sync",	"SegaCD Perfect Sync"),
	OPTBARG_STR("fastblur",		"Fast Blur"),
	OPTBARG_STR("fps",		"FPS counter"),
	OPTBARG_STR("message",		"Message Display"),
	OPTBARG_STR("led",		"SegaCD LEDs"),
	OPTBARG_STR("fixchksum",	"Auto Fix Checksum"),
	OPTBARG_STR("autopause",	"Auto Pause"),
	{NULL, NULL, NULL}
};

enum optBarg_enum
{
	OPTB_STRETCH = 0,
	OPTB_SWBLIT,
	OPTB_GREYSCALE,
	OPTB_INVERT,
	OPTB_SCALE,
	OPTB_SPRITELIMIT,
	OPTB_SOUND,
	OPTB_STEREO,
	OPTB_Z80,
	OPTB_YM2612,
	OPTB_YM2612_IMPROVED,
	OPTB_DAC,
	OPTB_PSG,
	OPTB_PSG_SINE,
	OPTB_PCM,
	OPTB_PWM,
	OPTB_CDDA,
	OPTB_PERFECT_SYNC,
	OPTB_FASTBLUR,
	OPTB_FPS,
	OPTB_MSG,
	OPTB_LED,
	OPTB_FIXCHKSUM,
	OPTB_AUTOPAUSE,
	OPTB_TOTAL
};

#define LONGOPT_1ARG(index) \
	{opt1arg_str[(index)].option, required_argument, NULL, 0}

#define LONGOPT_0ARG(index) \
	{opt0arg_str[(index)].option, no_argument, NULL, 0}

#define LONGOPT_BARG(index) \
	{optBarg_str[(index)].enable, no_argument, NULL, 0}, \
	{optBarg_str[(index)].disable, no_argument, NULL, 0}

static const struct option long_options[] =
{
	// 1-argument parameters.
	LONGOPT_1ARG(OPT1_GAME),
	LONGOPT_1ARG(OPT1_ROMPATH),
	LONGOPT_1ARG(OPT1_SAVEPATH),
	LONGOPT_1ARG(OPT1_SRAMPATH),
	LONGOPT_1ARG(OPT1_BRAMPATH),
	LONGOPT_1ARG(OPT1_DUMPWAVPATH),
	LONGOPT_1ARG(OPT1_DUMPGYMPATH),
	LONGOPT_1ARG(OPT1_SCREENSHOTPATH),
	LONGOPT_1ARG(OPT1_GENESISBIOS),
	LONGOPT_1ARG(OPT1_USACDBIOS),
	LONGOPT_1ARG(OPT1_EUROPECDBIOS),
	LONGOPT_1ARG(OPT1_JAPANCDBIOS),
	LONGOPT_1ARG(OPT1_32X68KBIOS),
	LONGOPT_1ARG(OPT1_32XMSH2BIOS),
	LONGOPT_1ARG(OPT1_32XSSH2BIOS),
	LONGOPT_1ARG(OPT1_CONTRAST),
	LONGOPT_1ARG(OPT1_BRIGHTNESS),
	LONGOPT_1ARG(OPT1_FRAMESKIP),
	LONGOPT_1ARG(OPT1_SOUNDRATE),
	LONGOPT_1ARG(OPT1_MSH2_SPEED),
	LONGOPT_1ARG(OPT1_SSH2_SPEED),
	LONGOPT_1ARG(OPT1_RAMCART_SIZE),
	
	// 0-argument parameters.
	LONGOPT_0ARG(OPT0_HELP),
	LONGOPT_0ARG(OPT0_FS),
	LONGOPT_0ARG(OPT0_WINDOW),
	LONGOPT_0ARG(OPT0_QUICKEXIT),
#ifdef GENS_CDROM
	LONGOPT_0ARG(OPT0_BOOT_CD),
#endif
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
	LONGOPT_0ARG(OPT0_DEBUG_CONSOLE),
#endif
	
	// Boolean parameters.
	LONGOPT_BARG(OPTB_STRETCH),
	LONGOPT_BARG(OPTB_SWBLIT),
	LONGOPT_BARG(OPTB_GREYSCALE),
	LONGOPT_BARG(OPTB_INVERT),
	LONGOPT_BARG(OPTB_SCALE),
	LONGOPT_BARG(OPTB_SPRITELIMIT),
	LONGOPT_BARG(OPTB_SOUND),
	LONGOPT_BARG(OPTB_STEREO),
	LONGOPT_BARG(OPTB_Z80),
	LONGOPT_BARG(OPTB_YM2612),
	LONGOPT_BARG(OPTB_YM2612_IMPROVED),
	LONGOPT_BARG(OPTB_DAC),
	LONGOPT_BARG(OPTB_PSG),
	LONGOPT_BARG(OPTB_PSG_SINE),
	LONGOPT_BARG(OPTB_PCM),
	LONGOPT_BARG(OPTB_PWM),
	LONGOPT_BARG(OPTB_CDDA),
	LONGOPT_BARG(OPTB_PERFECT_SYNC),
	LONGOPT_BARG(OPTB_FASTBLUR),
	LONGOPT_BARG(OPTB_FPS),
	LONGOPT_BARG(OPTB_MSG),
	LONGOPT_BARG(OPTB_LED),
	LONGOPT_BARG(OPTB_FIXCHKSUM),
	LONGOPT_BARG(OPTB_AUTOPAUSE),
	{NULL, 0, NULL, 0}
};

#define print_usage(option, helpmsg) fprintf(stderr, MM option " : " helpmsg "\n")
#define print_usage2(option, helpmsg) fprintf(stderr, MM ENABLE "-" option ", " MM DISABLE "-" option " : " helpmsg "\n")

static inline void printOpt1Arg(opt1arg_enum opt)
{
	fprintf(stderr, "--%s [%s]: %s\n",
		opt1arg_str[opt].option,
		opt1arg_str[opt].argument,
		opt1arg_str[opt].description);
}

static inline void printOpt0Arg(opt0arg_enum opt)
{
	fprintf(stderr, "--%s: %s\n",
		opt0arg_str[opt].option,
		opt0arg_str[opt].description);
}

static inline void printOptBArg(optBarg_enum opt)
{
	fprintf(stderr, "--%s, --%s: %s\n",
		optBarg_str[opt].enable,
		optBarg_str[opt].disable,
		optBarg_str[opt].description);
}

static void _usage(const char *argv0)
{
	// TODO: Display this in a message box on Win32.
	// Well, either that or a custom window.
	fprintf(stderr, GENS_APPNAME " " GENS_GS_VERSION " (" VERSION ")\n");
	const char *filename;
#if !defined(_WIN32)
	filename = argv0;
#else
	// argv0 contains the full pathname.
	filename = strrchr(argv0, '\\');
	if (filename == NULL)
		filename = "gens.exe";
	else
		filename++;
#endif
	fprintf(stderr, "Usage: %s [options] romfilename\n\nOptions:\n", filename);
	
	// Print the Help option first.
	printOpt0Arg(OPT0_HELP);
	
	// 1-argument parameters.
	for (int i = 0; i < static_cast<int>(OPT1_TOTAL); i++)
	{
		printOpt1Arg(static_cast<opt1arg_enum>(i));
	}
	
	// 0-argument parameters (except OPT1_HELP).
	for (int i = 1; i < static_cast<int>(OPT0_TOTAL); i++)
	{
		printOpt0Arg(static_cast<opt0arg_enum>(i));
	}
	
	// Boolean parameters.
	for (int i = 0; i < static_cast<int>(OPTB_TOTAL); i++)
	{
		printOptBArg(static_cast<optBarg_enum>(i));
	}
	
	exit(0);
}

#define TEST_OPTION_STRING(option, strbuf)		\
if (!strcmp(long_options[option_index].name, option))	\
{							\
	strlcpy(strbuf, optarg, sizeof(strbuf));	\
	continue;					\
}

#define TEST_OPTION_ENABLE(option, enablevar)					\
if (!strcmp(long_options[option_index].name, option.enable))			\
{										\
	enablevar = 1;								\
	continue;								\
}										\
else if (!strcmp(long_options[option_index].name, option.disable))		\
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

#define TEST_OPTION_NUMERIC_SCALE(option, numvar, scale)	\
if (!strcmp(long_options[option_index].name, option))		\
{								\
	numvar = strtol(optarg, (char**)NULL, 10) + scale;	\
	continue;						\
}


/**
 * parse_startup_rom(): Parse the startup ROM filename.
 * @param filename Startup ROM filename.
 * @param startup Pointer to startup information struct.
 */
static void parse_startup_rom(const char *filename, Gens_StartupInfo_t *startup)
{
	// TODO: Concatenate the path to Rom_Dir?
#ifdef GENS_OS_WIN32
	/* Win32: Check for "C:\" and "\\" prefixes. */
	if ((isalpha(filename[0]) && filename[1] == ':' && filename[2] == GSFT_DIR_SEP_CHR) ||
	    (filename[0] == GSFT_DIR_SEP_CHR && filename[1] == GSFT_DIR_SEP_CHR))
#else /* !GENS_OS_WIN32 */
	if (filename[0] == GSFT_DIR_SEP_CHR)
#endif /* GENS_OS_WIN32 */
	{
		// Absolute pathname.
		strlcpy(startup->filename, filename, sizeof(startup->filename));
	}
	else
	{
		// Relative pathname.
		if (!getcwd(startup->filename, sizeof(startup->filename)))
			return;
		strlcat(startup->filename, GSFT_DIR_SEP_STR, sizeof(startup->filename));
		strlcat(startup->filename, filename, sizeof(startup->filename));
	}
	
	// Set the startup mode.
	startup->mode = GSM_ROM;
}


/**
 * parse_args(): Parse command line arguments.
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @return Pointer to startup information struct. (MUST BE free()'d BY CALLER!)
 */
Gens_StartupInfo_t* parse_args(int argc, char *argv[])
{
	int c;
	int error = 0;
	int option_index = 0;
	
	// Create the startup information struct.
	Gens_StartupInfo_t *startup = (Gens_StartupInfo_t*)malloc(sizeof(Gens_StartupInfo_t));
	startup->mode = GSM_IDLE;
	startup->filename[0] = 0x00;
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
	startup->enable_debug_console = 0;
#endif
	
	while (1)
	{
		c = getopt_long(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;
		if (c == '?')
		{
			error = 1;
			continue;
		}
		
		if (!strcmp(long_options[option_index].name, opt1arg_str[OPT1_GAME].option))
		{
			// Startup ROM.
			if (optarg && optarg[0] != 0x00)
				parse_startup_rom(optarg, startup);
			continue;
		}
		
		// Test string options.
		TEST_OPTION_STRING(opt1arg_str[OPT1_ROMPATH].option, Rom_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_SAVEPATH].option, State_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_SRAMPATH].option, SRAM_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_BRAMPATH].option, BRAM_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_DUMPWAVPATH].option, PathNames.Dump_WAV_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_DUMPGYMPATH].option, PathNames.Dump_GYM_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_SCREENSHOTPATH].option, PathNames.Screenshot_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_GENESISBIOS].option, BIOS_Filenames.MD_TMSS);
		TEST_OPTION_STRING(opt1arg_str[OPT1_USACDBIOS].option, BIOS_Filenames.SegaCD_US);
		TEST_OPTION_STRING(opt1arg_str[OPT1_EUROPECDBIOS].option, BIOS_Filenames.MegaCD_EU);
		TEST_OPTION_STRING(opt1arg_str[OPT1_JAPANCDBIOS].option, BIOS_Filenames.MegaCD_JP);
		TEST_OPTION_STRING(opt1arg_str[OPT1_32X68KBIOS].option, BIOS_Filenames._32X_MC68000);
		TEST_OPTION_STRING(opt1arg_str[OPT1_32XMSH2BIOS].option, BIOS_Filenames._32X_MSH2);
		TEST_OPTION_STRING(opt1arg_str[OPT1_32XSSH2BIOS].option, BIOS_Filenames._32X_SSH2);
		
		TEST_OPTION_ENABLE(optBarg_str[OPTB_GREYSCALE], Greyscale);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_INVERT], Invert_Color);
		//TEST_OPTION_ENABLE(optBarg_str[OPTB_SCALE], Scale_Colors);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_SPRITELIMIT], Sprite_Over);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_FRAMESKIP].option, Frame_Skip);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_Z80], Z80_State);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_YM2612], YM2612_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_YM2612_IMPROVED], YM2612_Improv);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_DAC], DAC_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PSG], PSG_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PSG_SINE], PSG_Improv);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PCM], PCM_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PWM], PWM_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_CDDA], CDDA_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PERFECT_SYNC], SegaCD_Accurate);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_MSH2_SPEED].option, MSH2_Speed);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_SSH2_SPEED].option, SSH2_Speed);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_LED], Show_LED);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_FIXCHKSUM], Auto_Fix_CS);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_AUTOPAUSE], Auto_Pause);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_RAMCART_SIZE].option, BRAM_Ex_Size);
		
		// Contrast / Brightness
		TEST_OPTION_NUMERIC_SCALE(opt1arg_str[OPT1_CONTRAST].option, Contrast_Level, 100);
		TEST_OPTION_NUMERIC_SCALE(opt1arg_str[OPT1_BRIGHTNESS].option, Brightness_Level, 100);
		
		// Other options that can't be handled by macros.
		if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STRETCH].enable))
		{
			vdraw_set_stretch(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STRETCH].disable))
		{
			vdraw_set_stretch(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SOUND].enable))
		{
			audio_set_enabled(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SOUND].disable))
		{
			audio_set_enabled(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STEREO].enable))
		{
			audio_set_stereo(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STEREO].disable))
		{
			audio_set_stereo(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SWBLIT].enable))
		{
			vdraw_set_sw_render(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SWBLIT].disable))
		{
			vdraw_set_sw_render(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FASTBLUR].enable))
		{
			vdraw_set_fast_blur(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FASTBLUR].disable))
		{
			vdraw_set_fast_blur(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FPS].enable))
		{
			vdraw_set_fps_enabled(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FPS].disable))
		{
			vdraw_set_fps_enabled(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_MSG].enable))
		{
			vdraw_set_msg_enabled(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_MSG].disable))
		{
			vdraw_set_fps_enabled(false);
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_FS].option))
		{
			vdraw_set_fullscreen(true);
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_WINDOW].option))
		{
			vdraw_set_fullscreen(false);
		}
		else if (!strcmp(long_options[option_index].name, opt1arg_str[OPT1_SOUNDRATE].option))
		{
			int rate = atoi(optarg);
			
			if (rate == 11025 || rate == 22050 || rate == 44100)
			{
				audio_set_sound_rate(rate);
			}
			else
			{
				fprintf(stderr, "Invalid sound rate: %d\n", rate);
				exit(1);
			}
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_QUICKEXIT].option))
		{
			Quick_Exit = 1;
		}
#ifdef GENS_CDROM
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_BOOT_CD].option))
		{
			// Boot SegaCD on startup.
			startup->mode = GSM_BOOT_CD;
		}
#endif
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_DEBUG_CONSOLE].option))
		{
			// Enable Debug console.
			startup->enable_debug_console = 1;
		}
#endif
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_HELP].option))
		{
			_usage(argv[0]);
		}
		else 
		{
			_usage(argv[0]);
		}
	}
	
	// Make sure contrast and brightness are in range.
	if (Contrast_Level < 0)
		Contrast_Level = 0;
	else if (Contrast_Level > 200)
		Contrast_Level = 200;
		
	if (Brightness_Level < 0)
		Brightness_Level = 0;
	else if (Brightness_Level > 200)
		Brightness_Level = 200;
	
	if (optind < argc - 1 || error)
	{
		fputs("Arguments not understood.\n", stderr);
		_usage(argv[0]);
	}
	else if (optind == (argc - 1))
	{
		// Startup ROM.
		if (argv[optind] && argv[optind][0] != 0x00)
			parse_startup_rom(argv[optind], startup);
	}
	
	// Return the startup information.
	return startup;
}
