#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imageutil.hpp"
#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/byteswap.h"

#include "macros/file_m.h"

#ifdef GENS_PNG
#include <png.h>
#endif /* GENS_PNG */


/**
 * writeBMP(): Write a BMP image.
 * @param fImg File handle to save the image to.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @return 1 on success; 0 on error.
 */
int ImageUtil::writeBMP(FILE *fImg, const int w, const int h, const int pitch,
			const void *screen, const int bpp)
{
	if (!fImg || !screen || (w <= 0 || h <= 0 || pitch <= 0))
		return 0;
	
	unsigned char *bmpData = NULL;
	
	// Calculate the size of the bitmap image.
	int bmpSize = (w * h * 3) + 54;
	bmpData = static_cast<unsigned char*>(malloc(bmpSize));
	if (!bmpData)
	{
		// Could not allocate enough memory.
		fprintf(stderr, "writeBMP): Could not allocate enough memory for the bitmap data.\n");
		return 0;
	}
	
	// Build the bitmap image.
	
	// Bitmap header.
	bmpData[0] = 'B';
	bmpData[1] = 'M';
	
	cpu_to_le32_ucptr(&bmpData[2], bmpSize); // Size of the bitmap.
	cpu_to_le16_ucptr(&bmpData[6], 0); // Reserved.
	cpu_to_le16_ucptr(&bmpData[8], 0); // Reserved.
	cpu_to_le32_ucptr(&bmpData[10], 54); // Bitmap is located 54 bytes from the start of the file.
	cpu_to_le32_ucptr(&bmpData[14], 40); // Size of the bitmap header, in bytes. (lol win32)
	cpu_to_le32_ucptr(&bmpData[18], w); // Width (pixels)
	cpu_to_le32_ucptr(&bmpData[22], h); // Height (pixels)
	cpu_to_le16_ucptr(&bmpData[26], 1); // Number of planes. (always 1)
	cpu_to_le16_ucptr(&bmpData[28], 24); // bpp (24-bit is the most common.)
	cpu_to_le32_ucptr(&bmpData[30], 0); // Compression. (0 == no compression)
	cpu_to_le32_ucptr(&bmpData[34], bmpSize); // Size of the bitmap data, in bytes.
	cpu_to_le32_ucptr(&bmpData[38], 0x0EC4); // Pixels per meter, X
	cpu_to_le32_ucptr(&bmpData[39], 0x0EC4); // Pixels per meter, Y
	cpu_to_le32_ucptr(&bmpData[46], 0); // Colors used (0 on non-paletted bitmaps)
	cpu_to_le32_ucptr(&bmpData[50], 0); // "Important" colors (0 on non-paletted bitmaps)
	
	// TODO: Verify endianness requirements.
	
	//Src += Pitch * (Y - 1);
	int pos = 0;
	int x, y;
	
	// Bitmaps are stored upside-down.
	if (bpp == 15 || bpp == 16)
	{
		// 15-bit or 16-bit color.
		uint16_t MD_Color;
		const uint16_t *screen16 = static_cast<const uint16_t*>(screen);
		
		if (bpp == 15)
		{
			// 15-bit color, 555 pixel format.
			for (y = h - 1; y >= 0; y--)
			{
				for (x = 0; x < w; x++)
				{
					MD_Color = screen16[(y * pitch) + x];
					bmpData[54 + (pos * 3) + 2] = (uint8_t)((MD_Color & 0x7C00) >> 7);
					bmpData[54 + (pos * 3) + 1] = (uint8_t)((MD_Color & 0x03E0) >> 2);
					bmpData[54 + (pos * 3) + 0] = (uint8_t)((MD_Color & 0x001F) << 3);
					pos++;
				}
			}
		}
		else if (bpp == 16)
		{
			// 16-bit color, 565 pixel format.
			for (y = h - 1; y >= 0; y--)
			{
				for (x = 0; x < w; x++)
				{
					MD_Color = screen16[(y * pitch) + x];
					bmpData[54 + (pos * 3) + 2] = (uint8_t)((MD_Color & 0xF800) >> 8);
					bmpData[54 + (pos * 3) + 1] = (uint8_t)((MD_Color & 0x07E0) >> 3);
					bmpData[54 + (pos * 3) + 0] = (uint8_t)((MD_Color & 0x001F) << 3);
					pos++;
				}
			}
		}
	}
	else //if (bpp == 32)
	{
		// 32-bit color.
		// BMP uses 24-bit color, so a conversion is still necessary.
		uint32_t MD_Color32;
		const uint32_t *screen32 = static_cast<const uint32_t*>(screen);
		for (y = h - 1; y >= 0; y--)
		{
			for (x = 0; x < w; x++)
			{
				MD_Color32 = screen32[(y * pitch) + x];
				bmpData[54 + (pos * 3) + 2] = (uint8_t)((MD_Color32 >> 16) & 0xFF);
				bmpData[54 + (pos * 3) + 1] = (uint8_t)((MD_Color32 >> 8) & 0xFF);
				bmpData[54 + (pos * 3) + 0] = (uint8_t)(MD_Color32 & 0xFF);
				pos++;
			}
		}
	}
	
	fwrite(bmpData, 1, bmpSize + 54, fImg);
	free(bmpData);
	
	return 1;
}


