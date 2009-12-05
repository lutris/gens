/***************************************************************************
 * Gens: GYM file handler.                                                 *
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

#include "gym.hpp"

// C includes.
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// C++ includes.
#include <string>
using std::string;

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

#include "emulator/g_main.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "util/file/rom.hpp"

#include "ui/gens_ui.hpp"

// Video, Audio.
#include "video/vdraw.h"
#include "audio/audio.h"


static FILE *GYM_File = NULL;
int GYM_Dumping = 0;


/**
 * gym_dump_start(): Start dumping a GYM file.
 * @return 0 on success; non-zero on error.
 */
int gym_dump_start(void)
{
	char filename[GENS_PATH_MAX];
	unsigned char YM_Save[0x200], t_buf[4];
	int num, i;
	
	// A game must be loaded in order to dump a GYM.
	if (!Game)
		return -1;
	
	if (GYM_Dumping)
	{
		vdraw_text_write("GYM sound is already dumping", 1000);
		return -2;
	}
	
	// Make sure relative pathnames are handled correctly on Win32.
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif
	
	// Build the filename.
	num = -1;
	do
	{
		num++;
		szprintf(filename, sizeof(filename), "%s%s_%03d.gym", PathNames.Dump_GYM_Dir, ROM_Filename, num);
	} while (!access(filename, F_OK));
	
	GYM_File = fopen(filename, "w");
	if (!GYM_File)
		return -3;
	
	// Save the YM2612 registers.
	YM2612_Save(YM_Save);
	
	for (i = 0x30; i < 0x90; i++)
	{
		t_buf[0] = 1;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i];
		fwrite(t_buf, 3, 1, GYM_File);
		
		t_buf[0] = 2;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i + 0x100];
		fwrite(t_buf, 3, 1, GYM_File);
	}
	
	for (i = 0xA0; i < 0xB8; i++)
	{
		t_buf[0] = 1;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i];
		fwrite(t_buf, 3, 1, GYM_File);
		
		t_buf[0] = 2;
		t_buf[1] = i;
		t_buf[2] = YM_Save[i + 0x100];
		fwrite(t_buf, 3, 1, GYM_File);
	}
	
	t_buf[0] = 1;
	t_buf[1] = 0x22;
	t_buf[2] = YM_Save[0x22];
	fwrite(t_buf, 3, 1, GYM_File);
	
	t_buf[0] = 1;
	t_buf[1] = 0x27;
	t_buf[2] = YM_Save[0x27];
	fwrite(t_buf, 3, 1, GYM_File);
	
	t_buf[0] = 1;
	t_buf[1] = 0x28;
	t_buf[2] = YM_Save[0x28];
	fwrite(t_buf, 3, 1, GYM_File);
	
	vdraw_text_write("Starting to dump GYM sound", 1000);
	GYM_Dumping = 1;
	
	return 0;
}


/**
 * gym_dump_stop(): Stop dumping a GYM file.
 * @return 0 on success; non-zero on error.
 */
int gym_dump_stop(void)
{
	if (!GYM_Dumping)
	{
		vdraw_text_write("Already stopped", 1000);
		return -1;
	}
	
	if (GYM_File)
		fclose(GYM_File);
	audio_clear_sound_buffer();
	GYM_Dumping = 0;
	
	vdraw_text_write("GYM dump stopped", 1000);
	return 0;
}


/**
 * gym_dump_update(): Update a GYM dump.
 * @param v0
 * @param v1
 * @param v2
 * @return 0 on success; non-zero on error.
 */
int gym_dump_update(uint8_t v0, uint8_t v1, uint8_t v2)
{
	if (!GYM_Dumping || !GYM_File)
		return -1;
	
	char buf_tmp[4];
	size_t l;
	
	buf_tmp[0] = v0;
	l = 1;
	
	switch (v0)
	{
		case 1:
		case 2:
			buf_tmp[1] = v1;
			buf_tmp[2] = v2;
			l = 3;
			break;
		
		case 3:
			buf_tmp[1] = v1;
			l = 2;
			break;
	}
	
	int rval = fwrite(buf_tmp, l, 1, GYM_File);
	if (!rval)
		return -2;
	
	return 0;
}


/**
 * gym_play_start(): Start playing a GYM file.
 * @return 0 on success; non-zero on error.
 */
int gym_play_start(void)
{
	if (Game || audio_get_enabled())
		return -1;
	
	if (audio_get_gym_playing())
	{
		vdraw_text_write("Already playing GYM.", 1000);
		return -2;
	}
	
	audio_end();
	CPU_Mode = 0;
	
	if (audio_init(AUDIO_BACKEND_DEFAULT))
	{
		audio_set_enabled(false);
		vdraw_text_write("Can't initialize sound.", 1000);
		return -3;
	}
	
	if (audio_play_sound)
		audio_play_sound();
	
	string filename = GensUI::openFile("Load GYM File", NULL /*Rom_Dir*/, GYMFile);
	if (filename.length() == 0)
		return -4;
	
	// Attempt to open the GYM file.
	GYM_File = fopen(filename.c_str(), "rb");
	if (!GYM_File)
		return -5;
	
	YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
	PSG_Init(CLOCK_NTSC / 15, audio_get_sound_rate());
	audio_set_gym_playing(true);
	
	vdraw_text_write("Starting to play GYM", 1000);
	return 0;
}


/**
 * gym_play_stop(): Stop playing a GYM file.
 * @return 0 on success; non-zero on error.
 */
int gym_play_stop(void)
{
	if (!audio_get_gym_playing())
	{
		vdraw_text_write("Already stopped.", 1000);
		return -1;
	}
	
	if (GYM_File)
		fclose(GYM_File);
	audio_clear_sound_buffer();
	audio_set_gym_playing(false);
	
	vdraw_text_write("Stopped playing GYM.", 1000);
	return 0;
}


/**
 * gym_play_next(): Play the next part of the GYM file.
 * @return 0 on success; non-zero on error.
 */
static int gym_play_next(void)
{
	unsigned char c, c2;
	unsigned int l;
	int *buf[2];
	
	if (!audio_get_gym_playing() || !GYM_File)
		return -1;
	
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	
	do
	{
		l = fread(&c, 1, 1, GYM_File);
		if (l == 0)
			return 0;
		
		switch (c)
		{
			case 0:
				PSG_Update(buf, audio_seg_length);
				if (YM2612_Enable)
					YM2612_Update(buf, audio_seg_length);
				break;
			
			case 1:
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(0, c2);
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(1, c2);
				break;
			
			case 2:
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(2, c2);
				fread(&c2, 1, 1, GYM_File);
				YM2612_Write(3, c2);
				break;
			
			case 3:
				fread(&c2, 1, 1, GYM_File);
				PSG_Write(c2);
				break;
		}
	} while (c);
	
	return 0;
}


/**
 * gym_play(): Play the currently opened GYM file.
 * @return 0 on success; non-zero on error.
 */
int gym_play(void)
{
	if (gym_play_next())
	{
		gym_play_stop();
		return -1;
	}
	
	audio_write_sound_buffer(NULL);
	return 0;
}
