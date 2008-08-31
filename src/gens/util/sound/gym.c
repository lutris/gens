#include <string.h>
#include <gtk/gtk.h>

#include "gym.h"
#include "emulator/g_main.h"
#include "sdllayer/g_sdlsound.h"
#include "util/file/rom.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "emulator/gens.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "sdllayer/g_sdldraw.h"

#include "ui-common.h"

FILE *GYM_File;
int GYM_Dumping = 0;

static int
Exists (const char *filename)
{
  FILE *f = fopen (filename, "rb");
  if (f)
    {
      fclose (f);
      return 1;
    }
  return 0;
}



int
Start_GYM_Dump (void)
{
  char Name[1024], Name2[1024];
  char ext[12] = "_000.gym";
  unsigned char YM_Save[0x200], t_buf[4];
  int num = -1, i, j;

  SetCurrentDirectory (PathNames.Gens_Path);

  if (!Game)
    return 0;

  if (GYM_Dumping)
    {
      Put_Info ("GYM sound is already dumping", 1000);
      return 0;
    }

  strcpy (Name, Dump_GYM_Dir);
  strcat (Name, Rom_Name);

  do
    {
      if (num++ > 99999)
	{
	  Put_Info ("Too much GYM files in your GYM directory", 1000);
	  GYM_File = NULL;
	  return (0);
	}

      ext[0] = '_';
      i = 1;

      j = num / 10000;
      if (j)
	ext[i++] = '0' + j;
      j = (num / 1000) % 10;
      if (j)
	ext[i++] = '0' + j;
      j = (num / 100) % 10;
      ext[i++] = '0' + j;
      j = (num / 10) % 10;
      ext[i++] = '0' + j;
      j = num % 10;
      ext[i++] = '0' + j;
      ext[i++] = '.';
      ext[i++] = 'g';
      ext[i++] = 'y';
      ext[i++] = 'm';
      ext[i] = 0;

      if ((strlen (Name) + strlen (ext)) > 1023)
	return (0);

      strcpy (Name2, Name);
      strcat (Name2, ext);
    }

  while (Exists (Name2));

  GYM_File = fopen (Name2, "w");

  YM2612_Save (YM_Save);

  for (i = 0x30; i < 0x90; i++)
    {
      t_buf[0] = 1;
      t_buf[1] = i;
      t_buf[2] = YM_Save[i];
      fwrite (t_buf, 3, 1, GYM_File);
      t_buf[0] = 2;
      t_buf[1] = i;
      t_buf[2] = YM_Save[i + 0x100];
      fwrite (t_buf, 3, 1, GYM_File);
    }


  for (i = 0xA0; i < 0xB8; i++)
    {
      t_buf[0] = 1;
      t_buf[1] = i;
      t_buf[2] = YM_Save[i];
      fwrite (t_buf, 3, 1, GYM_File);
      t_buf[0] = 2;
      t_buf[1] = i;
      t_buf[2] = YM_Save[i + 0x100];
      fwrite (t_buf, 3, 1, GYM_File);
    }

  t_buf[0] = 1;
  t_buf[1] = 0x22;
  t_buf[2] = YM_Save[0x22];
  fwrite (t_buf, 3, 1, GYM_File);
  t_buf[0] = 1;
  t_buf[1] = 0x27;
  t_buf[2] = YM_Save[0x27];
  fwrite (t_buf, 3, 1, GYM_File);
  t_buf[0] = 1;
  t_buf[1] = 0x28;
  t_buf[2] = YM_Save[0x28];
  fwrite (t_buf, 3, 1, GYM_File);

  Put_Info ("Starting to dump GYM sound", 1000);
  GYM_Dumping = 1;

  return 1;
}


int
Stop_GYM_Dump (void)
{
  if (!GYM_Dumping)
    {
      Put_Info ("Already stopped", 1000);
      return 0;
    }

  if (GYM_File)
    fclose (GYM_File);
  Clear_Sound_Buffer ();

  Put_Info ("GYM dump stopped", 1000);
  GYM_Dumping = 0;

  return 1;
}