#ifdef GENS_PNG
/**
 * writePNG(): Write a PNG image.
 * @param fImg File handle to save the image to.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @param alpha Alpha channel specification. (32-bit color only.)
 * @return 1 on success; 0 on error.
 */
int ImageUtil::writePNG(FILE *fImg, const int w, const int h, const int pitch,
			const void *screen, const int bpp, const AlphaChannel alpha)
{
	if (!fImg || !screen || (w <= 0 || h <= 0 || pitch <= 0))
		return 0;
	
	png_structp png_ptr;
	png_infop info_ptr;
	
	// Initialize libpng.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fprintf(stderr, "writePNG(): Error initializing the PNG pointer.\n");
		return 0;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fprintf(stderr, "writePNG(): Error initializing the PNG info pointer.\n");
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 0;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// TODO: Is setjmp() really necessary?
		fprintf(stderr, "writePNG(): Error initializing the PNG setjmp pointer.\n");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 0;
	}
	
	// Initialize libpng I/O.
	png_init_io(png_ptr, fImg);
	
	// Disable PNG filters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	
	// Set the compression level to 5. (Levels range from 1 through 9.)
	// TODO: Add a UI option to set compression level.
	png_set_compression_level(png_ptr, 5);
	
	// Set up the PNG header.
	if (!(bpp == 32 && alpha != ALPHACHANNEL_NONE))
	{
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB,
			     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			     PNG_FILTER_TYPE_DEFAULT);
	}
	else
	{
		// 32-bit color, with alpha channel.
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
			     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
			     PNG_FILTER_TYPE_DEFAULT);
	}
	
	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);
	
#ifdef GENS_LIL_ENDIAN
	// PNG stores data in big-endian format.
	// On little-endian systems, byteswapping needs to be enabled.
	// TODO: Check if this really isn't needed on big-endian systems.
	png_set_swap(png_ptr);
#endif
	
	// Write the image.
	int x, y;
	if (bpp == 15 || bpp == 16)
	{
		// 15-bit or 16-bit color.
		uint16_t MD_Color;
		uint16_t *screen16 = (uint16_t*)screen;
		
		// Allocate the row buffer.
		uint8_t *rowBuffer;
		if ((rowBuffer = (uint8_t*)malloc(w * 3)) == NULL)
		{
			// Could not allocate enough memory.
			fprintf(stderr, "writePNG(): Could not allocate enough memory for the row buffer.\n");
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return 0;
		}
		
		if (bpp == 15)
		{
			// 15-bit color, 555 pixel format.
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++)
				{
					MD_Color = *screen16;
					rowBuffer[(x * 3) + 0] = (uint8_t)((MD_Color & 0x7C00) >> 7);
					rowBuffer[(x * 3) + 1] = (uint8_t)((MD_Color & 0x03E0) >> 2);
					rowBuffer[(x * 3) + 2] = (uint8_t)((MD_Color & 0x001F) << 3);
					screen16++;
				}
				
				// Write the row.
				png_write_row(png_ptr, rowBuffer);
				
				// Next row.
				screen16 += (pitch - w);
			}
		}
		else if (bpp == 16)
		{
			// 16-bit color, 565 pixel format.
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++)
				{
					MD_Color = *screen16;
					rowBuffer[(x * 3) + 0] = (uint8_t)((MD_Color & 0xF800) >> 8);
					rowBuffer[(x * 3) + 1] = (uint8_t)((MD_Color & 0x07E0) >> 3);
					rowBuffer[(x * 3) + 2] = (uint8_t)((MD_Color & 0x001F) << 3);
					screen16++;
				}
				
				// Write the row.
				png_write_row(png_ptr, rowBuffer);
				
				// Next row.
				screen16 += (pitch - w);
			}
		}
		
		// Free the row buffer.
		free(rowBuffer);
	}
	else // if (bpp == 32)
	{
		// 32-bit color.
		// libpng expects 24-bit data, but has a convenience function
		// to automatically convert 32-bit to 24-bit.
		// TODO: PNG_FILLER_AFTER, BGR mode - needed for little-endian.
		 // Figure out what's needed on big-endian.
		png_byte *row_pointers[240];
		uint32_t *screen32 = (uint32_t*)screen;
		
		for (y = 0; y < h; y++)
		{
			row_pointers[y] = (uint8_t*)&screen32[(y * pitch)];//&MD_Screen32[(y * 336) + 8];
		}
		
		if (!alpha)
		{
			// No alpha channel. Set filler byte.
			png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
		}
		else if (alpha == ALPHACHANNEL_TRANSPARENCY)
		{
			// 0x00 == opaque; 0xFF == transparent.
			png_set_invert_alpha(png_ptr);
		}
		
		png_set_bgr(png_ptr);
		png_write_rows(png_ptr, row_pointers, h);
	}
	
	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	return 1;
}
#endif /* GENS_PNG */


