#include "port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "rom.h"
#include "g_sdlsound.h"
#include "g_main.h"
#include "gens.h"
#include "g_md.h"
#include "g_mcd.h"
#include "g_32x.h"
#include "ggenie.h"
#include "cpu_68k.h"
#include "cd_sys.h"
#include "mem_m68k.h"
#include "mem_sh2.h"
#include "vdp_io.h"
#include "save.h"
#include "ccnet.h"
#include "misc.h"
#include "unzip.h"
#include "chd.h"
#include "wave.h"
#include "gym.h"
#include <assert.h>
#include "support.h"

Rom *My_Rom = NULL;
struct Rom *Game = NULL;
char Rom_Name[512];
char Rom_Dir[1024];
char IPS_Dir[1024];
char Recent_Rom[9][1024];
char US_CD_Bios[1024];
char EU_CD_Bios[1024];
char JA_CD_Bios[1024];
char _32X_Genesis_Bios[1024];
char _32X_Master_Bios[1024];
char _32X_Slave_Bios[1024];
char Genesis_Bios[1024];
char CDROM_DEV[64];
int CDROM_SPEED;


void
Get_Name_From_Path (char *Full_Path, char *Name)
{
  int i = 0;

  i = strlen (Full_Path) - 1;

  while ((i >= 0) && (Full_Path[i] != G_DIR_SEPARATOR))
    i--;

  if (i <= 0)
    {
      Name[0] = 0;
    }
  else
    {
      strcpy (Name, &Full_Path[++i]);
    }
}


void
Get_Dir_From_Path (char *Full_Path, char *Dir)
{
  int i = 0;

  i = strlen (Full_Path) - 1;

  while ((i >= 0) && (Full_Path[i] != G_DIR_SEPARATOR))
    i--;

  if (i <= 0)
    {
      Dir[0] = 0;
    }
  else
    {
      strncpy (Dir, Full_Path, ++i);
      Dir[i] = 0;
    }
}


void
Update_Recent_Rom (char *Path)
{
  int i;

  for (i = 0; i < 9; i++)
    if (!(strcmp (Recent_Rom[i], Path)))
      return;

  for (i = 8; i > 0; i--)
    strcpy (Recent_Rom[i], Recent_Rom[i - 1]);

  strcpy (Recent_Rom[0], Path);
}


void
Update_Rom_Dir (char *Path)
{
  Get_Dir_From_Path (Path, Rom_Dir);
}


void
Update_Rom_Name (char *Name)
{
  int i, leng;

  leng = strlen (Name) - 1;

  while ((leng >= 0) && (Name[leng] != G_DIR_SEPARATOR))
    leng--;

  leng++;
  i = 0;

  while ((Name[leng]) && (Name[leng] != '.'))
    Rom_Name[i++] = Name[leng++];

  Rom_Name[i] = 0;
}


void
Update_CD_Rom_Name (char *Name)
{
  int i, j;

  memcpy (Rom_Name, Name, 48);

  for (i = 0; i < 48; i++)
    {
      if ((Rom_Name[i] >= '0') && (Rom_Name[i] <= '9'))
	continue;
      if (Rom_Name[i] == ' ')
	continue;
      if ((Rom_Name[i] >= 'A') && (Rom_Name[i] <= 'Z'))
	continue;
      if ((Rom_Name[i] >= 'a') && (Rom_Name[i] <= 'z'))
	continue;
      Rom_Name[i] = ' ';
    }

  for (i = 0; i < 48; i++)
    {
      if (Rom_Name[i] != ' ')
	i = 100;
    }

  if (i < 100)
    strcpy (Rom_Name, "no name");

  for (i = 47, j = 48; i >= 0; i--, j--)
    {
      if (Rom_Name[i] != ' ')
	i = -1;
    }

  Rom_Name[j + 1] = 0;
}


