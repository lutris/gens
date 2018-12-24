/***************************************************************************
 * Gens: Image Utilities.                                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "imageutil.hpp"
#include "bmp.h"

// Message logging.
#include "macros/log_msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"

// libgsft includes.
#include "libgsft/gsft_byteswap.h"
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_szprintf.h"

// File management functions.
#include "util/file/file.hpp"

// Text drawing.
#include "video/vdraw_text.hpp"

#ifdef GENS_PNG
#include "libgsft/gsft_png_dll.h"
#endif /* GENS_PNG */


// Pixel masks.
static const uint16_t MASK_RED_15	= 0x7C00;
static const uint16_t MASK_GREEN_15	= 0x03E0;
static const uint16_t MASK_BLUE_15	= 0x001F;

static const uint16_t MASK_RED_16	= 0xF800;
static const uint16_t MASK_GREEN_16	= 0x07E0;
static const uint16_t MASK_BLUE_16	= 0x001F;

static const uint32_t MASK_RED_32	= 0xFF0000;
static const uint32_t MASK_GREEN_32	= 0x00FF00;
static const uint32_t MASK_BLUE_32	= 0x0000FF;

// Pixel shifts.
static const uint8_t SHIFT_RED_15	= 7;
static const uint8_t SHIFT_GREEN_15	= 2;
static const uint8_t SHIFT_BLUE_15	= 3;

static const uint8_t SHIFT_RED_16	= 8;
static const uint8_t SHIFT_GREEN_16	= 3;
static const uint8_t SHIFT_BLUE_16	= 3;

static const uint8_t SHIFT_RED_32	= 16;
static const uint8_t SHIFT_GREEN_32	= 8;
static const uint8_t SHIFT_BLUE_32	= 0;


/**
 * T_writeBMP_rows(): Write BMP rows.
 * @param pixel Typename.
 * @param maskR Red mask.
 * @param maskG Green mask.
 * @param maskB Blue mask.
 * @param shiftR Red shift. (Right)
 * @param shiftG Green shift. (Right)
 * @param shiftB Blue shift. (Left)
 * @param screen Pointer to the screen buffer.
 * @param bmpOut Buffer to write BMP data to.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 */
template<typename pixel,
	 const pixel maskR, const pixel maskG, const pixel maskB,
	 const unsigned int shiftR, const unsigned int shiftG, const unsigned int shiftB>
static inline void T_writeBMP_rows(const pixel *screen, uint8_t *bmpOut,
				   const int width, const int height, const int pitch)
{
	// Bitmaps are stored upside-down.
	for (int y = height - 1; y >= 0; y--)
	{
		const pixel *curScreen = &screen[y * pitch];
		for (int x = 0; x < width; x++)
		{
			pixel MD_Color = *curScreen++;
			*bmpOut++ = (uint8_t)((MD_Color & maskB) << shiftB);
			*bmpOut++ = (uint8_t)((MD_Color & maskG) >> shiftG);
			*bmpOut++ = (uint8_t)((MD_Color & maskR) >> shiftR);
		}
	}
}


/**
 * WriteBMP(): Write a BMP image.
 * @param fImg File handle to save the image to.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @return 0 on success; non-zero on error.
 */