int ImageUtil::write(const string& filename, const ImageFormat format,
		     const int w, const int h, const int pitch,
		     const void *screen, const int bpp, const AlphaChannel alpha)
{
	// Write an image file.
	FILE *fImg = fopen(filename.c_str(), "wb");
	if (!fImg)
	{
		fprintf(stderr, "Image::write(): Error opening %s.\n", filename.c_str());
		return 0;
	}
	
	int rval;
#ifdef GENS_PNG
	if (format == IMAGEFORMAT_PNG)
	{
		rval = writePNG(fImg, w, h, pitch, screen, bpp, alpha);
	}
	else
#endif /* GENS_PNG */
	{
		rval = writeBMP(fImg, w, h, pitch, screen, bpp);
	}
	
	fclose(fImg);
	return rval;
}


/**
 * screenShot(): Convenience function to take a screenshot of the game.
 * @return 1 on success; 0 on error.
 */
int ImageUtil::screenShot(void)
{
	// If no game is running, don't do anything.
	if (!Game)
		return 0;
	
	// Variables used:
	// VDP_Num_Vis_Lines: Number of lines visible on the screen. (bitmap height)
	// MD_Screen: MD screen buffer.
	// VDP_Reg.Set4: If 0x01 is set, 320 pixels width; otherwise, 256 pixels width.
	// TODO: Use macros in video/v_inline.h
	const int w = (VDP_Reg.Set4 & 0x01 ? 320 : 256);
	const int h = VDP_Num_Vis_Lines;
	
	// Build the filename.
	int num = -1;
	char filename[GENS_PATH_MAX];
#ifdef GENS_PNG
	const char* ext = "png";
#else /* !GENS_PNG */
	const char* ext = "bmp";
#endif /* GENS_PNG */
	
	do
	{
		num++;
		sprintf(filename, "%s%s_%03d.%s", PathNames.Screenshot_Dir, ROM_Name, num, ext);
	} while (fileExists(filename));
	
	// Attempt to open the file.
	FILE *img = fopen(filename, "wb");
	if (!img)
	{
		// Error opening the file.
		fprintf(stderr, "ImageUtil::screenShot(): Error opening %s\n", filename);
		return 0;
	}
	
	// Save the image.
	void *screen;
	if (bppMD == 15 || bppMD == 16)
		screen = (void*)(&MD_Screen[8]);
	else //if (bppMD == 32)
		screen = (void*)(&MD_Screen32[8]);
	
	int rval;
#ifdef GENS_PNG
	rval = writePNG(img, w, h, 336, screen, bppMD);
#else /* !GENS_PNG */
	rval = writeBMP(img, w, h, 336, screen, bppMD);
#endif /* GENS_PNG */
	
	// Close the file.
	fclose(img);
	
	if (rval == 1)
		MESSAGE_NUM_L("Screen shot %d saved", "Screen shot %d saved", num, 1500);
	
	return rval;
}
