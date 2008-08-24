#include "port.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <string.h>
#include "g_sdlsound.h"
#include "psg.h"
#include "ym2612.h"
#include "mem_m68k.h"
#include "vdp_io.h"
#include "pcm.h"
#include "misc.h"		// for Have_MMX flag

int Seg_To_Buffer = 8; // for frame skip
int Seg_L[882], Seg_R[882];
int Seg_Length, SBuffer_Length;
int Sound_Rate = 44100, Sound_Segs = 8;
int Bytes_Per_Unit = 4;
int Sound_Enable;
int Sound_Stereo = 1;
int Sound_Is_Playing = 0;
int Sound_Initialised = 0;
int WAV_Dumping = 0;
int GYM_Playing = 0;
unsigned int Sound_Interpol[882];
unsigned int Sound_Extrapol[312][2];

char Dump_Dir[1024] = "";
char Dump_GYM_Dir[1024] = "";

static int audio_len = 0;
unsigned char *pMsndOut = 0;

unsigned char *audiobuf = 0;

void proc (void *user, Uint8 * buffer, int len)
{
	if (audio_len < (int)len)
	{
		memcpy (buffer, user, audio_len);
		audio_len = 0;
		return;
	}
	memcpy (buffer, user, len);
	audio_len -= len;
	memcpy (user, (unsigned char *) user + len, audio_len);
}


/**
 * Init_Sound(): Initialize sound.
 * @return 1 on success; -1 if sound was already initialized; 0 on error.
 */
int Init_Sound(void)
{
	int i;
	int videoLines;
	
	if (Sound_Initialised)
		return -1;
	
	End_Sound ();
	
	switch (Sound_Rate)
	{
		case 11025:
			Seg_Length = (CPU_Mode ? 220 : 184);
			break;
		case 16000:
			Seg_Length = (CPU_Mode ? 330 : 276);
			break;
		case 22050:
			Seg_Length = (CPU_Mode ? 441 : 368);
			break;
		case 32000:
			Seg_Length = (CPU_Mode ? 660 : 552);
			break;
		case 44100:
			Seg_Length = (CPU_Mode ? 882 : 735);
			break;
		case 48000:
			Seg_Length = (CPU_Mode ? 990 : 828);
			break;
	}
	
	videoLines = (CPU_Mode ? 312 : 262);
	for (i = 0; i < videoLines; i++)
	{
		Sound_Extrapol[i][0] = ((Seg_Length * i) / videoLines);
		Sound_Extrapol[i][1] = (((Seg_Length * (i + 1)) / videoLines) - Sound_Extrapol[i][0]);
	}
	for (i = 0; i < Seg_Length; i++)
		Sound_Interpol[i] = ((videoLines * i) / Seg_Length);
	
	memset (Seg_L, 0, Seg_Length << 2);
	memset (Seg_R, 0, Seg_Length << 2);
	
	if (SDL_InitSubSystem (SDL_INIT_AUDIO) == -1)
		return 0;
	
	pMsndOut = (unsigned char *) malloc (Seg_Length << 2);
	
	SDL_AudioSpec spec;
	
	spec.freq = Sound_Rate;
	spec.format = AUDIO_S16SYS;
	spec.channels = (Sound_Stereo == 0) ? 1 : 2; // TODO: Initializing 1 channel seems to double-free if it's later changed...
	spec.samples = 1024;
	spec.callback = proc;
	audiobuf = (unsigned char *) malloc ((spec.samples * spec.channels * 2 * 4) * sizeof (short));
	
	spec.userdata = audiobuf;
	
	memset(audiobuf, 0, (spec.samples * spec.channels * 2 * 4) * sizeof (short));
	if (SDL_OpenAudio(&spec, 0) != 0)
		return 0;
	SDL_PauseAudio(0);
	
	Sound_Initialised = 1;
	return 1;
}


/**
 * End_Sound(): Stop sound output.
 */
void End_Sound(void)
{
	SDL_PauseAudio(1);
	free(audiobuf);
	audiobuf = NULL;
	
	free(pMsndOut);
	pMsndOut = NULL;
	
	if (Sound_Initialised)
	{
		Sound_Is_Playing = 0;
		Sound_Initialised = 0;
	}
	SDL_QuitSubSystem (SDL_INIT_AUDIO);
}


int Get_Current_Seg (void)
{
	// We aren't using DIRECTX so these won't work :(
	
	/* 
	unsigned long R;
	
	lpDSBuffer->GetCurrentPosition(&R, NULL);
	return(R / (Seg_Length * Bytes_Per_Unit));
	*/
	
}

int Lots_In_Audio_Buffer(void)
{
	return (audio_len > Seg_Length * Seg_To_Buffer);
}

int Check_Sound_Timing (void)
{
  return 0;
}


void Write_Sound_Stereo (short *Dest, int length)
{
  int i, out_L, out_R;
  short *dest = Dest;

  for (i = 0; i < Seg_Length; i++)
    {
      out_L = Seg_L[i];
      Seg_L[i] = 0;

      if (out_L < -0x7FFF) *dest++ = -0x7FFF;
      else if (out_L > 0x7FFF) *dest++ = 0x7FFF;
      else *dest++ = (short) out_L;

      out_R = Seg_R[i];
      Seg_R[i] = 0;

      if (out_R < -0x7FFF) *dest++ = -0x7FFF;
      else if (out_R > 0x7FFF) *dest++ = 0x7FFF;
      else *dest++ = (short) out_R;
    }
}


