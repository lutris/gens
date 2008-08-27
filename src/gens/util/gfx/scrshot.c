#include "scrshot.h"
#include "port.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "rom.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "g_sdldraw.h"

char ScrShot_Dir[GENS_PATH_MAX] = "." G_DIR_SEPARATOR_S;

/**
 * Save_Shot(): Save a screenshot.
 * @return 1 on success; 0 on error.
 */
int Save_Shot(void)
{
	FILE *ScrShot_File = 0;
	unsigned char *Dest = NULL;
	int i, j, num = -1, stated;
	char filename[GENS_PATH_MAX], ext[16];
	struct stat sbuf;
	
	// Bitmap dimensions.
	int w, h;
	int x, y, bmpSize;
	
	// Used for converting the MD frame to standard bitmap format.
	int pos;
	unsigned short MD_Color;
	
	// If no game is running, don't do anything.
	if (!Game)
		return 0;
	
	// Variables used:
	// VDP_Num_Vis_Lines: Number of lines visible on the screen. (bitmap height)
	// MD_Screen: MD screen buffer.
	// TODO: Find out where 256x224 mode is set.
	w = 320;
	h = VDP_Num_Vis_Lines;
	
	// Calculate the size of the bitmap image.
	bmpSize = (w * h * 3) + 54;
	if ((Dest = (unsigned char*)malloc(bmpSize)) == NULL)
	{
		// Could not allocate enough memory.
		return 0;
	}
	
	// Clear the bitmap memory.
	memset(Dest, 0, bmpSize);
	
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
	
	if ((ScrShot_File = fopen(filename, "wb")) == 0)
		return 0;
	
	// Build the bitmap image.
	Dest[0] = 'B';
	Dest[1] = 'M';
	
	Dest[2] = (unsigned char) ((bmpSize >> 0) & 0xFF);
	Dest[3] = (unsigned char) ((bmpSize >> 8) & 0xFF);
	Dest[4] = (unsigned char) ((bmpSize >> 16) & 0xFF);
	Dest[5] = (unsigned char) ((bmpSize >> 24) & 0xFF);
	
	Dest[6] = Dest[7] = Dest[8] = Dest[9] = 0;
	
	Dest[10] = 54;
	Dest[11] = Dest[12] = Dest[13] = 0;
	
	Dest[14] = 40;
	Dest[15] = Dest[16] = Dest[17] = 0;
	
	Dest[18] = (unsigned char) ((w >> 0) & 0xFF);
	Dest[19] = (unsigned char) ((w >> 8) & 0xFF);
	Dest[20] = (unsigned char) ((w >> 16) & 0xFF);
	Dest[21] = (unsigned char) ((w >> 24) & 0xFF);
	
	Dest[22] = (unsigned char) ((h >> 0) & 0xFF);
	Dest[23] = (unsigned char) ((h >> 8) & 0xFF);
	Dest[24] = (unsigned char) ((h >> 16) & 0xFF);
	Dest[25] = (unsigned char) ((h >> 24) & 0xFF);
	
	Dest[26] = 1;
	Dest[27] = 0;
	
	Dest[28] = 24;
	Dest[29] = 0;
	
	Dest[30] = Dest[31] = Dest[32] = Dest[33] = 0;
	
	Dest[34] = (unsigned char) ((bmpSize >> 0) & 0xFF);
	Dest[35] = (unsigned char) ((bmpSize >> 8) & 0xFF);
	Dest[36] = (unsigned char) ((bmpSize >> 16) & 0xFF);
	Dest[37] = (unsigned char) ((bmpSize >> 24) & 0xFF);
	
	Dest[38] = Dest[42] = 0xC4;
	Dest[39] = Dest[43] = 0x0E;
	Dest[40] = Dest[44] = Dest[41] = Dest[45] = 0;
	
	Dest[46] = Dest[47] = Dest[48] = Dest[49] = 0;
	Dest[50] = Dest[51] = Dest[52] = Dest[53] = 0;
	
	// Start/end coordinates in the MD_Screen buffer.
	// 320x224: start = 8, end = 328
	// 256x224: start = 8, end = 262 (TODO: Determine when 256x224 mode is active.)
	
	//Src += Pitch * (Y - 1);
	pos = 0;
	// Bitmaps are stored upside-down.
	if (!Mode_555)
	{
		// 16-bit color, 565 pixel format.
		for (y = h - 1; y >= 0; y--)
		{
			for (x = 8; x < 8 + w; x++)
			{
				MD_Color = MD_Screen[(y * 336) + x];
				Dest[54 + (pos * 3) + 2] = (unsigned char)((MD_Color & 0xF800) >> 8);
				Dest[54 + (pos * 3) + 1] = (unsigned char)((MD_Color & 0x07E0) >> 3);
				Dest[54 + (pos * 3) + 0] = (unsigned char)((MD_Color & 0x001F) << 3);
				pos++;
			}
		}
	}
	else
	{
		// 16-bit color, 555 pixel format.
		for (y = h - 1; y >= 0; y--)
		{
			for (x = 8; x < 8 + w; x++)
			{
				MD_Color = MD_Screen[(y * 336) + x];
				Dest[54 + (pos * 3) + 2] = (unsigned char)((MD_Color & 0x7C00) >> 7);
				Dest[54 + (pos * 3) + 1] = (unsigned char)((MD_Color & 0x03E0) >> 2);
				Dest[54 + (pos * 3) + 0] = (unsigned char)((MD_Color & 0x001F) << 3);
				pos++;
			}
		}
	}
	
	fwrite(Dest, 1, bmpSize + 54, ScrShot_File);
	fclose(ScrShot_File);
	
	if (Dest)
	{
		free (Dest);
		Dest = NULL;
	}
	
	MESSAGE_NUM_L("Screen shot %d saved", "Screen shot %d saved", num, 1500);
	
	return 1;
}
