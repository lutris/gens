#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "port/port.h"
#include "util/file//save.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"
#include "gens.h"
#include "g_main.hpp"
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
#include "parse.h"
#include "ui_proxy.hpp"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.h"
#endif

#define print_usage(option, helpmsg) fprintf(stderr, MM option " : " helpmsg "\n")
#define print_usage2(option, helpmsg) fprintf(stderr, MM ENABLE "-" option ", " MM DISABLE "-" option " : " helpmsg "\n")

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
	
	exit (0);	
}

void
parseArgs (int argc, char **argv)
{
  int c;
  int error = 0;

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
	{GAME, required_argument, 0, 0},
	{ROMPATH, required_argument, 0, 0},
	{SAVEPATH, required_argument, 0, 0},
	{SRAMPATH, required_argument, 0, 0},
	{BRAMPATH, required_argument, 0, 0},
	{DUMPPATH, required_argument, 0, 0},
	{DUMPGYMPATH, required_argument, 0, 0},
	{SCRSHTPATH, required_argument, 0, 0},
	{PATPATH, required_argument, 0, 0},
	{IPSPATH, required_argument, 0, 0},
	{GCOFFPATH, required_argument, 0, 0},
	{GENSMANPATH, required_argument, 0, 0},
	{GENBIOS, required_argument, 0, 0},
	{USABIOS, required_argument, 0, 0},
	{EURBIOS, required_argument, 0, 0},
	{JAPBIOS, required_argument, 0, 0},
	{_32X68kBIOS, required_argument, 0, 0},
	{_32XMBIOS, required_argument, 0, 0},
	{_32XSBIOS, required_argument, 0, 0},
	{FS, no_argument, 0, 0},
	{WINDOW, no_argument, 0, 0},
	{RENDERMODE, required_argument, 0, 0},
	{STRETCH_ENABLE, no_argument, 0, 0},
	{STRETCH_DISABLE, no_argument, 0, 0},
	{SWBLIT_ENABLE, no_argument, 0, 0},
	{SWBLIT_DISABLE, no_argument, 0, 0},
	{CONTRAST, required_argument, 0, 0},
	{BRIGHTNESS, required_argument, 0, 0},
	{GREYSCALE_ENABLE, no_argument, 0, 0},
	{GREYSCALE_DISABLE, no_argument, 0, 0},
	{INVERT_ENABLE, no_argument, 0, 0},
	{INVERT_DISABLE, no_argument, 0, 0},
	{SPRITELIMIT_ENABLE, no_argument, 0, 0},
	{SPRITELIMIT_DISABLE, no_argument, 0, 0},
	{FRAMESKIP, required_argument, 0, 0},
	{SOUND_ENABLE, no_argument, 0, 0},
	{SOUND_DISABLE, no_argument, 0, 0},
	{SOUNDRATE, required_argument, 0, 0},
	{STEREO_ENABLE, no_argument, 0, 0},
	{STEREO_DISABLE, no_argument, 0, 0},
	{Z80_ENABLE, no_argument, 0, 0},
	{Z80_DISABLE, no_argument, 0, 0},
	{YM2612_ENABLE, no_argument, 0, 0},
	{YM2612_DISABLE, no_argument, 0, 0},
	{PSG_ENABLE, no_argument, 0, 0},
	{PSG_DISABLE, no_argument, 0, 0},
	{DAC_ENABLE, no_argument, 0, 0},
	{DAC_DISABLE, no_argument, 0, 0},
	{PCM_ENABLE, no_argument, 0, 0},
	{PCM_DISABLE, no_argument, 0, 0},
	{PWM_ENABLE, no_argument, 0, 0},
	{PWM_DISABLE, no_argument, 0, 0},
	{CDDA_ENABLE, no_argument, 0, 0},
	{CDDA_DISABLE, no_argument, 0, 0},
	{PSGIMPROVED_ENABLE, no_argument, 0, 0},
	{PSGIMPROVED_DISABLE, no_argument, 0, 0},
	{YMIMPROVED_ENABLE, no_argument, 0, 0},
	{YMIMPROVED_DISABLE, no_argument, 0, 0},
	{DACIMPROVED_ENABLE, no_argument, 0, 0},
	{DACIMPROVED_DISABLE, no_argument, 0, 0},
	{PERFECTSYNC_ENABLE, no_argument, 0, 0},
	{PERFECTSYNC_DISABLE, no_argument, 0, 0},
	{MSH2SPEED, required_argument, 0, 0},
	{SSH2SPEED, required_argument, 0, 0},
	{FASTBLUR_ENABLE, no_argument, 0, 0},
	{FASTBLUR_DISABLE, no_argument, 0, 0},
	{FPS_ENABLE, no_argument, 0, 0},
	{FPS_DISABLE, no_argument, 0, 0},
	{MSG_ENABLE, no_argument, 0, 0},
	{MSG_DISABLE, no_argument, 0, 0},
	{LED_ENABLE, no_argument, 0, 0},
	{LED_DISABLE, no_argument, 0, 0},
	{FIXCHKSUM_ENABLE, no_argument, 0, 0},
	{FIXCHKSUM_DISABLE, no_argument, 0, 0},
	{AUTOPAUSE_ENABLE, no_argument, 0, 0},
	{AUTOPAUSE_DISABLE, no_argument, 0, 0},
	{RAMCARTSIZE, required_argument, 0, 0},
	{QUICKEXIT, no_argument, 0, 0},
	{HELP, no_argument, 0, 0},
	{0, 0, 0, 0}
      };

	c = getopt_long (argc, argv, "", long_options, &option_index);
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
			strcpy (PathNames.Start_Rom, Rom_Dir);
			strcat (PathNames.Start_Rom, optarg);
		}
	}
    else if (!strcmp (long_options[option_index].name, ROMPATH))
	{
	  strcpy (Rom_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, SAVEPATH))
	{
	  strcpy (State_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, SRAMPATH))
	{
	  strcpy (SRAM_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, BRAMPATH))
	{
	  strcpy (BRAM_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, DUMPPATH))
	{
	  strcpy (Dump_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, DUMPGYMPATH))
	{
	  strcpy (Dump_GYM_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, SCRSHTPATH))
	{
	  strcpy (ScrShot_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, PATPATH))
	{
	  strcpy (Patch_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, IPSPATH))
	{
	  strcpy (IPS_Dir, optarg);
	}
      else if (!strcmp (long_options[option_index].name, GCOFFPATH))
	{
	  strcpy (Misc_Filenames.GCOffline, optarg);
	}
      else if (!strcmp (long_options[option_index].name, GENSMANPATH))
	{
	  strcpy (Misc_Filenames.Manual, optarg);
	}
      else if (!strcmp (long_options[option_index].name, GENBIOS))
	{
	  strcpy (BIOS_Filenames.MD_TMSS, optarg);
	}
      else if (!strcmp (long_options[option_index].name, USABIOS))
	{
	  strcpy (BIOS_Filenames.SegaCD_US, optarg);
	}
      else if (!strcmp (long_options[option_index].name, EURBIOS))
	{
	  strcpy (BIOS_Filenames.MegaCD_EU, optarg);
	}
      else if (!strcmp (long_options[option_index].name, JAPBIOS))
	{
	  strcpy (BIOS_Filenames.MegaCD_JP, optarg);
	}
      else if (!strcmp (long_options[option_index].name, _32X68kBIOS))
	{
	  strcpy (BIOS_Filenames._32X_MC68000, optarg);
	}
      else if (!strcmp (long_options[option_index].name, _32XMBIOS))
	{
	  strcpy (BIOS_Filenames._32X_MSH2, optarg);
	}
      else if (!strcmp (long_options[option_index].name, _32XSBIOS))
	{
	  strcpy (BIOS_Filenames._32X_SSH2, optarg);
	}
      else if (!strcmp (long_options[option_index].name, FS))
	{
	  Video.Full_Screen = 1;
	}
      else if (!strcmp (long_options[option_index].name, WINDOW))
	{
	  Video.Full_Screen = 0;
	}
      else if (!strcmp (long_options[option_index].name, RENDERMODE))
	{
	  int mode = strtol (optarg, (char **) NULL, 10);
	  
	  if ((mode <0) || (mode>=NB_FILTER))
	  {
	  	fprintf(stderr, "Invalid render mode : %d\n", mode);
	  	exit(1);
	  }
	  else {
	  	fprintf(stderr, "Render mode : %d\n", mode);
	  	Video.Render_Mode = mode;
	  }
	}
	/* TODO: C++ support
      else if (!strcmp (long_options[option_index].name, STRETCH_ENABLE))
	{
	  Stretch = 1;
	}
      else if (!strcmp (long_options[option_index].name, STRETCH_DISABLE))
	{
	  Stretch = 0;
	}
      else if (!strcmp (long_options[option_index].name, SWBLIT_ENABLE))
	{
	  Blit_Soft = 1;
	}
      else if (!strcmp (long_options[option_index].name, SWBLIT_DISABLE))
	{
	  Blit_Soft = 0;
	}
	*/
      else if (!strcmp (long_options[option_index].name, CONTRAST))
	{
	  Contrast_Level = strtol (optarg, (char **) NULL, 10);
	}
      else if (!strcmp (long_options[option_index].name, BRIGHTNESS))
	{
	  Brightness_Level = strtol (optarg, (char **) NULL, 10);
	}
      else if (!strcmp (long_options[option_index].name, GREYSCALE_ENABLE))
	{
	  Greyscale = 1;
	}
      else if (!strcmp (long_options[option_index].name, GREYSCALE_DISABLE))
	{
	  Greyscale = 0;
	}
      else if (!strcmp (long_options[option_index].name, INVERT_ENABLE))
	{
	  Invert_Color = 1;
	}
      else if (!strcmp (long_options[option_index].name, INVERT_DISABLE))
	{
	  Invert_Color = 0;
	}
      else if (!strcmp (long_options[option_index].name, SPRITELIMIT_ENABLE))
	{
	  Sprite_Over = 1;
	}
      else if (!strcmp (long_options[option_index].name, SPRITELIMIT_DISABLE))
	{
	  Sprite_Over = 0;
	}
      else if (!strcmp (long_options[option_index].name, FRAMESKIP))
	{
	  Frame_Skip = strtol (optarg, (char **) NULL, 10);
	}
      else if (!strcmp (long_options[option_index].name, SOUND_ENABLE))
	{
	  Sound_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, SOUND_DISABLE))
	{
	  Sound_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, SOUNDRATE))
	{
	   int rate = strtol (optarg, (char **) NULL, 10);
	   switch(rate) {
	   	case 11025:
	   	case 16000:
	   	case 22050:
	   	case 32000:
	   	case 44100:
	   	case 48000:
		   Sound_Rate =	rate;
		   break;
	   	default:
	 	   fprintf(stderr, "Invalid rate");
	   	   break;
	   }
	}
      else if (!strcmp (long_options[option_index].name, STEREO_ENABLE))
	{
	  Sound_Stereo = 1;
	}
      else if (!strcmp (long_options[option_index].name, STEREO_DISABLE))
	{
	  Sound_Stereo = 0;
	}
      else if (!strcmp (long_options[option_index].name, Z80_ENABLE))
	{
	  Z80_State = 1;
	}
      else if (!strcmp (long_options[option_index].name, Z80_DISABLE))
	{
	  Z80_State = 0;
	}
      else if (!strcmp (long_options[option_index].name, YM2612_ENABLE))
	{
	  YM2612_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, YM2612_DISABLE))
	{
	  YM2612_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, PSG_ENABLE))
	{
	  PSG_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, PSG_DISABLE))
	{
	  PSG_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, DAC_ENABLE))
	{
	  DAC_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, DAC_DISABLE))
	{
	  DAC_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, PCM_ENABLE))
	{
	  PCM_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, PCM_DISABLE))
	{
	  PCM_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, PWM_ENABLE))
	{
	  PWM_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, PWM_DISABLE))
	{
	  PWM_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, CDDA_ENABLE))
	{
	  CDDA_Enable = 1;
	}
      else if (!strcmp (long_options[option_index].name, CDDA_DISABLE))
	{
	  CDDA_Enable = 0;
	}
      else if (!strcmp (long_options[option_index].name, PSGIMPROVED_ENABLE))
	{
	  PSG_Improv = 1;
	}
      else if (!strcmp (long_options[option_index].name, PSGIMPROVED_DISABLE))
	{
	  PSG_Improv = 0;
	}
      else if (!strcmp (long_options[option_index].name, YMIMPROVED_ENABLE))
	{
	  YM2612_Improv = 1;
	}
      else if (!strcmp (long_options[option_index].name, YMIMPROVED_DISABLE))
	{
	  YM2612_Improv = 0;
	}
      else if (!strcmp (long_options[option_index].name, DACIMPROVED_ENABLE))
	{
	  DAC_Improv = 1;
	}
      else if (!strcmp (long_options[option_index].name, DACIMPROVED_DISABLE))
	{
	  DAC_Improv = 0;
	}
      else if (!strcmp (long_options[option_index].name, PERFECTSYNC_ENABLE))
	{
	  SegaCD_Accurate = 1;
	}
      else if (!strcmp (long_options[option_index].name, PERFECTSYNC_DISABLE))
	{
	  SegaCD_Accurate = 0;
	}
      else if (!strcmp (long_options[option_index].name, MSH2SPEED))
	{
	  MSH2_Speed = strtol (optarg, (char **) NULL, 10);
	}
      else if (!strcmp (long_options[option_index].name, SSH2SPEED))
	{
	  SSH2_Speed = strtol (optarg, (char **) NULL, 10);
	}
      else if (!strcmp (long_options[option_index].name, FASTBLUR_ENABLE))
	{
	  Video.Fast_Blur = 1;
	}
      else if (!strcmp (long_options[option_index].name, FASTBLUR_DISABLE))
	{
	  Video.Fast_Blur = 0;
	}
      else if (!strcmp (long_options[option_index].name, FPS_ENABLE))
	{
	  Show_FPS = 1;
	}
      else if (!strcmp (long_options[option_index].name, FPS_DISABLE))
	{
	  Show_FPS = 0;
	}
      else if (!strcmp (long_options[option_index].name, MSG_ENABLE))
	{
	  Show_Message = 1;
	}
      else if (!strcmp (long_options[option_index].name, MSG_DISABLE))
	{
	  Show_Message = 0;
	}
      else if (!strcmp (long_options[option_index].name, LED_ENABLE))
	{
	  Show_LED = 1;
	}
      else if (!strcmp (long_options[option_index].name, LED_DISABLE))
	{
	  Show_LED = 0;
	}
      else if (!strcmp (long_options[option_index].name, FIXCHKSUM_ENABLE))
	{
	  Auto_Fix_CS = 1;
	}
      else if (!strcmp (long_options[option_index].name, FIXCHKSUM_DISABLE))
	{
	  Auto_Fix_CS = 0;
	}
      else if (!strcmp (long_options[option_index].name, AUTOPAUSE_ENABLE))
	{
	  Auto_Pause = 1;
	}
      else if (!strcmp (long_options[option_index].name, AUTOPAUSE_DISABLE))
	{
	  Auto_Pause = 0;
	}
      else if (!strcmp (long_options[option_index].name, RAMCARTSIZE))
	{
	  BRAM_Ex_Size = strtol (optarg, (char **) NULL, 10);
	}
      else if (!strcmp (long_options[option_index].name, QUICKEXIT))
	{
	  Quick_Exit = 1;
	}
      else if (!strcmp (long_options[option_index].name, HELP))
	{
		_usage();
	} else 
	{
		_usage();
	}
    }

  if (optind < argc - 1 || error)
    {
      fputs ("Arguments not understood.\n", stderr);
      _usage();
    }
  else if (optind == argc - 1)
    {
      if (argv[optind][0] != '/') {
        GetCurrentDirectory (900, PathNames.Start_Rom);
        strcat (PathNames.Start_Rom, "/");
      }
      strcat (PathNames.Start_Rom, argv[optind]);
    }
}