int ImageUtil::WriteBMP(FILE *fImg, const int w, const int h, const int pitch,
			const void *screen, const int bpp)
{
	if (!fImg || !screen || (w <= 0 || h <= 0 || pitch <= 0))
		return 1;
	
	// Calculate the size of the bitmap image.
	int bmpSize = (w * h * 3);
	uint8_t *bmpData = new uint8_t[bmpSize];
	if (!bmpData)
	{
		// Could not allocate enough memory.
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Could not allocate enough memory for the bitmap data.");
		return 2;
	}
	
	// Build the bitmap image.
	bmp_header_t bmp_header;
	
	// Magic Number.
	bmp_header.magic_number[0] = 'B';
	bmp_header.magic_number[1] = 'M';
	
	// Bitmap header data.
	bmp_header.size			= cpu_to_le32(bmpSize + sizeof(bmp_header));
	bmp_header.reserved1		= cpu_to_le16(0);
	bmp_header.reserved2		= cpu_to_le16(0);
	bmp_header.bmp_start		= cpu_to_le32(sizeof(bmp_header));
	bmp_header.bmp_header_size	= cpu_to_le32(40);
	bmp_header.width		= cpu_to_le32(w);
	bmp_header.height		= cpu_to_le32(h);
	bmp_header.planes		= cpu_to_le16(1);
	bmp_header.bpp			= cpu_to_le16(24);
	bmp_header.compression		= cpu_to_le32(0);
	bmp_header.bmp_size		= cpu_to_le32(bmpSize);
	bmp_header.ppm_x		= cpu_to_le32(0x0EC4);
	bmp_header.ppm_y		= cpu_to_le32(0x0EC4);
	bmp_header.colors_used		= cpu_to_le32(0);
	bmp_header.important_colors	= cpu_to_le32(0);
	
	// TODO: Verify endianness requirements.
	
	if (bpp == 15)
	{
		// 15-bit color. (Mode 555)
		T_writeBMP_rows<uint16_t,
				MASK_RED_15, MASK_GREEN_15, MASK_BLUE_15,
				SHIFT_RED_15, SHIFT_GREEN_15, SHIFT_BLUE_15>
			       (static_cast<const uint16_t*>(screen), bmpData, w, h, pitch);
	}
	else if (bpp == 16)
	{
		// 16-bit color. (Mode 565)
		T_writeBMP_rows<uint16_t,
				MASK_RED_16, MASK_GREEN_16, MASK_BLUE_16,
				SHIFT_RED_16, SHIFT_GREEN_16, SHIFT_BLUE_16>
			       (static_cast<const uint16_t*>(screen), bmpData, w, h, pitch);
	}
	else //if (bpp == 32)
	{
		// 32-bit color.
		// BMP uses 24-bit color, so a conversion is still necessary.
		T_writeBMP_rows<uint32_t,
				MASK_RED_32, MASK_GREEN_32, MASK_BLUE_32,
				SHIFT_RED_32, SHIFT_GREEN_32, SHIFT_BLUE_32>
			       (static_cast<const uint32_t*>(screen), bmpData, w, h, pitch);
	}
	
	fwrite(&bmp_header, 1, sizeof(bmp_header), fImg);
	fwrite(bmpData, 1, bmpSize, fImg);
	delete[] bmpData;
	
	return 0;
}


#ifdef GENS_PNG
/**
 * T_writePNG_rows_16(): Write 16-bit PNG rows.
 * @param pixel Typename.
 * @param maskR Red mask.
 * @param maskG Green mask.
 * @param maskB Blue mask.
 * @param shiftR Red shift. (Right)
 * @param shiftG Green shift. (Right)
 * @param shiftB Blue shift. (Left)
 * @param screen Pointer to the screen buffer.
 * @param png_ptr PNG pointer.
 * @param info_ptr PNG info pointer.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @return 0 on success; non-zero on error.
 */
template<typename pixel,
	 const pixel maskR, const pixel maskG, const pixel maskB,
	 const unsigned int shiftR, const unsigned int shiftG, const unsigned int shiftB>
