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

#ifndef GENS_PARSE_HPP
#define GENS_PARSE_HPP

#ifdef __cplusplus
extern "C" {
#endif

#define MM			"--"
#define ENABLE			"enable"
#define DISABLE			"disable"

// 1 arg option
#define GAME			"game"
#define ROMPATH 		"rompath"
#define SAVEPATH		"savepath"
#define SRAMPATH		"srampath"
#define BRAMPATH        	"brampath"
#define DUMPPATH        	"dumppath"
#define DUMPGYMPATH		"dumpgympath"
#define SCRSHTPATH		"screenshotpath"
#define PATPATH			"patchpath"
#define IPSPATH			"ipspath"
#define GCOFFPATH		"gcofflinepath"
#define GENSMANPATH		"gensmanualpath"
#define GENBIOS			"genesisbios"
#define USABIOS			"usacdbios"
#define EURBIOS			"europecdbios"
#define JAPBIOS			"japancdbios"
#define _32X68kBIOS		"32x68kbios"
#define _32XMBIOS		"32xmsh2bios"
#define _32XSBIOS		"32xssh2bios"
#define CONTRAST		"contrast"
#define BRIGHTNESS		"brightness"
#define RENDERMODE		"render-mode"
#define FRAMESKIP		"frameskip"
#define SOUNDRATE		"soundrate"
#define MSH2SPEED		"msh2-speed"
#define SSH2SPEED		"ssh2-speed"
#define RAMCARTSIZE		"ramcart-size"

// no arg option

#define FS			"fs"
#define WINDOW			"window"
#define QUICKEXIT		"quickexit"

// enable and disable options
#define STRETCH			"stretch"
#define STRETCH_ENABLE		ENABLE "-" STRETCH
#define STRETCH_DISABLE		DISABLE "-" STRETCH

#define SWBLIT			"swBlit"
#define SWBLIT_ENABLE		ENABLE "-" SWBLIT
#define SWBLIT_DISABLE		DISABLE "-" SWBLIT

#define GREYSCALE		"greyscale"
#define GREYSCALE_ENABLE	ENABLE "-" GREYSCALE
#define GREYSCALE_DISABLE	DISABLE "-" GREYSCALE

#define INVERT			"invert"
#define INVERT_ENABLE		ENABLE "-" INVERT
#define INVERT_DISABLE		DISABLE "-" INVERT

#define SPRITELIMIT		"spritelimit"
#define SPRITELIMIT_ENABLE	ENABLE "-" SPRITELIMIT
#define SPRITELIMIT_DISABLE	DISABLE "-" SPRITELIMIT

#define SOUND			"sound"
#define SOUND_ENABLE		ENABLE "-" SOUND
#define SOUND_DISABLE		DISABLE "-" SOUND

#define STEREO			"soundstereo"
#define STEREO_ENABLE		ENABLE "-" STEREO
#define STEREO_DISABLE		DISABLE "-" STEREO

#define Z80			"z80"
#define Z80_ENABLE		ENABLE "-" Z80
#define Z80_DISABLE		DISABLE "-" Z80

#define YM2612			"ym2612"
#define YM2612_ENABLE		ENABLE "-" YM2612
#define YM2612_DISABLE		DISABLE "-" YM2612

#define PSG			"psg"
#define PSG_ENABLE		ENABLE "-" PSG
#define PSG_DISABLE		DISABLE "-" PSG

#define DAC			"dac"
#define DAC_ENABLE		ENABLE "-" DAC
#define DAC_DISABLE		DISABLE "-" DAC

#define PCM			"pcm"
#define PCM_ENABLE		ENABLE "-" PCM
#define PCM_DISABLE		DISABLE "-" PCM

#define PWM			"pwm"
#define PWM_ENABLE		ENABLE "-" PWM
#define PWM_DISABLE		DISABLE "-" PWM

#define CDDA			"cdda"
#define CDDA_ENABLE		ENABLE "-" CDDA
#define CDDA_DISABLE		DISABLE "-" CDDA

#define IMPROVED		"improved"

#define PSGIMPROVED		IMPROVED "-" PSG
#define PSGIMPROVED_ENABLE	ENABLE "-" IMPROVED "-" PSG
#define PSGIMPROVED_DISABLE	DISABLE "-" IMPROVED "-" PSG

#define YMIMPROVED		IMPROVED "-" YM2612
#define YMIMPROVED_ENABLE	ENABLE "-" IMPROVED "-" YM2612
#define YMIMPROVED_DISABLE	DISABLE "-" IMPROVED "-" YM2612

#define DACIMPROVED		IMPROVED "-" DAC
#define DACIMPROVED_ENABLE	ENABLE "-" IMPROVED "-" DAC
#define DACIMPROVED_DISABLE	DISABLE "-" IMPROVED "-" DAC

#define PERFECTSYNC		"perfectsynchro"
#define PERFECTSYNC_ENABLE	ENABLE "-" PERFECTSYNC
#define PERFECTSYNC_DISABLE	DISABLE "-" PERFECTSYNC

#define FASTBLUR		"fastblur"
#define FASTBLUR_ENABLE		ENABLE "-" FASTBLUR
#define FASTBLUR_DISABLE	DISABLE "-" FASTBLUR

#define FPS			"fps"
#define FPS_ENABLE		ENABLE "-" FPS
#define FPS_DISABLE		DISABLE "-" FPS

#define MSG			"message"
#define MSG_ENABLE		ENABLE "-" MSG
#define MSG_DISABLE		DISABLE "-" MSG

#define LED			"led"
#define LED_ENABLE		ENABLE "-" LED
#define LED_DISABLE		DISABLE "-" LED

#define FIXCHKSUM		"fixchksum"
#define FIXCHKSUM_ENABLE	ENABLE "-" FIXCHKSUM
#define FIXCHKSUM_DISABLE	DISABLE "-" FIXCHKSUM

#define AUTOPAUSE		"autopause"
#define AUTOPAUSE_ENABLE	ENABLE "-" AUTOPAUSE
#define AUTOPAUSE_DISABLE	DISABLE "-" AUTOPAUSE

#define HELP			"help"

void parseArgs (int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* GENS_PARSE_H */