int
Detect_Format (char *Name)
{
  FILE *f;
  unzFile zf;
  unz_file_info zinf;
  struct chd *cf;
  int i;
  unsigned char buf[1024];
  char zname[256];

  // SetCurrentDirectory (Gens_Path);

  memset (buf, 0, 1024);

  if ((!strcasecmp ("zip", &Name[strlen (Name) - 3]))
      || (!strcasecmp ("zsg", &Name[strlen (Name) - 3])))
    {
      zf = unzOpen (Name);

      if (!zf)
	return -1;

      i = unzGoToFirstFile (zf);

      while (i == UNZ_OK)
	{
	  unzGetCurrentFileInfo (zf, &zinf, zname, 128, NULL, 0, NULL, 0);

	  if ((!strncasecmp ("smd", &zname[strlen (zname) - 3], 3))
	      ||
	      (!strncasecmp ("bin", &zname[strlen (zname) - 3], 3))
	      ||
	      (!strncasecmp ("gen", &zname[strlen (zname) - 3], 3))
	      ||
	      (!strncasecmp ("32x", &zname[strlen (zname) - 3], 3))
	      || (!strncasecmp ("iso", &zname[strlen (zname) - 3], 3)))
	    {
	      i = 0;
	      break;
	    }

	  i = unzGoToNextFile (zf);
	}

      if (i)
	return 0;
      if (unzLocateFile (zf, zname, 1) != UNZ_OK)
	return 0;
      if (unzOpenCurrentFile (zf) != UNZ_OK)
	return 0;

      unzReadCurrentFile (zf, buf, 1024);

      unzCloseCurrentFile (zf);
      unzClose (zf);
    }
  else if (!strcasecmp (".gz", &Name[strlen (Name) - 3]))
    {
      f = (FILE *) gzopen (Name, "rb");
      if (f == NULL)
	return -1;
      gzread (f, buf, 1024);
      gzclose (f);
    }
  else if (!strcasecmp (".chd", &Name[strlen (Name) -4]))
    {
      //char *p;
      //int n;

      cf = chd_open(Name, NULL);
      if (cf == NULL)
	return -1;

      chd_read_range(cf, buf, 0, 1024);

      chd_close(cf);
    }
  else
    {
      strcpy (zname, Name);

      f = fopen (zname, "rb");

      if (f == NULL)
	return -1;

      fread (buf, 1, 1024, f);

      fclose (f);
    }

  if (!strncasecmp ("SEGADISCSYSTEM", &buf[0x00], 14))
    return SEGACD_IMAGE;	// Sega CD (ISO)
  if (!strncasecmp ("SEGADISCSYSTEM", &buf[0x10], 14))
    return SEGACD_IMAGE + 1;	// Sega CD (BIN)

  i = 0;

  if (strncasecmp ("SEGA", &buf[0x100], 4))
    {
      // Maybe interleaved

      if (!strncasecmp ("EA", &buf[0x200 + (0x100 / 2)], 2))
	i = 1;
      if ((buf[0x08] == 0xAA) && (buf[0x09] == 0xBB) && (buf[0x0A] == 0x06))
	i = 1;
    }

  if (i)			// interleaved
    {
      if ((!strncasecmp ("32X", &zname[strlen (zname) - 3], 3))
	  && (buf[0x200 / 2] == 0x4E))
	return _32X_ROM + 1;
      if (!strncasecmp ("3X", &buf[0x200 + (0x105 / 2)], 2))
	return _32X_ROM + 1;
    }
  else
    {
      if ((!strncasecmp ("32X", &zname[strlen (zname) - 3], 3))
	  && (buf[0x200] == 0x4E))
	return _32X_ROM;
      if (!strncasecmp ("32X", &buf[0x105], 3))
	return _32X_ROM;
    }

  return GENESIS_ROM + i;
}


void
De_Interleave (void)
{
  unsigned char buf[16384];
  unsigned char *Src;
  int i, j, Nb_Blocks, ptr;

  Src = &Rom_Data[0x200];

  Rom_Size -= 512;

  Nb_Blocks = Rom_Size / 16384;

  for (ptr = 0, i = 0; i < Nb_Blocks; i++, ptr += 16384)
    {
      memcpy (buf, &Src[ptr], 16384);

      for (j = 0; j < 8192; j++)
	{
	  Rom_Data[ptr + (j << 1) + 1] = buf[j];
	  Rom_Data[ptr + (j << 1)] = buf[j + 8192];
	}
    }
}


