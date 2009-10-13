/***************************************************************************
 * Gens: WAV file handler.                                                 *
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

#include "wave.h"

/* C includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

#include "emulator/g_main.hpp"
#include "libgsft/gsft_byteswap.h"
#include "gens/gens_window_sync.hpp"

#include "util/file/rom.hpp"

/* Video and audio handlers. */
#include "video/vdraw.h"
#include "audio/audio.h"

/* Make sure the structs are packed. */
#ifndef PACKED
#define PACKED __attribute__ ((packed))
#endif

/* WAV header struct. */
/* Description from http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
/* All values are little-endian, except for character fields. */
typedef struct PACKED _wav_header_t
{
	/* RIFF chunk descriptor. */
	struct PACKED
	{
		char		ChunkID[4];	/* Contains "RIFF". */
		uint32_t	ChunkSize;	/* Size of the whole WAV, minus 8. */
		char		Format[4];	/* Contains "WAVE". */
	} riff;
	
	/* "fmt " sub-chunk. */
	struct PACKED
	{
		char		SubchunkID[4];	/* Contains "fmt ". */
		uint32_t	SubchunkSize;	/* Size of the subchunk, minus 8. */
		uint16_t	AudioFormat;	/* PCM == 1 */
		uint16_t	NumChannels;
		uint32_t	SampleRate;
		uint32_t	ByteRate;
		uint16_t	BlockAlign;
		uint16_t	BitsPerSample;
	} fmt;
	
	struct PACKED
	{
		char		SubchunkID[4];	/* Contains "data". */
		uint32_t	SubchunkSize;	/* Size of the whole WAV, minus 8, minus size of riff and fmt. */
	} data;
} wav_header_t;

int WAV_Dumping = 0;

/* Current WAV file. */
static wav_header_t WAV_Header;
static FILE *WAV_File = NULL;


/**
 * wav_dump_start(): Start dumping a WAV file.
 * @return 0 on success; non-zero on error.
 */
int wav_dump_start(void)
{
	/* Make sure relative pathnames are handled correctly on Win32. */
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif
	
	/* A game must be loaded in order to dump a WAV. */
	if (!Game)
		return 0;
	
	if (WAV_Dumping)
	{
		vdraw_text_write("WAV sound is already dumping.", 1000);
		return 0;
	}
	
	/* Build the filename. */
	char filename[GENS_PATH_MAX];
	int num = -1;
	do
	{
		num++;
		szprintf(filename, sizeof(filename), "%s%s_%03d.wav", PathNames.Dump_WAV_Dir, ROM_Filename, num);
	} while (!access(filename, F_OK));
	
	/* Open the file. */
	WAV_File = fopen(filename, "wb");
	if (!WAV_File)
	{
		vdraw_text_write("Error opening WAV file.", 1000);
		return 1;
	}
	
	/* Create the WAV header. */
	memset(&WAV_Header, 0x00, sizeof(WAV_Header));
	
	/* "RIFF" header. */
	static const char ChunkID_RIFF[4] = {'R', 'I', 'F', 'F'};
	static const char FormatID_WAV[4] = {'W', 'A', 'V', 'E'};
	memcpy(WAV_Header.riff.ChunkID, ChunkID_RIFF, sizeof(WAV_Header.riff.ChunkID));
	memcpy(WAV_Header.riff.Format, FormatID_WAV, sizeof(WAV_Header.riff.Format));
	
	/* "fmt " header. */
	static const char SubchunkID_fmt[4] = {'f', 'm', 't', ' '};
	memcpy(WAV_Header.fmt.SubchunkID, SubchunkID_fmt, sizeof(WAV_Header.fmt.SubchunkID));
	WAV_Header.fmt.SubchunkSize	= cpu_to_le32(sizeof(WAV_Header.fmt) - 8);
	WAV_Header.fmt.AudioFormat	= cpu_to_le16(1); /* PCM */
	WAV_Header.fmt.NumChannels	= cpu_to_le16((audio_get_stereo() ? 2 : 1));
	WAV_Header.fmt.SampleRate	= cpu_to_le32(audio_get_sound_rate());
	WAV_Header.fmt.BitsPerSample	= cpu_to_le16(16); /* Gens is currently hard-coded to 16-bit audio. */
	
	/* Calculated fields. */
	WAV_Header.fmt.BlockAlign	= cpu_to_le16(WAV_Header.fmt.NumChannels * (WAV_Header.fmt.BitsPerSample / 8));
	WAV_Header.fmt.ByteRate		= cpu_to_le32(WAV_Header.fmt.BlockAlign * WAV_Header.fmt.SampleRate);
	
	/* "data" header. */
	static const char SubchunkID_data[4] = {'d', 'a', 't', 'a'};
	memcpy(WAV_Header.data.SubchunkID, SubchunkID_data, sizeof(WAV_Header.data.SubchunkID));
	
	/* Write the initial header to the file. */
	fwrite(&WAV_Header, sizeof(WAV_Header), 1, WAV_File);
	
	/* WAV dump started. */
	vdraw_text_write("Starting to dump WAV sound.", 1000);
	WAV_Dumping = 1;
	Sync_Gens_Window_SoundMenu();
	return 0;
}


/**
 * wav_dump_stop(): Stop dumping a WAV file.
 * @return 0 on success; non-zero on error.
 */
int wav_dump_stop(void)
{
	if (!WAV_Dumping || !WAV_File)
	{
		WAV_Dumping = 0;
		vdraw_text_write("Not dumping WAV sound.", 1000);
		Sync_Gens_Window_SoundMenu();
		return 1;
	}
	
	/* Get the current position in the WAV file. */
	uint32_t wav_pos = (uint32_t)ftell(WAV_File);
	
	/* Seek to the beginning of the WAV file in order to update the header. */
	fseek(WAV_File, 0, SEEK_SET);
	
	/* Set the size values in the header. */
	WAV_Header.riff.ChunkSize	= cpu_to_le32(wav_pos - 8);
	WAV_Header.data.SubchunkSize	= cpu_to_le32(wav_pos - 8 - sizeof(WAV_Header.riff) - sizeof(WAV_Header.fmt));
	
	/* Write the final header. */
	fwrite(&WAV_Header, sizeof(WAV_Header), 1, WAV_File);
	
	/* Close the file. */
	fclose(WAV_File);
	WAV_File = NULL;
	WAV_Dumping = 0;
	vdraw_text_write("WAV dump stopped.", 1000);
	Sync_Gens_Window_SoundMenu();
	return 0;
}


/**
 * wav_dump_update(): Update the WAV file.
 * @return 0 on success; non-zero on error.
 */
int wav_dump_update(void)
{
	if (!WAV_Dumping || !WAV_File)
	{
		if (WAV_File)
		{
			fclose(WAV_File);
			WAV_File = 0;
		}
		
		WAV_Dumping = 0;
		Sync_Gens_Window_SoundMenu();
		return 1;
	}
	
	// TODO: Byteswap on big-endian.
	short buf[(882 * 2) + 16];
	audio_write_sound_buffer(buf);
	
	unsigned int length = (audio_seg_length * 2);
	if (audio_get_stereo())
		length *= 2;
	
	/* Write the sound data. */
	fwrite(buf, 1, length, WAV_File);
	return 0;
}