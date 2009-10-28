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

#ifndef GENS_IMAGEUTIL_HPP
#define GENS_IMAGEUTIL_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// TODO: Eliminate this include.
// Move GENS_PATH_MAX somewhere else.
#include "emulator/g_main.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class ImageUtil
{
	public:
		enum ImageFormat
		{
			IMAGEFORMAT_BMP = 0,
#ifdef GENS_PNG
			IMAGEFORMAT_PNG = 1,
#endif /* GENS_PNG */
			IMAGEFORMAT_COUNT
		};
		
		enum AlphaChannel
		{
			ALPHACHANNEL_NONE,
			ALPHACHANNEL_OPACITY,
			ALPHACHANNEL_TRANSPARENCY
		};
		
#ifdef GENS_PNG
		static const ImageFormat DefaultImageFormat = IMAGEFORMAT_PNG;
#else /* !GENS_PNG */
		static const ImageFormat DefaultImageFormat = IMAGEFORMAT_BMP;
#endif /* GENS_PNG */
		
		static int WriteImage(const char* filename, const ImageFormat format,
					const int w, const int h, const int pitch,
					const void *screen, const int bpp,
					const AlphaChannel alpha = ALPHACHANNEL_NONE);
		
		static int ScreenShot(void);
	
	protected:
		static int WriteBMP(FILE *fImg, const int w, const int h, const int pitch,
					const void *screen, const int bpp);
		
		static int WritePNG(FILE *fImg, const int w, const int h, const int pitch,
					const void *screen, const int bpp,
					const AlphaChannel alpha = ALPHACHANNEL_NONE);
	
	private:
		// Don't allow instantiation of this class.
		ImageUtil() { }
		~ImageUtil() { }
};

#endif /* __cplusplus */

#endif /* GENS_IMAGEUTIL_HPP */
