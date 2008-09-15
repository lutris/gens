/**
 * Gens: Audio base class.
 */

#ifndef GENS_AUDIO_HPP
#define GENS_AUDIO_HPP

class Audio
{
	public:
		Audio();
		virtual ~Audio();
		
		// Initialize / End sound.
		virtual int initSound(void) = 0;
		virtual void endSound(void) = 0;
		
		// Miscellaneous functions that need to be sorted through.
		virtual int getCurrentSeg(void) = 0;
		bool lotsInAudioBuffer(void);
		virtual int checkSoundTiming(void) = 0;
		virtual int writeSoundBuffer(void *dumpBuf) = 0;
		virtual int clearSoundBuffer(void) = 0;
		virtual int playSound(void) = 0;
		virtual int stopSound(void) = 0;
		
		// WAV dumping.
		// TODO: Move to util/sound/wav.c(pp)
		int startWAVDump(void);
		int updateWAVDump(void);
		int stopWAVDump(void);
		
		// Properties
		int soundRate(void);
		void setSoundRate(int newSoundRate);
		bool dumpingWAV(void);
		void setDumpingWAV(bool newDumpingWAV);
		bool playingGYM(void);
		void setPlayingGYM(bool newPlayingGYM);
		bool enabled(void);
		void setEnabled(bool newEnabled);
		bool stereo(void);
		void setStereo(bool newStereo);
		int segLength(void);
		bool soundInitialized(void);
		
		// TODO: Write property access functions for these variables.
		int Seg_L[882], Seg_R[882];
		unsigned int Sound_Extrapol[312][2];
		
	protected:
		void writeSoundStereo(short *dest, int length);
		void dumpSoundStereo(short *dest, int length);
		void writeSoundMono(short *dest, int length);
		void dumpSoundMono(short *dest, int length);
		
		bool m_soundInitialized;
		
		int Seg_To_Buffer; // for frame skip
		//int Seg_L[882], Seg_R[882];
		int m_segLength, SBuffer_Length;
		int m_soundRate, Sound_Segs;
		int Bytes_Per_Unit;
		bool m_enabled;
		bool m_stereo;
		bool m_soundIsPlaying;
		bool m_dumpingWAV;
		bool m_playingGYM;
		unsigned int Sound_Interpol[882];
		//unsigned int Sound_Extrapol[312][2];
		
		int audio_len;
		unsigned char *pMsndOut;
		unsigned char *audiobuf;
};

#endif /* GENS_AUDIO_HPP */
