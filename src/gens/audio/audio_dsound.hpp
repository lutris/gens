/**
 * Gens: Audio class - DirectSound
 */

#ifndef GENS_AUDIO_DSOUND_HPP
#define GENS_AUDIO_DSOUND_HPP

#include "audio.hpp"

// DirectSound 3 is required.
#define DIRECTSOUND_VERSION 0x0300

#include <windows.h>
#include <dsound.h>

class Audio_DSound : public Audio
{
	public:
		Audio_DSound();
		~Audio_DSound();
		
		// Initialize / End sound.
		int initSound(void);
		void endSound(void);
		
		// Miscellaneous functions that need to be sorted through.
		int getCurrentSeg(void);
		int checkSoundTiming(void);
		int writeSoundBuffer(void *dumpBuf);
		int clearSoundBuffer(void);
		int playSound(void);
		int stopSound(void);
		
	protected:
		// DirectSound stuff
		LPDIRECTSOUND lpDS;
		WAVEFORMATEX MainWfx;
		DSBUFFERDESC dsbdesc;
		LPDIRECTSOUNDBUFFER lpDSPrimary, lpDSBuffer;
		HMMIO MMIOOut;
		MMCKINFO CkOut;
		MMCKINFO CkRIFF;
		MMIOINFO MMIOInfoOut;
		
		int Bytes_Per_Unit;
		
		// TODO: Figure out what these things do.
		int WP, RP;
};

#endif /* GENS_AUDIO_DSOUND_HPP */