static inline int T_writePNG_rows_16(const pixel *screen, png_structp png_ptr, png_infop info_ptr,
				     const int width, const int height, const int pitch)
{
	// Allocate the row buffer.
	uint8_t *rowBuffer = new uint8_t[width * 3];
	if (!rowBuffer)
	{
		// Could not allocate enough memory.
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Could not allocate enough memory for the PNG row buffer.");
		ppng_destroy_write_struct(&png_ptr, &info_ptr);
		return 1;
	}
	
	// Write the rows.
	for (int y = 0; y < height; y++)
	{
		uint8_t *rowBufPtr = rowBuffer;
		for (int x = 0; x < width; x++)
		{
			pixel MD_Color = *screen++;
			*rowBufPtr++ = (uint8_t)((MD_Color & maskR) >> shiftR);
			*rowBufPtr++ = (uint8_t)((MD_Color & maskG) >> shiftG);
			*rowBufPtr++ = (uint8_t)((MD_Color & maskB) << shiftB);
		}
		
		// Write the row.
		ppng_write_row(png_ptr, rowBuffer);
		
		// Next row.
		screen += (pitch - width);
	}
	
	// Free the row buffer.
	delete[] rowBuffer;
	
	return 0;
}


/**
 * WritePNG(): Write a PNG image.
 * @param fImg File handle to save the image to.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @param alpha Alpha channel specification. (32-bit color only.)
 * @return 0 on success; non-zero on error.
 */
int ImageUtil::WritePNG(FILE *fImg, const int w, const int h, const int pitch,
			const void *screen, const int bpp, const AlphaChannel alpha)
{
	if (!fImg || !screen || (w <= 0 || h <= 0 || pitch <= 0))
		return 1;
	
	int rval = gsft_png_dll_init();
	if (rval)
		return rval;
	
	png_structp png_ptr;
	png_infop info_ptr;
	
	// Initialize libpng.
	png_ptr = ppng_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Error initializing the PNG pointer.");
		return 2;
	}
	info_ptr = ppng_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Error initializing the PNG info pointer.");
		ppng_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return 3;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// TODO: Is setjmp() really necessary?
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Error initializing the PNG setjmp pointer.");
		ppng_destroy_write_struct(&png_ptr, &info_ptr);
		return 4;
	}
	
	// Initialize libpng I/O.
	ppng_init_io(png_ptr, fImg);
	
	// Disable PNG filters.
	ppng_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	
	// Set the compression level to 5. (Levels range from 1 through 9.)
	// TODO: Add a UI option to set compression level.
	ppng_set_compression_level(png_ptr, 5);
	
	// Set up the PNG header.
	if (!(bpp == 32 && alpha != ALPHACHANNEL_NONE))
	{
		ppng_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	}
	else
	{
		// 32-bit color, with alpha channel.
		ppng_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	}
	
	// Write the PNG information to the file.
	ppng_write_info(png_ptr, info_ptr);
	
#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
	// PNG stores data in big-endian format.
	// On little-endian systems, byteswapping needs to be enabled.
	// TODO: Check if this really isn't needed on big-endian systems.
	ppng_set_swap(png_ptr);
