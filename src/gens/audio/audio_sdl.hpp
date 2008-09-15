/**
 * Gens: Audio class - SDL
 */

#ifndef GENS_AUDIO_SDL_HPP
#define GENS_AUDIO_SDL_HPP

#include "audio.hpp"

class Audio_SDL : public Audio
{
	public:
		Audio_SDL();
		~Audio_SDL();
		
		// Initialize / End sound.
		int initSound(void);
		void endSound(void);
		
		// Audio callback functions.
		static void AudioCallback(void *user, Uint8 *buffer, int len);
		void audioCallback_int(Uint8 *buffer, int len);
		
		// Miscellaneous functions that need to be sorted through.
		int getCurrentSeg(void);
		int checkSoundTiming(void);
		int writeSoundBuffer(void *dumpBuf);
		int clearSoundBuffer(void);
		int playSound(void);
		int stopSound(void);
};

#endif /* GENS_AUDIO_SDL_HPP */
