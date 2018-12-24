/***************************************************************************
 * Gens: Audio Handler - Base Code.                                        *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "audio.h"

// Gens includes.
#include "gens_core/mem/mem_m68k.h"

// C includes.
#include <string.h>

// Audio backends.
#if defined(GENS_OS_WIN32)
	#include "audio_dsound_t.h"
#elif defined(GENS_OS_UNIX)
	#include "audio_sdl.h"
#elif defined(GENS_OS_HAIKU)
	#include "audio_haiku.h"
#endif

static const audio_backend_t * const audio_backends[] =
{
	#if defined(GENS_OS_WIN32)
		&audio_backend_dsound,
	#elif defined(GENS_OS_UNIX)
		&audio_backend_sdl,
	#elif defined(GENS_OS_HAIKU)
		&audio_backend_haiku,
	#endif
};

// Current backend.
const audio_backend_t *audio_cur_backend = NULL;
AUDIO_BACKEND audio_cur_backend_id;

// Function pointers.
int	(*audio_write_sound_buffer)(short *dump_buf) = NULL;
int	(*audio_play_sound)(void) = NULL;
int	(*audio_stop_sound)(void) = NULL;
void	(*audio_wait_for_audio_buffer)(void) = NULL;
#ifdef GENS_OS_WIN32
void	(*audio_wp_seg_wait)(void) = NULL;
void	(*audio_wp_inc)(void) = NULL;
#endif /* GENS_OS_WIN32 */

// Audio data.
int Seg_L[882], Seg_R[882];
unsigned int Sound_Extrapol[312][2];
unsigned int Sound_Interpol[882];

// External values. (TODO: Make these properties.)
int	audio_seg_length;
BOOL	audio_initialized = FALSE;
BOOL	audio_sound_is_playing = FALSE;
int	audio_seg_to_buffer = 8; // for frame skip

// Properties.
static int	audio_sound_rate = 44100;
static BOOL	audio_enabled = TRUE;
static BOOL	audio_stereo = TRUE;
static BOOL	audio_gym_playing = FALSE;


/**
 * audio_init(): Initialize the Audio subsystem.
 * @param backend Audio backend to initialize.
 * @return 0 on success; non-zero on error.
 */
int audio_init(AUDIO_BACKEND backend)
{
	if (backend < 0 || backend >= AUDIO_BACKEND_MAX)
	{
		// Invalid backend.
		return -1;
	}
	
	if (audio_cur_backend)
	{
		// Backend is currently initialized.
		return -2;
	}
	
	// Set up the variables.
	audio_cur_backend = audio_backends[backend];
	audio_cur_backend_id = backend;
	
	// Copy the function pointers.
	audio_write_sound_buffer = audio_cur_backend->write_sound_buffer;
	audio_play_sound = audio_cur_backend->play_sound;
	audio_stop_sound = audio_cur_backend->stop_sound;
	audio_wait_for_audio_buffer = audio_cur_backend->wait_for_audio_buffer;
#ifdef GENS_OS_WIN32
	audio_wp_seg_wait = audio_cur_backend->wp_seg_wait;
	audio_wp_inc = audio_cur_backend->wp_inc;
#endif /* GENS_OS_WIN32 */
	
	// Calculate the segment length.
	audio_calc_segment_length();
	
	// Build the sound extrapolation and interpolation tables.
	int i;
	int videoLines = (CPU_Mode ? 312 : 262);
	for (i = 0; i < videoLines; i++)
	{
		Sound_Extrapol[i][0] = ((audio_seg_length * i) / videoLines);
		Sound_Extrapol[i][1] = (((audio_seg_length * (i + 1)) / videoLines) - Sound_Extrapol[i][0]);
	}
	for (i = 0; i < audio_seg_length; i++)
		Sound_Interpol[i] = ((videoLines * i) / audio_seg_length);
	
	// Clear the segment buffers.
	memset(Seg_L, 0x00, sizeof(Seg_L));
	memset(Seg_R, 0x00, sizeof(Seg_R));
	
	// Initialize the backend.
	if (audio_cur_backend->init)
		return audio_cur_backend->init();
	
	// Initialized successfully.
	return 0;
}


/**
 * audio_end(): Shut down the Audio backend.
 * @return 0 on success; non-zero on error.
 */
int audio_end(void)
{
	if (!audio_cur_backend)
	{
		// No Audio backend is initialized.
		return -1;
	}
	
	// Shut down the Audio backend.
	if (audio_cur_backend->end)
		audio_cur_backend->end();
	
	audio_cur_backend = NULL;
	return 0;
}


/**
 * audio_clear_sound_buffer(): Clear the sound buffer.
 */
void audio_clear_sound_buffer(void)
{
	if (!audio_cur_backend || !audio_cur_backend->clear_sound_buffer)
		return;
	
	audio_cur_backend->clear_sound_buffer();
}


/**
 * audio_calc_segment_length(): Calculate the segment length based on the sound rate.
 */
void audio_calc_segment_length(void)
{
	switch (audio_sound_rate)
	{
		case 11025:
			audio_seg_length = (CPU_Mode ? 220 : 184);
			break;
		case 22050:
			audio_seg_length = (CPU_Mode ? 441 : 368);
			break;
		case 44100:
			audio_seg_length = (CPU_Mode ? 882 : 735);
			break;
	}
}


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


/** Properties **/


int audio_get_sound_rate(void)
{
	return audio_sound_rate;
}
void audio_set_sound_rate(const int new_sound_rate)
{
	audio_sound_rate = new_sound_rate;
	// TODO: Adjust the audio subsystem to use the new rate.
}


BOOL audio_get_gym_playing(void)
{
	return audio_gym_playing;
}
void audio_set_gym_playing(const BOOL new_gym_playing)
{
	audio_gym_playing = (new_gym_playing ? TRUE : FALSE);
	// TODO: GYM code.
}


BOOL audio_get_enabled(void)
{
	return audio_enabled;
}
void audio_set_enabled(const BOOL new_enabled)
{
	audio_enabled = (new_enabled ? TRUE : FALSE);
	// TODO: Enabled code.
}


BOOL audio_get_stereo(void)
{
	return audio_stereo;
}
void audio_set_stereo(const BOOL new_stereo)
{
	audio_stereo = (new_stereo ? TRUE : FALSE);
	// TODO: Stereo code.
}
