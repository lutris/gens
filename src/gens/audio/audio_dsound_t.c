/***************************************************************************
 * Gens: Audio Handler - DirectSound 3 Backend. (audio_backend_t struct)   *
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

#include "audio_dsound_t.h"
#include "audio_dsound.hpp"

// Audio Backend struct.
const audio_backend_t audio_backend_dsound =
{
	.init = audio_dsound_init,
	.end = audio_dsound_end,
	
	.write_sound_buffer = audio_dsound_write_sound_buffer,
	.clear_sound_buffer = audio_dsound_clear_sound_buffer,
	
	.play_sound = audio_dsound_play_sound,
	.stop_sound = audio_dsound_stop_sound,
	
	.wait_for_audio_buffer = audio_dsound_wait_for_audio_buffer,
 
	// Win32-specific functions.
	.wp_seg_wait = audio_dsound_wp_seg_wait,
	.wp_inc = audio_dsound_wp_inc
};
