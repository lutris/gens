/***************************************************************************
 * Gens: Audio Handler - Base Code.                                        *
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

#ifndef GENS_AUDIO_H
#define GENS_AUDIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "libgsft/gsft_bool.h"


// Audio backends.
typedef enum
{
	#ifdef GENS_OS_WIN32
		AUDIO_BACKEND_DSOUND,
	#else /* !GENS_OS_WIN32 */
		AUDIO_BACKEND_SDL,
	#endif /* GENS_OS_WIN32 */
	AUDIO_BACKEND_MAX
} AUDIO_BACKEND;

// Assuming only one backend per platform right now.
#ifdef GENS_OS_WIN32
	#define AUDIO_BACKEND_DEFAULT AUDIO_BACKEND_DSOUND
#else /* !GENS_OS_WIN32 */
	#define AUDIO_BACKEND_DEFAULT AUDIO_BACKEND_SDL
#endif /* GENS_OS_WIN32 */

// Audio backend function pointers.
typedef struct
{
	int	(*init)(void);
	int	(*end)(void);
	
	int	(*write_sound_buffer)(short *dump_buf);
	void	(*clear_sound_buffer)(void);
	
	int	(*play_sound)(void);
	int	(*stop_sound)(void);
	
	void	(*wait_for_audio_buffer)(void);
	
#ifdef GENS_OS_WIN32
	// Win32-specific functions.
	void	(*wp_seg_wait)(void);
	void	(*wp_inc)(void);
#endif /* GENS_OS_WIN32 */
} audio_backend_t;

int	audio_init(AUDIO_BACKEND backend);
int	audio_end(void);

// Current backend.
extern const audio_backend_t *audio_cur_backend;
extern AUDIO_BACKEND audio_cur_backend_id;

// Clear the sound buffer.
void	audio_clear_sound_buffer(void);

// Function pointers.
extern int	(*audio_write_sound_buffer)(short *dump_buf);
extern int	(*audio_play_sound)(void);
extern int	(*audio_stop_sound)(void);
extern void	(*audio_wait_for_audio_buffer)(void);
#ifdef GENS_OS_WIN32
extern void	(*audio_wp_seg_wait)(void);
extern void	(*audio_wp_inc)(void);
#endif /* GENS_OS_WIN32 */

// Write sound data.
void	audio_write_sound_stereo(short *dest, int length);
void	audio_dump_sound_stereo(short *dest, int length);
void	audio_write_sound_mono(short *dest, int length);
void	audio_dump_sound_mono(short *dest, int length);

// Audio data.
extern int Seg_L[882], Seg_R[882];
extern unsigned int Sound_Extrapol[312][2];
extern unsigned int Sound_Interpol[882];

// External values. (TODO: Make these properties.)
extern int	audio_seg_length;
extern BOOL	audio_initialized;
extern BOOL	audio_sound_is_playing;
extern int	audio_seg_to_buffer; // for frame skip

// Set segment length based on sound rate.
void	audio_calc_segment_length(void);

// Properties.
int	audio_get_sound_rate(void);
void	audio_set_sound_rate(const int new_sound_rate);
BOOL	audio_get_gym_playing(void);
void	audio_set_gym_playing(const BOOL new_gym_playing);
BOOL	audio_get_enabled(void);
void	audio_set_enabled(const BOOL new_enabled);
BOOL	audio_get_stereo(void);
void	audio_set_stereo(const BOOL new_stereo);

#ifdef __cplusplus
}
#endif

#endif /* GENS_AUDIO_H */