void
Fill_Infos (void)
{
  int i;

  // Finally we do the IPS patch here, we can have the translated game name

  IPS_Patching ();

  for (i = 0; i < 16; i++)
    My_Rom->Console_Name[i] = Rom_Data[i + 256];

  for (i = 0; i < 16; i++)
    My_Rom->Copyright[i] = Rom_Data[i + 272];

  for (i = 0; i < 48; i++)
    My_Rom->Rom_Name[i] = Rom_Data[i + 288];

  for (i = 0; i < 48; i++)
    My_Rom->Rom_Name_W[i] = Rom_Data[i + 336];

  My_Rom->Type[0] = Rom_Data[384];
  My_Rom->Type[1] = Rom_Data[385];

  for (i = 0; i < 12; i++)
    My_Rom->Version[i] = Rom_Data[i + 386];

  My_Rom->Checksum = (Rom_Data[398] << 8) | Rom_Data[399];

  for (i = 0; i < 16; i++)
    My_Rom->IO_Support[i] = Rom_Data[i + 400];

  My_Rom->Rom_Start_Adress = Rom_Data[416] << 24;
  My_Rom->Rom_Start_Adress |= Rom_Data[417] << 16;
  My_Rom->Rom_Start_Adress |= Rom_Data[418] << 8;
  My_Rom->Rom_Start_Adress |= Rom_Data[419];

  My_Rom->Rom_End_Adress = Rom_Data[420] << 24;
  My_Rom->Rom_End_Adress |= Rom_Data[421] << 16;
  My_Rom->Rom_End_Adress |= Rom_Data[422] << 8;
  My_Rom->Rom_End_Adress |= Rom_Data[423];

  My_Rom->R_Size = My_Rom->Rom_End_Adress - My_Rom->Rom_Start_Adress + 1;

  for (i = 0; i < 12; i++)
    My_Rom->Ram_Infos[i] = Rom_Data[i + 424];

  My_Rom->Ram_Start_Adress = Rom_Data[436] << 24;
  My_Rom->Ram_Start_Adress |= Rom_Data[437] << 16;
  My_Rom->Ram_Start_Adress |= Rom_Data[438] << 8;
  My_Rom->Ram_Start_Adress |= Rom_Data[439];

  My_Rom->Ram_End_Adress = Rom_Data[440] << 24;
  My_Rom->Ram_End_Adress |= Rom_Data[441] << 16;
  My_Rom->Ram_End_Adress |= Rom_Data[442] << 8;
  My_Rom->Ram_End_Adress |= Rom_Data[443];

  for (i = 0; i < 12; i++)
    My_Rom->Modem_Infos[i] = Rom_Data[i + 444];

  for (i = 0; i < 40; i++)
    My_Rom->Description[i] = Rom_Data[i + 456];

  for (i = 0; i < 3; i++)
    My_Rom->Countries[i] = Rom_Data[i + 496];

  My_Rom->Console_Name[16] = 0;
  My_Rom->Copyright[16] = 0;
  My_Rom->Rom_Name[48] = 0;
  My_Rom->Rom_Name_W[48] = 0;
  My_Rom->Type[2] = 0;
  My_Rom->Version[12] = 0;
  My_Rom->IO_Support[12] = 0;
  My_Rom->Ram_Infos[12] = 0;
  My_Rom->Modem_Infos[12] = 0;
  My_Rom->Description[40] = 0;
  My_Rom->Countries[3] = 0;
}


int
Get_Rom ()
{
  GtkWidget *widget;
  gint res;
  char Name[2048];

  widget = create_file_chooser_dialog ("Open Rom", GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(widget), Rom_Dir);
  addRomsFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      gchar *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
     strncpy (Name, filename, 2048);
      g_free (filename);
      
    }
      else
    {
      strcpy (Name, "\n");
    }
    
  gtk_widget_destroy (widget);
  if (res == GTK_RESPONSE_CANCEL)
  	return 0;
  return Open_Rom (Name);
}

