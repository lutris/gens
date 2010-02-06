/***************************************************************************
 * Gens: Audio Handler - Write Functions.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "audio_write.h"
#include "audio.h"


/**
 * audio_write_sound_stereo(): Write a stereo sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void audio_write_sound_stereo(short *dest, int length)
{
	int i, out_L, out_R;

	for (i = 0; i < audio_seg_length; i++)
	{
		// Left channel
		out_L = Seg_L[i];
		Seg_L[i] = 0;
		
		if (out_L < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_L > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = (short)(out_L);
		
		// Right channel
		out_R = Seg_R[i];
		Seg_R[i] = 0;
		
		if (out_R < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_R > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = (short)(out_R);
	}
}


/**
 * audio_dump_sound_stereo(): Dump a stereo sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void audio_dump_sound_stereo(short *dest, int length)
{
	int i, out_L, out_R;
	
	for (i = 0; i < audio_seg_length; i++)
	{
		// Left channel
		out_L = Seg_L[i];
		
		if (out_L < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_L > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = (short)(out_L);
		
		// Right channel
		out_R = Seg_R[i];
		
		if (out_R < -0x7FFF)
			*dest++ = -0x7FFF;
		else if (out_R > 0x7FFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = (short)(out_R);
	}
}


/**
 * audio_write_sound_mono(): Write a mono sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void audio_write_sound_mono(short *dest, int length)
{
	int i, out;
	
	for (i = 0; i < audio_seg_length; i++)
	{
		out = Seg_L[i] + Seg_R[i];
		Seg_L[i] = Seg_R[i] = 0;
		
		if (out < -0xFFFF)
			*dest++ = -0x7FFF;
		else if (out > 0xFFFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = (short)(out >> 1);
	}
}


/**
 * audio_dump_sound_mono(): Dump a mono sound segment.
 * @param dest Destination buffer.
 * @param length Length of the sound segment. (TODO: Unused?)
 */
void audio_dump_sound_mono(short *dest, int length)
{
	int i, out;
	
	for (i = 0; i < audio_seg_length; i++)
	{
		out = Seg_L[i] + Seg_R[i];
		
		if (out < -0xFFFF)
			*dest++ = -0x7FFF;
		else if (out > 0xFFFF)
			*dest++ = 0x7FFF;
		else
			*dest++ = (short)(out >> 1);
	}
}