/**
 * Start_Play_GYM(): Play a GYM file.
 * @return 1 if a GYM file was loaded.
 */
int Start_Play_GYM(void)
{
	char filename[GENS_PATH_MAX];
	
	if (Game || !(Sound_Enable))
		return 0;
	
	if (GYM_Playing)
	{
		Put_Info ("Already playing GYM", 1000);
		return 0;
	}
	
	End_Sound();
	CPU_Mode = 0;
	
	if (!Init_Sound())
	{
		Sound_Enable = 0;
		Put_Info ("Can't initialise SDL Sound", 1000);
		return 0;
	}
	
	Play_Sound ();
	
	if (UI_OpenFile("Load GYM File", NULL /*Rom_Dir*/, GYMFile, filename) != 0)
		return 0;
	
	// Make sure a filename was actually selected.
	if (strlen(filename) == 0)
		return 0;
	
	// Attempt to open the GYM file.
	GYM_File = fopen (filename, "r");
	if (!GYM_File)
		return 0;
	
	YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	PSG_Init (CLOCK_NTSC / 15, Sound_Rate);
	GYM_Playing = 1;
	
	Put_Info ("Starting to play GYM", 1000);
	return 1;
}


int
Stop_Play_GYM (void)
{

  if (!GYM_Playing)
    {
      Put_Info ("Already stopped", 1000);
      return 0;
    }
  if (GYM_File)
    fclose (GYM_File);
  Clear_Sound_Buffer ();
  GYM_Playing = 0;

  Put_Info ("Stop playing GYM", 1000);

  return 1;
}


int
GYM_Next (void)
{
  unsigned char c, c2;
  unsigned long l;
  int *buf[2];

  buf[0] = Seg_L;
  buf[1] = Seg_R;

  if (!(GYM_Playing) || !(GYM_File))
    return 0;

  do
    {
      l = fread (&c, 1, 1, GYM_File);
      if (l == 0)
	return 0;

      switch (c)
	{
	case 0:
	  PSG_Update (buf, Seg_Length);
	  if (YM2612_Enable)
	    YM2612_Update (buf, Seg_Length);
	  break;

	case 1:
	  fread (&c2, 1, 1, GYM_File);
	  YM2612_Write (0, c2);
	  fread (&c2, 1, 1, GYM_File);
	  YM2612_Write (1, c2);
	  break;

	case 2:
	  fread (&c2, 1, 1, GYM_File);
	  YM2612_Write (2, c2);
	  fread (&c2, 1, 1, GYM_File);
	  YM2612_Write (3, c2);
	  break;

	case 3:
	  fread (&c2, 1, 1, GYM_File);
	  PSG_Write (c2);
	  break;
	}

    }
  while (c);

  return 1;
}


int
Play_GYM (void)
{
  if (!GYM_Next ())
    {
      Stop_Play_GYM ();
      return 0;
    }

  Write_Sound_Buffer (NULL);
  return 1;
}

int
Play_GYM_Bench (void)
{
  if (!GYM_Next ())
    {
      Stop_Play_GYM ();
      return 0;
    }

  Write_Sound_Buffer (NULL);

  return 1;
}


int
Update_GYM_Dump (int v0, int v1, unsigned char v2)
{

  int bResult;
  char buf_tmp[4];
  unsigned long l;

  if (!GYM_Dumping)
    return 0;

  if (GYM_File == NULL)
    return 0;

  v1 &= 0xFF;

  buf_tmp[0] = v0;
  l = 1;

  switch (v0)
    {
    case 1:
    case 2:
      buf_tmp[1] = (unsigned char) v1;
      buf_tmp[2] = v2;
      l = 3;
      break;

    case 3:
      buf_tmp[1] = (unsigned char) v1;
      l = 2;
      break;
    }
  bResult = fwrite (buf_tmp, l, 1, GYM_File);
  if (!bResult)
    return 0;

  return 1;
}