int
Open_Rom (char *Name)
{
  int sys;

  Free_Rom (Game);
  sys = Detect_Format (Name);

  if (sys < 1)
    return -1;

  Update_Recent_Rom (Name);
  Update_Rom_Dir (Name);

  if ((sys >> 1) < 3)		// Have to load a rom
    {
      if ((!strcasecmp ("zip", &Name[strlen (Name) - 3]))
	  || (!strcasecmp ("zsg", &Name[strlen (Name) - 3])))
	{
	  Game = Load_Rom_Zipped (Name, sys & 1);
	}
      else if (!strcasecmp ("gz", &Name[strlen (Name) - 2]))
	Game = Load_Rom_Gz (Name, sys & 1);
      else
	{
	  Game = Load_Rom (Name, sys & 1);
	}
    }

  switch (sys >> 1)
    {
    default:
    case 1:			// Genesis rom
      if (Game)
	Genesis_Started = Init_Genesis (Game);

      return Genesis_Started;
      break;

    case 2:			// 32X rom
      if (Game)
	_32X_Started = Init_32X (Game);

      return _32X_Started;
      break;

    case 3:			// Sega CD image
      SegaCD_Started = Init_SegaCD (Name);

      return SegaCD_Started;
      break;

    case 4:			// Sega CD 32X image
      break;
    }

  return -1;
  
}


// Rom is already in buffer, we just need to fill rom structure
// and do init stuff...
int
Load_Rom_CC (char *Name, int Size)
{
  My_Rom = (Rom *) malloc (sizeof (Rom));

  if (!My_Rom)
    {
      Game = NULL;
      return 0;
    }

  Update_Rom_Name (Name);
  Rom_Size = Size;
  Fill_Infos ();

  Game = My_Rom;

  Genesis_Started = Init_Genesis (Game);
  return Genesis_Started;
}


Rom *
Load_Bios (char *Name)
{
  FILE *Rom_File;
  //SetCurrentDirectory (Gens_Path);
  if ((Rom_File = fopen (Name, "rb")) == 0)
    return 0;
  fclose (Rom_File);
  Free_Rom (Game);

  if (!strcasecmp ("ZIP", &Name[strlen (Name) - 3]))
    return (Game = Load_Rom_Zipped (Name, 0));
  else if (!strcasecmp ("gz", &Name[strlen (Name) - 2]))
    return Game = Load_Rom_Gz (Name, 0);
  else
    return (Game = Load_Rom (Name, 0));
}


Rom *
Load_Rom (char *Name, int inter)
{

  FILE *Rom_File;
  int Size = 0;

  //SetCurrentDirectory (Gens_Path);

  if ((Rom_File = fopen (Name, "rb")) == 0)
    {
      Game = NULL;
      return (NULL);
    }

  fseek (Rom_File, 0, SEEK_END);
  Size = ftell (Rom_File);
  fseek (Rom_File, 0, SEEK_SET);

  if ((Size) > ((6 * 1024 * 1024) + 512))
    {
      Game = NULL;
      fclose (Rom_File);
      return (NULL);
    }
  My_Rom = (Rom *) malloc (sizeof (Rom));

  if (!My_Rom)
    {
      Game = NULL;
      fclose (Rom_File);
      return (NULL);
    }

  memset (Rom_Data, 0, 6 * 1024 * 1024);
  fread (Rom_Data, Size, 1, Rom_File);
  fclose (Rom_File);
  Update_Rom_Name (Name);

  Rom_Size = Size;
  if (inter)
    De_Interleave ();
  Fill_Infos ();

  return My_Rom;
}

