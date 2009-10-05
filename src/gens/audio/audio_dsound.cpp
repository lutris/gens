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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "audio.h"
#include "audio_dsound.hpp"

// Message logging.
#include "macros/log_msg.h"

// MMX audio functions.
#ifdef GENS_X86_ASM
#include "audio_mmx.h"
#endif /* GENS_X86_ASM */

// Gens includes.
#include "gens/gens_window.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"
#include "mdp/mdp_cpuflags.h"

// Input Handler - Update Controllers.
#include "input/input_update.h"

// C includes.
#include <string.h>
#include <time.h>

// DirectSound 3 is required.
#define DIRECTSOUND_VERSION 0x0300
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

// DirectSound variables.
static LPDIRECTSOUND lpDS = NULL;
static WAVEFORMATEX MainWfx;
static DSBUFFERDESC dsbdesc;
static LPDIRECTSOUNDBUFFER lpDSPrimary = NULL;
static LPDIRECTSOUNDBUFFER lpDSBuffer = NULL;

static int Bytes_Per_Unit;
static int Sound_Segs = 8;

// DirectSound write and read pointers.
static int WP, RP;

// DirectSound functions.
static int audio_dsound_get_current_seg(void);
int audio_dsound_check_sound_timing(void);

// Cooperative level (Win32)
static void audio_dsound_set_cooperative_level(void);


/**
 * audio_dsound_init(): Initialize the DirectSound subsystem.
 * @return 0 on success; non-zero on error.
 */
int audio_dsound_init(void)
{
	HRESULT rval;
	WAVEFORMATEX wfx;
	
	if (audio_initialized)
		return -1;
	
	// Make sure sound is shut down first.
	audio_dsound_end();
	
	// Attempt to initialize DirectSound.
	rval = DirectSoundCreate(NULL, &lpDS, NULL);
	if (rval != DS_OK)
		return -1;
	
	// Set the DirectSound cooperative level.
	audio_dsound_set_cooperative_level();
	
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	
	rval = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSPrimary, NULL);
	if (rval != DS_OK)
	{
		lpDS->Release();
		lpDS = NULL;
		return -2;
	}
	
	Bytes_Per_Unit = 2 * (audio_get_stereo() ? 2 : 1);
	
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (audio_get_stereo() ? 2 : 1);
	wfx.nSamplesPerSec = audio_get_sound_rate();
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = Bytes_Per_Unit;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * Bytes_Per_Unit;
	
	rval = lpDSPrimary->SetFormat(&wfx);
	if (rval != DS_OK)
	{
		lpDSPrimary->Release();
		lpDSPrimary = NULL;
		lpDS->Release();
		lpDS = NULL;
		return -3;
	}
	
	memset(&MainWfx, 0, sizeof(WAVEFORMATEX));
	MainWfx.wFormatTag = WAVE_FORMAT_PCM;
	MainWfx.nChannels = (audio_get_stereo() ? 2 : 1);
	MainWfx.nSamplesPerSec = audio_get_sound_rate();
	MainWfx.wBitsPerSample = 16;
	MainWfx.nBlockAlign = Bytes_Per_Unit;
	MainWfx.nAvgBytesPerSec = MainWfx.nSamplesPerSec * Bytes_Per_Unit;
	
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
	dsbdesc.dwBufferBytes = audio_seg_length * Sound_Segs * Bytes_Per_Unit;
	dsbdesc.lpwfxFormat = &MainWfx;
	
	LOG_MSG(audio, LOG_MSG_LEVEL_DEBUG1,
		"Seg l : %d   Num Seg : %d   Taille : %d", audio_seg_length, Sound_Segs, Bytes_Per_Unit);
	
	rval = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL);
	if (rval != DS_OK)
	{
		lpDS->Release();
		lpDS = NULL;
		return -4;
	}
	
	// Sound is initialized.
	audio_initialized = TRUE;
	return 0;
}


/**
 * audio_dsound_end(): Shut down the DirectSound subsystem.
 * @return 0 on success; non-zero on error.
 */
int audio_dsound_end(void)
{
	audio_sound_is_playing = FALSE;
	audio_initialized = FALSE;
	
	if (lpDSPrimary)
	{
		lpDSPrimary->Release();
		lpDSPrimary = NULL;
	}
	
	if (lpDSBuffer)
	{
		lpDSBuffer->Stop();
		lpDSBuffer->Release();
		lpDSBuffer = NULL;
	}
	
	if (lpDS)
	{
		lpDS->Release();
		lpDS = NULL;
	}
	
	return 0;
}


/**
 * audio_dsound_get_current_seg(): Get the current DSound segment.
 * @return DSound segment.
 */
static int audio_dsound_get_current_seg(void)
{
	DWORD R;
	
	lpDSBuffer->GetCurrentPosition(&R, NULL);
	return (R / (audio_seg_length * Bytes_Per_Unit));
}


/**
 * audio_dsound_check_sound_timing(): Check sound timing.
 * @return ???
 */
int audio_dsound_check_sound_timing(void)
{
	DWORD R;
	
	lpDSBuffer->GetCurrentPosition(&R, NULL);
	
	RP = R / (audio_seg_length * Bytes_Per_Unit);
	if (RP == ((WP + 1) & (Sound_Segs - 1)))
		return 2;
	
	if (RP != WP)
		return 1;
	
	return 0;
}


/**
 * audio_dsound_write_sound_buffer(): Write the sound buffer to the audio output.
 * @param dump_buf Sound dumping buffer.
 * @return 0 on success; non-zero on error.
 */