#endif
	
	// Write the image.
	if (bpp == 15)
	{
		// 15-bit color. (Mode 555)
		rval = T_writePNG_rows_16<uint16_t,
					  MASK_RED_15, MASK_GREEN_15, MASK_BLUE_15,
					  SHIFT_RED_15, SHIFT_GREEN_15, SHIFT_BLUE_15>
					 (static_cast<const uint16_t*>(screen), png_ptr, info_ptr, w, h, pitch);
		
		if (rval != 0)
			return 5;
	}
	else if (bpp == 16)
	{
		// 16-bit color. (Mode 565)
		rval = T_writePNG_rows_16<uint16_t,
					  MASK_RED_16, MASK_GREEN_16, MASK_BLUE_16,
					  SHIFT_RED_16, SHIFT_GREEN_16, SHIFT_BLUE_16>
					 (static_cast<const uint16_t*>(screen), png_ptr, info_ptr, w, h, pitch);
		
		if (rval != 0)
			return 6;
	}
	else // if (bpp == 32)
	{
		// 32-bit color.
		// Depending on the alpha channel settings, libpng expects either
		// 24-bit data (no alpha) or 32-bit data (with alpha); however,
		// libpng offers an option to automatically convert 32-bit data
		// without alpha channel to 24-bit. (png_set_filler())
		
		// TODO: PNG_FILLER_AFTER, BGR mode - needed for little-endian.
		 // Figure out what's needed on big-endian.
		
		png_byte **row_pointers = static_cast<png_byte**>(malloc(sizeof(png_byte*) * h));
		uint32_t *screen32 = (uint32_t*)screen;
		
		for (int y = 0; y < h; y++)
		{
			row_pointers[y] = (uint8_t*)&screen32[y * pitch];
		}
		
		if (!alpha)
		{
			// No alpha channel. Set filler byte.
			ppng_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
		}
		else if (alpha == ALPHACHANNEL_TRANSPARENCY)
		{
			// Alpha channel indicates transparency.
			// 0x00 == opaque; 0xFF == transparent.
			ppng_set_invert_alpha(png_ptr);
		}
		
		ppng_set_bgr(png_ptr);
		ppng_write_rows(png_ptr, row_pointers, h);
		
		free(row_pointers);
	}
	
	// Finished writing.
	ppng_write_end(png_ptr, info_ptr);
	ppng_destroy_write_struct(&png_ptr, &info_ptr);
	
	return 0;
}
#endif /* GENS_PNG */


/**
 * WriteImage(): Write an image.
 * @param filename Filename to write to.
 * @param format Image format.
 * @param w Width of the image.
 * @param h Height of the image.
 * @param pitch Pitch of the image. (measured in pixels)
 * @param screen Pointer to screen buffer.
 * @param bpp Bits per pixel.
 * @param alpha Alpha channel specification. (32-bit color only.)
 * @return 0 on success; non-zero on error.
 */
int ImageUtil::WriteImage(const char* filename, const ImageFormat format,
			  const int w, const int h, const int pitch,
			  const void *screen, const int bpp, const AlphaChannel alpha)
{
	if (!filename)
		return 1;
	
	// Write an image file.
	FILE *fImg = fopen(filename, "wb");
	if (!fImg)
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Error opening %s.", filename);
		return 2;
	}
	
	int rval;
#ifdef GENS_PNG
	if (format == IMAGEFORMAT_PNG)
	{
		rval = WritePNG(fImg, w, h, pitch, screen, bpp, alpha);
	}
	else
#endif /* GENS_PNG */
	{
		rval = WriteBMP(fImg, w, h, pitch, screen, bpp);
	}
	
	fclose(fImg);
	return rval;
}


/**
 * ScreenShot(): Convenience function to take a screenshot of the game.
 * @return 0 on success; non-zero on error.
 */
int ImageUtil::ScreenShot(void)
{
	// If no game is running, don't do anything.
	if (!Game)
		return 1;
	
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
	
	const char *ext;
	ImageFormat fmt;
#ifdef GENS_PNG
	if (gsft_png_dll_init() == 0)
	{
		// PNG initialized.
		ext = "png";
		fmt = IMAGEFORMAT_PNG;
	}
	else
#endif /* GENS_PNG */
	{
		// Couldn't initialize PNG.
		// Use BMP instead.
		ext = "bmp";
		fmt = IMAGEFORMAT_BMP;
	}
	
	do
	{
		num++;
		szprintf(filename, sizeof(filename), "%s%s_%03d.%s", PathNames.Screenshot_Dir, ROM_Filename, num, ext);
	} while (gsft_file_exists(filename));
	
	void *screen;
	if (bppMD == 15 || bppMD == 16)
		screen = (void*)(&MD_Screen[8]);
	else //if (bppMD == 32)
		screen = (void*)(&MD_Screen32[8]);
	
	// Attempt to save the screenshot.
	int rval = WriteImage(filename, fmt, w, h, 336, screen, bppMD);
	
	if (!rval)
		vdraw_text_printf(1500, "Screen shot %d saved.", num);
	
	return rval;
}