Rom *
Load_Rom_Gz (char *Name, int inter)
{
  FILE *Rom_File;
  int Size = 0;
  char *read_buf[1024];
  //SetCurrentDirectory (Gens_Path);
  if ((Rom_File = (FILE *) gzopen (Name, "rb")) == 0)
    {
	  printf("No genesis or 32X roms file found in gzip");
      open_msgbox ("No genesis or 32X roms file found in gzip");
      Game = NULL;
      return (NULL);
    }

  while (gzeof (Rom_File) == 0)
    {
      Size += gzread (Rom_File, read_buf, 1024);
    }
  gzrewind (Rom_File);
  if ((Size) > ((6 * 1024 * 1024) + 512))
    {
      Game = NULL;
      gzclose (Rom_File);
      return (NULL);
    }
  My_Rom = (Rom *) malloc (sizeof (Rom));
  if (!My_Rom)
    {
      Game = NULL;
      gzclose (Rom_File);
      return (NULL);
    }

  memset (Rom_Data, 0, 6 * 1024 * 1024);
  gzread (Rom_File, Rom_Data, Size);
  gzclose (Rom_File);
  Update_Rom_Name (Name);
  Rom_Size = Size;
  if (inter)
    De_Interleave ();
  Fill_Infos ();
  return My_Rom;
}


Rom *
Load_Rom_Zipped (char *Name, int inter)
{
  int Size = 0;
  int bResult;
  int Current;
  char Tmp[256];
  char File_Name[132];
  unz_file_info Infos;
  unzFile Rom_File;

  //SetCurrentDirectory (Gens_Path);

  Rom_File = unzOpen (Name);

  if (!Rom_File)
    {
      open_msgbox ("No genesis or 32X roms file found in zip");

      Game = NULL;
      return (NULL);
    }

  Current = unzGoToFirstFile (Rom_File);

  while (Current == UNZ_OK)
    {
      unzGetCurrentFileInfo (Rom_File, &Infos, File_Name, 128, NULL,
			     0, NULL, 0);

      if ((!strncasecmp
	   (".smd", File_Name + strlen (File_Name) - 4, 4))
	  ||
	  (!strncasecmp
	   (".bin", File_Name + strlen (File_Name) - 4, 4))
	  ||
	  (!strncasecmp
	   (".gen", File_Name + strlen (File_Name) - 4, 4))
	  || (!strncasecmp (".32x", File_Name + strlen (File_Name) - 4, 4)))
	{
	  Size = Infos.uncompressed_size;
	  break;
	}

      Current = unzGoToNextFile (Rom_File);
    }

  if ((Current != UNZ_END_OF_LIST_OF_FILE && Current != UNZ_OK) || !Size)
    {

      open_msgbox ("No genesis or 32X roms file found in zip");

      unzClose (Rom_File);
      Game = NULL;
      return (NULL);
    }

  if (Size > ((6 * 1024 * 1024) + 512))
    {
      unzClose (Rom_File);
      Game = NULL;
      return (NULL);
    }

  if (unzLocateFile (Rom_File, File_Name, 1) != UNZ_OK)
    {
      unzClose (Rom_File);
      Game = NULL;
      return NULL;
    }

  if (unzOpenCurrentFile (Rom_File) != UNZ_OK)
    {
      unzClose (Rom_File);
      Game = NULL;
      return NULL;
    }

  My_Rom = (Rom *) malloc (sizeof (Rom));

  if (!My_Rom)
    {
      unzClose (Rom_File);
      Game = NULL;
      return (NULL);
    }

  memset (Rom_Data, 0, 6 * 1024 * 1024);
  bResult = unzReadCurrentFile (Rom_File, Rom_Data, Size);

  unzCloseCurrentFile (Rom_File);

  if ((bResult <= 0) || (bResult != Size))
    {
      sprintf (Tmp, "Error in zip file : \n");

      switch (bResult)
	{
	case UNZ_ERRNO:
	  strcat (Tmp, "Unknow...");
	  break;

	case UNZ_EOF:
	  strcat (Tmp, "Unexpected end of file.");
	  break;

	case UNZ_PARAMERROR:
	  strcat (Tmp, "Parameter error.");
	  break;

	case UNZ_BADZIPFILE:
	  strcat (Tmp, "Wrong zip file.");
	  break;

	case UNZ_INTERNALERROR:
	  strcat (Tmp, "Internal error.");
	  break;

	case UNZ_CRCERROR:
	  strcat (Tmp, "CRC error.");
	  break;
	}
      open_msgbox (Tmp);
      unzClose (Rom_File);
      Game = NULL;
      return (NULL);
    }

  unzClose (Rom_File);

  Update_Rom_Name (File_Name);

  Rom_Size = Size;

  if (inter)
    De_Interleave ();

  Fill_Infos ();

  return My_Rom;
}


