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

#ifndef GENS_WAVE_H
#define GENS_WAVE_H

/* C includes. */
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* WAV header struct. */
/* Description from http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
/* All values are little-endian, except for character fields. */
typedef struct _wav_header_t
{
	/* RIFF chunk descriptor. */
	struct
	{
		char		ChunkID[4];	/* Contains "RIFF". */
		uint32_t	ChunkSize;	/* Size of the whole WAV, minus 8. */
		char		Format[4];	/* Contains "WAVE". */
	} riff;
	
	/* "fmt " sub-chunk. */
	struct
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
	
	struct
	{
		char		SubchunkID[4];	/* Contains "data". */
		uint32_t	SubchunkSize;	/* Size of the whole WAV, minus 8, minus size of riff and fmt. */
	} data;
} wav_header_t;

extern int WAV_Dumping;

/* WAV dump functions. */
int wav_dump_start(void);
int wav_dump_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_WAVE_H */