int audio_dsound_write_sound_buffer(short *dump_buf)
{
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	HRESULT rval;
	
	if (dump_buf)
	{
		if (audio_get_stereo())
			audio_dump_sound_stereo(dump_buf, audio_seg_length);
		else
			audio_dump_sound_mono(dump_buf, audio_seg_length);
		return 0;
	}
	
	rval = lpDSBuffer->Lock(WP * audio_seg_length * Bytes_Per_Unit, audio_seg_length * Bytes_Per_Unit,
				&lpvPtr1, &dwBytes1, NULL, NULL, 0);
	
	if (rval == DSERR_BUFFERLOST)
	{
		lpDSBuffer->Restore();
		rval = lpDSBuffer->Lock(WP * audio_seg_length * Bytes_Per_Unit, audio_seg_length * Bytes_Per_Unit,
					&lpvPtr1, &dwBytes1, NULL, NULL, 0);
	}
	
	if (rval == DSERR_BUFFERLOST || !lpvPtr1)
		return -1;
	
	// TODO: Master Volume support
#if 0
	for(int i = 0; i < Seg_Length; i++) 
	{
		Seg_R[i] *= MastVol;
		Seg_R[i] >>= 8;
		Seg_L[i] *= MastVol;
		Seg_L[i] >>= 8;
	}
#endif
	
	if (audio_get_stereo())
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & MDP_CPUFLAG_X86_MMX)
			audio_write_sound_stereo_x86_mmx(Seg_L, Seg_R, reinterpret_cast<short*>(lpvPtr1), audio_seg_length);
		else
#endif
			audio_write_sound_stereo(reinterpret_cast<short*>(lpvPtr1), audio_seg_length);
	}
	else
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & MDP_CPUFLAG_X86_MMX)
			audio_write_sound_mono_x86_mmx(Seg_L, Seg_R, reinterpret_cast<short*>(lpvPtr1), audio_seg_length);
		else
#endif
			audio_write_sound_mono(reinterpret_cast<short*>(lpvPtr1), audio_seg_length);
	}
	
	lpDSBuffer->Unlock(lpvPtr1, dwBytes1, NULL, 0);
	
	return 0;
}


/**
 * audio_dsound_clear_sound_buffer(): Clear the sound buffer.
 */
void audio_dsound_clear_sound_buffer(void)
{
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	HRESULT rval;
	int i;

	if (!audio_initialized)
		return;
	
	rval = lpDSBuffer->Lock(0, audio_seg_length * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);

	if (rval == DSERR_BUFFERLOST)
	{
		lpDSBuffer->Restore();
		rval = lpDSBuffer->Lock(0, audio_seg_length * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);
	}
	
	if (rval != DS_OK)
		return;
	
	signed short *w = reinterpret_cast<signed short*>(lpvPtr1);
	
	for (i = 0; i < (audio_seg_length * Sound_Segs * Bytes_Per_Unit); i += 2)
	{
		*w++ = 0;
	}
	
	rval = lpDSBuffer->Unlock(lpvPtr1, dwBytes1, NULL, 0);
	return;
}


/**
 * audio_dsound_play_sound(): Start playing sound.
 * @return 0 on success; non-zero on error.
 */
int audio_dsound_play_sound(void)
{
	HRESULT rval;
	
	if (audio_sound_is_playing)
		return -1;
	
	rval = lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
	
	audio_dsound_clear_sound_buffer();
	
	if (rval != DS_OK)
		return -2;
	
	audio_sound_is_playing = TRUE;
	return 0;
}


/**
 * audio_dsound_stop_sound(): Stop playing sound.
 * @return 0 on success; non-zero on error.
 */
int audio_dsound_stop_sound(void)
{
	HRESULT rval;
	
	rval = lpDSBuffer->Stop();
	
	if (rval != DS_OK)
		return -1;
	
	audio_sound_is_playing = FALSE;
	return 0;
}


void audio_dsound_wp_seg_wait(void)
{
	while (WP == audio_dsound_get_current_seg())
	{
		// NOTE: This while loop MUST be empty.
		// Adding a Sleep() causes Gens/GS to lag
		// horribly on some systems.
	}
}


void audio_dsound_wp_inc(void)
{
	WP = (WP + 1) & (Sound_Segs - 1);
	if (WP == audio_dsound_get_current_seg())
		WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);
}


/**
 * audio_dsound_wait_for_audio_buffer(): Wait for the audio buffer to empty out.
 * This function is used for Auto Frame Skip.
 */
void audio_dsound_wait_for_audio_buffer(void)
{
	RP = audio_dsound_get_current_seg();
	while (WP != RP)
	{
		audio_write_sound_buffer(NULL);
		WP = (WP + 1) & (Sound_Segs - 1);
		input_update_controllers();
	}
}


/**
 * audio_dsound_set_cooperative_level(): Sets the cooperative level.
 */
static void audio_dsound_set_cooperative_level(void)
{
	if (!gens_window || !lpDS)
		return;
	
	HRESULT rval;
	rval = lpDS->SetCooperativeLevel(gens_window, DSSCL_PRIORITY);
	if (rval != DS_OK)
	{
		LOG_MSG(audio, LOG_MSG_LEVEL_WARNING,
			"lpDS->SetCooperativeLevel() failed.");
		// TODO: Error handling code.
	}
	else
	{
		LOG_MSG(audio, LOG_MSG_LEVEL_INFO,
			"lpDS->SetCooperativeLevel() succeeded.");
	}
}