unsigned short
Calculate_Checksum (void)
{
  unsigned short checksum = 0;
  unsigned int i;

  if (!Game)
    return (0);

  for (i = 512; i < Rom_Size; i += 2)
    {
      checksum += (unsigned short) (Rom_Data[i + 0]);
      checksum += (unsigned short) (Rom_Data[i + 1] << 8);
    }

  return checksum;
}


void
Fix_Checksum (void)
{
  unsigned short checks;

  if (!Game)
    return;

  checks = Calculate_Checksum ();

  if (Rom_Size)
    {
      Rom_Data[0x18E] = checks & 0xFF;
      Rom_Data[0x18F] = checks >> 8;
      _32X_Rom[0x18E] = checks >> 8;;
      _32X_Rom[0x18F] = checks & 0xFF;
    }
}


unsigned int
Calculate_CRC32 (void)
{
  unsigned int crc = 0;

  Byte_Swap (Rom_Data, Rom_Size);
  crc = crc32 (0, Rom_Data, Rom_Size);
  Byte_Swap (Rom_Data, Rom_Size);

  return crc;
}


int
IPS_Patching (void)
{
  FILE *IPS_File;
  char Name[1024];
  unsigned char buf[16];
  unsigned int adr, len, i;

  //SetCurrentDirectory (Gens_Path);

  strcpy (Name, IPS_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, ".ips");

  IPS_File = fopen (Name, "rb");

  if (IPS_File == NULL)
    return 1;

  fseek (IPS_File, 0, SEEK_SET);

  fread (buf, 1, 5, IPS_File);
  buf[5] = 0;

  if (strcasecmp ((char *) buf, "patch"))
    {
      fclose (IPS_File);
      return 2;
    }

  fread (buf, 1, 3, IPS_File);
  buf[3] = 0;

  while (strcasecmp ((char *) buf, "eof"))
    {
      adr = (unsigned int) buf[2];
      adr += (unsigned int) (buf[1] << 8);
      adr += (unsigned int) (buf[0] << 16);

      if (fread (buf, 1, 2, IPS_File) == 0)
	{
	  fclose (IPS_File);
	  return 3;
	}

      len = (unsigned int) buf[1];
      len += (unsigned int) (buf[0] << 8);

      for (i = 0; i < len; i++)
	{
	  if (fread (buf, 1, 1, IPS_File) == 0)
	    {
	      fclose (IPS_File);
	      return 3;
	    }

	  if (adr < Rom_Size)
	    Rom_Data[adr++] = buf[0];
	}

      if (fread (buf, 1, 3, IPS_File) == 0)
	{
	  fclose (IPS_File);
	  return 3;
	}

      buf[3] = 0;
    }

  fclose (IPS_File);

  return 0;
}


void
Free_Rom (Rom * Rom_MD)
{
  if (Game == NULL)
    return;

#ifdef CC_SUPPORT
  CC_Close ();
#endif

  if (SegaCD_Started)
    Save_BRAM ();
  Save_SRAM ();
  Save_Patch_File ();
  if (WAV_Dumping)
    Stop_WAV_Dump ();
  if (GYM_Dumping)
    Stop_GYM_Dump ();
  if (SegaCD_Started)
    Stop_CD ();
  Net_Play = 0;
  Genesis_Started = 0;
  _32X_Started = 0;
  SegaCD_Started = 0;
  Game = NULL;

  if (Rom_MD)
    {
      free (Rom_MD);
      Rom_MD = NULL;
    }

  if (Intro_Style == 3)
    Init_Genesis_Bios ();

  SetWindowText ("Gens - Idle");
}
