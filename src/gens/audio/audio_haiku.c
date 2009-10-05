/***************************************************************************
 * Gens: Audio Handler - Haiku Backend.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Copyright (c) 2009 by Phil Costin                                       *
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

// MMX audio functions.
#ifdef GENS_X86_ASM
#include "audio_mmx.h"
#endif /* GENS_X86_ASM */

// Gens includes.
#include "emulator/g_main.hpp"

// CPU flags.
#include "gens_core/misc/cpuflags.h"
#include "mdp/mdp_cpuflags.h"

// C includes.
#include <string.h>
#include <time.h>

// Function prototypes.
static int	audio_haiku_init(void);
static int	audio_haiku_end(void);

static int	audio_haiku_write_sound_buffer(short *dump_buf);
static void	audio_haiku_clear_sound_buffer(void);

static void	audio_haiku_wait_for_audio_buffer(void);

// Haiku audio variables.
static int audio_haiku_len;
static unsigned char *audio_haiku_audiobuf = NULL;

// Haiku functions.
static void audio_haiku_callback(void *user, uint8_t *buffer, int len);

// Audio Backend struct.
const audio_backend_t audio_backend_haiku =
{
	.init = audio_haiku_init,
	.end = audio_haiku_end,
	
	.write_sound_buffer = audio_haiku_write_sound_buffer,
	.clear_sound_buffer = audio_haiku_clear_sound_buffer,
	
	.play_sound = NULL,
	.stop_sound = NULL,
	
	.wait_for_audio_buffer = audio_haiku_wait_for_audio_buffer
};


/**
 * audio_haiku_init(): Initialize the Haiku audio subsystem.
 * @return 0 on success; non-zero on error.
 */
static int audio_haiku_init(void)
{
	return 0;
}


/**
 * audio_haiku_end(): Shut down the Haiku audio subsystem.
 * @return 0 on success; non-zero on error.
 */
static int audio_haiku_end(void)
{
	return 0;
}


/**
 * audio_haiku_callback(): Haiku audio callback.
 * @param user
 * @param buffer
 * @param len
 */
static void audio_haiku_callback(void *user, uint8_t *buffer, int len)
{
}


/**
 * audio_haiku_write_sound_buffer(): Write the sound buffer to the audio output.
 * @param dump_buf Sound dumping buffer.
 * @return 0 on success; non-zero on error.
 */
static int audio_haiku_write_sound_buffer(short *dump_buf)
{
	return 0;
}


/**
 * audio_haiku_clear_sound_buffer(): Clear the sound buffer.
 */
static void audio_haiku_clear_sound_buffer(void)
{
}


/**
 * audio_haiku_wait_for_audio_buffer(): Wait for the audio buffer to empty out.
 * This function is used for Auto Frame Skip.
 */
static void audio_haiku_wait_for_audio_buffer(void)
{
}