void Dump_Sound_Stereo (short *Dest, int length)
{
  int i, out_L, out_R;
  short *dest = Dest;

  for (i = 0; i < Seg_Length; i++)
    {
      out_L = Seg_L[i];

      if (out_L < -0x7FFF) *dest++ = -0x7FFF;
      else if (out_L > 0x7FFF) *dest++ = 0x7FFF;
      else *dest++ = (short) out_L;

      out_R = Seg_R[i];

      if (out_R < -0x7FFF) *dest++ = -0x7FFF;
      else if (out_R > 0x7FFF) *dest++ = 0x7FFF;
      else *dest++ = (short) out_R;
    }
}


/**
 * Write_Sound_Mono(): Write a mono sound segment.
 * @param Dest Destination buffer.
 * @param length Length of the sound segment.
 */
void Write_Sound_Mono (short *Dest, int length)
{
	int i, out;
	short *dest = Dest;
	
	for (i = 0; i < Seg_Length; i++)
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


void Dump_Sound_Mono (short *Dest, int length)
{
  int i, out;
  short *dest = Dest;

  for (i = 0; i < Seg_Length; i++)
    {
      out = Seg_L[i] + Seg_R[i];

      if (out < -0xFFFF) *dest++ = -0x7FFF;
      else if (out > 0xFFFF) *dest++ = 0x7FFF;
      else*dest++ = (short) (out >> 1);
    }
}


/**
 * Write_Sound_Buffer(): Write the sound buffer to the audio output.
 * @param Dump_BUf Sound buffer.
 * @return 1 on success.
 */
int Write_Sound_Buffer (void *Dump_Buf)
{
	struct timespec rqtp = { 0, 1000000 };
	
	SDL_LockAudio ();
	
	if (Sound_Stereo)
	{
		if (Have_MMX)
			Write_Sound_Stereo_MMX(Seg_L, Seg_R, (short *)pMsndOut, Seg_Length);
		else
			Write_Sound_Stereo ((short *) pMsndOut, Seg_Length);
	}
	else
	{
		if (Have_MMX)
			Write_Sound_Mono_MMX (Seg_L, Seg_R, (short *)pMsndOut, Seg_Length);
		else
			Write_Sound_Mono ((short *) pMsndOut, Seg_Length);
	}
	
	memcpy(audiobuf + audio_len, pMsndOut, Seg_Length * 4);
	audio_len += Seg_Length * 4;
	
	SDL_UnlockAudio();
	
	while (audio_len > 1024 * 2 * 2 * 4)
	{
		nanosleep (&rqtp, NULL);	
		if (fast_forward)
			audio_len = 1024;
	} //SDL_Delay(1); 
	return 1;
}


int Clear_Sound_Buffer (void)
{
#if 0
  LPVOID lpvPtr1;
  DWORD dwBytes1;
  HRESULT rval;
  int i;

  if (!Sound_Initialised) return 0;

  rval = lpDSBuffer->Lock (0, Seg_Length * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);

  if (rval == DSERR_BUFFERLOST)
    {
      lpDSBuffer->Restore ();
      rval = lpDSBuffer->Lock (0, Seg_Length * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);

    }

  if (rval == DS_OK)
    {
      signed short *w = (signed short *) lpvPtr1;

      for (i = 0; i < Seg_Length * Sound_Segs * Bytes_Per_Unit; i += 2)
	*w++ = (signed short) 0;

      rval = lpDSBuffer->Unlock (lpvPtr1, dwBytes1, NULL, NULL);

      if (rval == DS_OK) return 1;
    }
#endif
  return 0;
}


int Play_Sound (void)
{
#if 0
  HRESULT rval;

  if (Sound_Is_Playing)  return 1;

  rval = lpDSBuffer->Play (0, 0, DSBPLAY_LOOPING);

  Clear_Sound_Buffer ();

  if (rval != DS_OK) return 0;
#endif
  return (Sound_Is_Playing = 1);
}


int Stop_Sound (void)
{
#if 0
  HRESULT rval;

  rval = lpDSBuffer->Stop ();

  if (rval != DS_OK) return 0;

  Sound_Is_Playing = 0;
#endif
  return 1;
}


int Start_WAV_Dump (void)
{
#if 0
  char Name[1024] = "";

  if (!(Sound_Is_Playing) || !(Game)) return (0);

  if (WAV_Dumping)
    {
      Put_Info ("WAV sound is already dumping", 1000);
      return (0);
    }

  strcpy (Name, Dump_Dir);
  strcat (Name, Rom_Name);

  if (WaveCreateFile (Name, &MMIOOut, &MainWfx, &CkOut, &CkRIFF))
    {
      Put_Info ("Error in WAV dumping", 1000);
      return (0);
    }

  if (WaveStartDataWrite (&MMIOOut, &CkOut, &MMIOInfoOut))
    {
      Put_Info ("Error in WAV dumping", 1000);
      return (0);
    }

  Put_Info ("Starting to dump WAV sound", 1000);
  WAV_Dumping = 1;
#endif
  return 1;
}


int Update_WAV_Dump (void)
{
#if 0
  unsigned char Buf_Tmp[882 * 4 + 16];
  unsigned int length, Writted;

  if (!WAV_Dumping) return 0;

  Write_Sound_Buffer (Buf_Tmp);

  length = Seg_Length << 1;

  if (Sound_Stereo) length *= 2;

  if (WaveWriteFile (MMIOOut, length, &Buf_Tmp[0], &CkOut, &Writted, &MMIOInfoOut))
    {
      Put_Info ("Error in WAV dumping", 1000);
      return 0;
    }
#endif
  return (1);
}


int Stop_WAV_Dump (void)
{
#if 0
  if (!WAV_Dumping)
    {
      Put_Info ("Already stopped", 1000);
      return 0;
    }

  if (WaveCloseWriteFile (&MMIOOut, &CkOut, &CkRIFF, &MMIOInfoOut, 0))
    return 0;

  Put_Info ("WAV dump stopped", 1000);
  WAV_Dumping = 0;
#endif
  return 1;
}
