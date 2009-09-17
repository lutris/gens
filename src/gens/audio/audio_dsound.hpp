/***************************************************************************
 * Gens: Audio Handler - DirectSound 3 Backend.                            *
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

#ifndef GENS_AUDIO_DSOUND_HPP
#define GENS_AUDIO_DSOUND_HPP

#include "libgsft/gsft_bool.h"
#include "audio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes.
int	audio_dsound_init(void);
int	audio_dsound_end(void);

int	audio_dsound_write_sound_buffer(short *dump_buf);
void	audio_dsound_clear_sound_buffer(void);

int	audio_dsound_play_sound(void);
int	audio_dsound_stop_sound(void);

void	audio_dsound_wait_for_audio_buffer(void);

// Win32-specific functions.
void	audio_dsound_wp_seg_wait(void);
void	audio_dsound_wp_inc(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_AUDIO_DSOUND_HPP */
