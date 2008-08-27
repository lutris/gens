#include "port.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "gens.h"
#include "g_main.h"
#include "g_sdldraw.h"
#include "rom.h"

char ScrShot_Dir[GENS_PATH_MAX] = "." G_DIR_SEPARATOR_S;

/**
 * Save_Shot(): Save a screenshot.
 * @param Screen Screen to save.
 * @param mode
 * @param X
 * @param Y
 * @param Pitch
 * @return 1 on success; 0 on error.
 */
int Save_Shot(unsigned char *Screen, int mode, int X, int Y, int Pitch)
{
	FILE *ScrShot_File = 0;
	unsigned char *Src = NULL, *Dest = NULL;
	int i, j, tmp, offs, num = -1, stated;
	char filename[GENS_PATH_MAX], ext[16];
	struct stat sbuf;
	
	// Set the current directory to the Gens directory.
	// TODO: Use the screenshots directory instead?
	// TODO: Is this even needed?
	SetCurrentDirectory(Settings.PathNames.Gens_Path);
	
	// If no game is running, don't do anything.
	if (!Game)
		return 0;
	
	// Calculate the size of the bitmap image.
	i = (X * Y * 3) + 54;
	if ((Dest = (unsigned char *) malloc (i)) == NULL)
	{
		// Could not allocate enough memory.
		return 0;
	}
	
	// Clear the bitmap memory.
	memset(Dest, 0, i);
	
	// Build the filename.
	do
	{
		if (num++ > 99999)
		{
			free (Dest);
			return 0;
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
		ext[i++] = 'b';
		ext[i++] = 'm';
		ext[i++] = 'p';
		ext[i] = 0;
		
		strcpy (filename, ScrShot_Dir);
		strcat (filename, Rom_Name);
		strcat (filename, ext);
		stated = stat(filename, &sbuf);
	}
	while (stated == 0);
	
	i = (X * Y * 3) + 54;
	
	if ((ScrShot_File = fopen(filename, "wb")) == 0)
		return 0;
	
	// Build the bitmap image.
	Dest[0] = 'B';
	Dest[1] = 'M';
	
	Dest[2] = (unsigned char) ((i >> 0) & 0xFF);
	Dest[3] = (unsigned char) ((i >> 8) & 0xFF);
	Dest[4] = (unsigned char) ((i >> 16) & 0xFF);
	Dest[5] = (unsigned char) ((i >> 24) & 0xFF);
	
	Dest[6] = Dest[7] = Dest[8] = Dest[9] = 0;
	
	Dest[10] = 54;
	Dest[11] = Dest[12] = Dest[13] = 0;
	
	Dest[14] = 40;
	Dest[15] = Dest[16] = Dest[17] = 0;
	
	Dest[18] = (unsigned char) ((X >> 0) & 0xFF);
	Dest[19] = (unsigned char) ((X >> 8) & 0xFF);
	Dest[20] = (unsigned char) ((X >> 16) & 0xFF);
	Dest[21] = (unsigned char) ((X >> 24) & 0xFF);
	
	Dest[22] = (unsigned char) ((Y >> 0) & 0xFF);
	Dest[23] = (unsigned char) ((Y >> 8) & 0xFF);
	Dest[24] = (unsigned char) ((Y >> 16) & 0xFF);
	Dest[25] = (unsigned char) ((Y >> 24) & 0xFF);
	
	Dest[26] = 1;
	Dest[27] = 0;
	
	Dest[28] = 24;
	Dest[29] = 0;
	
	Dest[30] = Dest[31] = Dest[32] = Dest[33] = 0;
	
	i -= 54;
	
	Dest[34] = (unsigned char) ((i >> 0) & 0xFF);
	Dest[35] = (unsigned char) ((i >> 8) & 0xFF);
	Dest[36] = (unsigned char) ((i >> 16) & 0xFF);
	Dest[37] = (unsigned char) ((i >> 24) & 0xFF);
	
	Dest[38] = Dest[42] = 0xC4;
	Dest[39] = Dest[43] = 0x0E;
	Dest[40] = Dest[44] = Dest[41] = Dest[45] = 0;
	
	Dest[46] = Dest[47] = Dest[48] = Dest[49] = 0;
	Dest[50] = Dest[51] = Dest[52] = Dest[53] = 0;
	
	Src = (unsigned char *) (Screen);
	Src += Pitch * (Y - 1);
	
	if (mode)
	{
		for (offs = 54, j = 0; j < Y; j++, Src -= Pitch, offs += (3 * X))
		{
			for (i = 0; i < X; i++)
			{
				tmp = (unsigned int) (Src[2 * i + 0] + (Src[2 * i + 1] << 8));
				Dest[offs + (3 * i) + 2] = ((tmp >> 7) & 0xF8);
				Dest[offs + (3 * i) + 1] = ((tmp >> 2) & 0xF8);
				Dest[offs + (3 * i) + 0] = ((tmp << 3) & 0xF8);
			}
		}
	}
	else
	{
		for (offs = 54, j = 0; j < Y; j++, Src -= Pitch, offs += (3 * X))
		{
			for (i = 0; i < X; i++)
			{
				tmp = (unsigned int) (Src[2 * i + 0] + (Src[2 * i + 1] << 8));
				Dest[offs + (3 * i) + 2] = ((tmp >> 8) & 0xF8);
				Dest[offs + (3 * i) + 1] = ((tmp >> 3) & 0xFC);
				Dest[offs + (3 * i) + 0] = ((tmp << 3) & 0xF8);
			}
		}
	}
	fwrite(Dest, 1, (X * Y * 3) + 54, ScrShot_File);
	fclose(ScrShot_File);
	
	if (Dest)
	{
		free (Dest);
		Dest = NULL;
	}
	
	MESSAGE_NUM_L("Screen shot %d saved", "Screen shot %d saved", num, 1500);
	
	return 1;
}
