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
#include "byteswap.h"

char ScrShot_Dir[GENS_PATH_MAX] = "." G_DIR_SEPARATOR_S;

/**
 * Save_Shot(): Save a screenshot.
 * @return 1 on success; 0 on error.
 */
int Save_Shot(void)
{
	FILE *ScrShot_File = 0;
	unsigned char *Dest = NULL;
	int num;
	char filename[GENS_PATH_MAX];
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
	// VDP_Reg.Set4: If 0x01 is set, 320 pixels width; otherwise, 256 pixels width.
	w = (VDP_Reg.Set4 & 0x01 ? 320 : 256);
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
	num = -1;
	do
	{
		num++;
		sprintf(filename, "%s%s_%03d.bmp", ScrShot_Dir, Rom_Name, num);
	}
	while (!stat(filename, &sbuf));
	
	// Attempt to open the file.
	if ((ScrShot_File = fopen(filename, "wb")) == 0)
		return 0;
	
	// Build the bitmap image.
	
	// Bitmap header.
	Dest[0] = 'B';
	Dest[1] = 'M';
	
	cpu_to_le32_ucptr(&Dest[2], bmpSize); // Size of the bitmap.
	cpu_to_le16_ucptr(&Dest[6], 0); // Reserved.
	cpu_to_le16_ucptr(&Dest[8], 0); // Reserved.
	cpu_to_le32_ucptr(&Dest[10], 54); // Bitmap is located 54 bytes from the start of the file.
	cpu_to_le32_ucptr(&Dest[14], 40); // Size of the bitmap header, in bytes. (lol win32)
	cpu_to_le32_ucptr(&Dest[18], w); // Width (pixels)
	cpu_to_le32_ucptr(&Dest[22], h); // Height (pixels)
	cpu_to_le16_ucptr(&Dest[26], 1); // Number of planes. (always 1)
	cpu_to_le16_ucptr(&Dest[28], 24); // bpp (24-bit is the most common.)
	cpu_to_le32_ucptr(&Dest[30], 0); // Compression. (0 == no compression)
	cpu_to_le32_ucptr(&Dest[34], bmpSize); // Size of the bitmap data, in bytes.
	cpu_to_le32_ucptr(&Dest[38], 0x0EC4); // Pixels per meter, X
	cpu_to_le32_ucptr(&Dest[39], 0x0EC4); // Pixels per meter, Y
	cpu_to_le32_ucptr(&Dest[46], 0); // Colors used (0 on non-paletted bitmaps)
	cpu_to_le32_ucptr(&Dest[50], 0); // "Important" colors (0 on non-paletted bitmaps)
	
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
		free(Dest);
		Dest = NULL;
	}
	
	MESSAGE_NUM_L("Screen shot %d saved", "Screen shot %d saved", num, 1500);
	
	return 1;
}
