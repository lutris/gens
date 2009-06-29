/* md_ntsc 0.1.2. http://www.slack.net/~ant/ */

#include "md_ntsc.hpp"

/* Copyright (C) 2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

md_ntsc_setup_t const md_ntsc_monochrome = {{{0,-1, 0, 0,.2,  0, 0,-.2,-.2,-1}}, 0, 0};
md_ntsc_setup_t const md_ntsc_composite  = {{{0, 0, 0, 0, 0,  0, 0,  0,  0, 0}}, 0, 0};
md_ntsc_setup_t const md_ntsc_svideo     = {{{0, 0, 0, 0, 0,  0,.2, -1, -1, 0}}, 0, 0};
md_ntsc_setup_t const md_ntsc_rgb        = {{{0, 0, 0, 0,.2,  0,.7, -1, -1,-1}}, 0, 0};

#define alignment_count 2
#define burst_count     1
#define rescale_in      1
#define rescale_out     1

#define artifacts_mid   0.40f
#define fringing_mid    0.30f
#define std_decoder_hue 0

#define gamma_size      8
#define artifacts_max   1.00f
#define LUMA_CUTOFF     0.1974

#include "md_ntsc_impl.h"

#include <string.h>

/* 2 input pixels -> 4 composite samples */
const pixel_info_t md_ntsc_pixels[alignment_count] =
{
	{PIXEL_OFFSET(-4, -9), {0.1f, 0.9f, 0.9f, 0.1f}},
	{PIXEL_OFFSET(-2, -7), {0.1f, 0.9f, 0.9f, 0.1f}},
};

static void correct_errors(md_ntsc_rgb_t color, md_ntsc_rgb_t* out)
{
	for (unsigned int i = 0; i < rgb_kernel_size / 4; i++)
	{
		md_ntsc_rgb_t error = color -
				out [i    ] - out [i + 2    +16] - out [i + 4    ] - out [i + 6    +16] -
				out [i + 8] - out [(i+10)%16+16] - out [(i+12)%16] - out [(i+14)%16+16];
		
		CORRECT_ERROR(i + 6 + 16);
		/*DISTRIBUTE_ERROR( 2+16, 4, 6+16 );*/
	}
}


/**
 * md_ntsc_init(): Initialize the NTSC filter.
 * @param ntsc NTSC filter information.
 * @param setup Setup parameters.
 */
void md_ntsc_init(md_ntsc_t* ntsc, md_ntsc_setup_t const* setup)
{
	init_t impl;
	
	// Default to composite video if no setup parameters were specified.
	if (!setup)
		setup = &md_ntsc_composite;
	init(&impl, setup);
	
	for (int entry = 0; entry < md_ntsc_palette_size; entry++)
	{
		float bb = impl.to_float [entry >> 6 & 7];
		float gg = impl.to_float [entry >> 3 & 7];
		float rr = impl.to_float [entry      & 7];
		
		float y, i, q = RGB_TO_YIQ(rr, gg, bb, y, i);
		
		int r, g, b = YIQ_TO_RGB(y, i, q, impl.to_rgb, int, r, g);
		md_ntsc_rgb_t rgb = PACK_RGB(r, g, b);
		
		if (setup->palette_out)
			RGB_PALETTE_OUT(rgb, &setup->palette_out [entry * 3]);
		
		if (ntsc)
		{
			gen_kernel(&impl, y, i, q, ntsc->table [entry]);
			correct_errors(rgb, ntsc->table [entry]);
		}
	}
}


/**
 * MD_NTSC_RGB_OUT_(): Templated RGB OUT function.
 */
/* x is always zero except in snes_ntsc library */
template<typename pixel, pixel maskR, pixel maskG, pixel maskB, int shiftR, int shiftG, int shiftB>
static inline void MD_NTSC_RGB_OUT_(pixel *rgb_out, int x, md_ntsc_rgb_t raw_)
{
	*(rgb_out) = ((raw_ >> (shiftR - x)) & maskR) |
		     ((raw_ >> (shiftG - x)) & maskG) |
		     ((raw_ >> (shiftB - x)) & maskB);
}


/**
 * T_md_ntsc_blit(): Templated NTSC blit function.
 */
template<typename pixel, pixel maskR, pixel maskG, pixel maskB, int shiftR, int shiftG, int shiftB>
static inline void T_md_ntsc_blit(md_ntsc_t const* ntsc, uint16_t const* input,
				  int in_row_width, int in_width, int height,
				  pixel *rgb_out, int out_pitch)
{
	const int chunk_count = in_width / md_ntsc_in_chunk - 1;
	
	/* Calculate the output pitch difference for one scanline. */
	const int outPitchDiff = (out_pitch / sizeof(pixel));
	
	while (height--)
	{
		uint16_t const* line_in = input;
		MD_NTSC_BEGIN_ROW(ntsc, md_ntsc_black,
				  line_in[0], line_in[1], line_in[2]);
		pixel *restrict line_out = rgb_out;
		int n;
		line_in += 3;
		
		for (n = chunk_count; n; --n)
		{
			/* order of input and output pixels must not be altered */
			MD_NTSC_COLOR_IN(0, ntsc, line_in[0]);
			MD_NTSC_RGB_OUT(0, line_out[0]);
			MD_NTSC_RGB_OUT(1, line_out[1]);
			
			MD_NTSC_COLOR_IN(1, ntsc, line_in[1]);
			MD_NTSC_RGB_OUT(2, line_out[2]);
			MD_NTSC_RGB_OUT(3, line_out[3]);
			
			MD_NTSC_COLOR_IN(2, ntsc, line_in[2]);
			MD_NTSC_RGB_OUT(4, line_out[4]);
			MD_NTSC_RGB_OUT(5, line_out[5]);
			
			MD_NTSC_COLOR_IN(3, ntsc, line_in[3]);
			MD_NTSC_RGB_OUT(6, line_out[6]);
			MD_NTSC_RGB_OUT(7, line_out[7]);
			
			line_in  += 4;
			line_out += 8;
		}
		
		/* finish final pixels */
		MD_NTSC_COLOR_IN(0, ntsc, line_in[0]);
		MD_NTSC_RGB_OUT(0, line_out[0]);
		MD_NTSC_RGB_OUT(1, line_out[1]);
		
		MD_NTSC_COLOR_IN(1, ntsc, md_ntsc_black);
		MD_NTSC_RGB_OUT(2, line_out[2]);
		MD_NTSC_RGB_OUT(3, line_out[3]);
		
		MD_NTSC_COLOR_IN(2, ntsc, md_ntsc_black);
		MD_NTSC_RGB_OUT(4, line_out[4]);
		MD_NTSC_RGB_OUT(5, line_out[5]);
		
		MD_NTSC_COLOR_IN(3, ntsc, md_ntsc_black);
		MD_NTSC_RGB_OUT(6, line_out[6]);
		MD_NTSC_RGB_OUT(7, line_out[7]);
		
		input += in_row_width;
		rgb_out += (outPitchDiff * 2);
	}
}


/**
 * T_double_scan_image(): Templated double-scan image function.
 */
template<typename pixel, pixel lowPixelMask, pixel darkenMask>
static inline void T_double_scan_image(pixel *rgb_out, int in_width, int height,
				       int out_pitch, bool scanline, bool interp)
{
	// Calculate the output pitch difference for one scanline.
	const int outPitchDiff = (out_pitch / sizeof(pixel));
	
	// Fill in the scanlines.
	if (!scanline && !interp)
	{
		// Scanlines and interpolation are disabled.
		// Do a simple double-scan.
		while (height--)
		{
			memcpy(rgb_out + outPitchDiff, rgb_out, in_width * sizeof(pixel) * 2);
			rgb_out += (outPitchDiff * 2);
		}
	}
	else
	{
		// Scanlines and/or interpolation are enabled.
		if (interp)
		{
			// Interpolation is enabled.
			for (unsigned int y = height; y != 0; y--)
			{
				const pixel *in = rgb_out;
				pixel *out = rgb_out + outPitchDiff;
				
				for (unsigned int x = in_width*2; x != 0; x--)
				{
					const pixel prev = *in;
					
					// Get the current pixel and the pixel on the next scanline.
					const pixel next = (x > 1 && y > 1) ? *(in + (outPitchDiff * 2)) : 0;
					
					// Mix RGB without losing low bits.
					const uint32_t mixed = (prev + next) + ((prev ^ next) & lowPixelMask);
					
					if (scanline)
					{
						// Darken the pixel by 12%.
						*out = (mixed >> 1) - (mixed >> 4 & darkenMask);
					}
					else
					{
						// Don't darken the pixel at all.
						*out = (mixed >> 1);
					}
					
					// Next set of pixels.
					in++;
					out++;
				}
				
				// Next line.
				rgb_out += (outPitchDiff * 2);
			}
		}
		else
		{
			// Interpolation is disabled.
			for (unsigned int y = height; y != 0; y--)
			{
				const pixel *in = rgb_out;
				pixel *out = rgb_out + outPitchDiff;
				
				for (unsigned int x = in_width*2; x != 0; x--)
				{
					const pixel prev = *in;
					
					// Mix RGB without losing low bits.
					//const uint32_t mixed = (prev + prev) + ((prev ^ prev) & lowPixelMask);
					
					// Darken by 12%.
					*out = prev - (prev >> 3 & darkenMask);
					
					// Next set of pixels.
					in++;
					out++;
				}
				
				// Next line.
				rgb_out += (outPitchDiff * 2);
			}
		}
	}
}

/* MDP Renderer Function */

#include <stdlib.h>

/* MDP includes. */
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"
static md_ntsc_t *mdp_md_ntsc = NULL;

// NTSC setup struct.
md_ntsc_setup_t mdp_md_ntsc_setup;

// Scanline / Interpolation options.
int mdp_md_ntsc_scanline = 1;
int mdp_md_ntsc_interp = 1;

/* Sony CXA2025AS US decoder matrix. */
const float mdp_md_ntsc_sony_cxa2025as_us_matrix[6] = {1.630, 0.317, -0.378, -0.466, -1.089, 1.677};

int MDP_FNCALL mdp_md_ntsc_init(void)
{
	// Allocate mdp_md_ntsc.
	mdp_md_ntsc = (md_ntsc_t*)malloc(sizeof(md_ntsc_t));
	
	// Initialize mdp_md_ntsc_setup.
	mdp_md_ntsc_setup = md_ntsc_composite;
	
	// Initialize mdp_md_ntsc.
	md_ntsc_init(mdp_md_ntsc, &mdp_md_ntsc_setup);
}

int MDP_FNCALL mdp_md_ntsc_end(void)
{
	// Free mdp_md_ntsc.
	free(mdp_md_ntsc);
	mdp_md_ntsc = NULL;
}

void MDP_FNCALL mdp_md_ntsc_reinit_setup(void)
{
	// Reinitialize mdp_md_ntsc.
	md_ntsc_init(mdp_md_ntsc, &mdp_md_ntsc_setup);
}


int MDP_FNCALL mdp_md_ntsc_blit(mdp_render_info_t *render_info)
{
	if (!render_info)
		return -MDP_ERR_RENDER_INVALID_RENDERINFO;;
	
	if (MDP_RENDER_VMODE_GET_SRC(render_info->vmodeFlags) != MDP_RENDER_VMODE_RGB_565)
	{
		// NTSC filter only supports 16-bit color input.
		return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	switch (MDP_RENDER_VMODE_GET_DST(render_info->vmodeFlags))
	{
		case MDP_RENDER_VMODE_RGB_565:
			// 16-bit color.
			T_md_ntsc_blit<uint16_t, 0xF800, 0x07E0, 0x001F, 13, 8, 4>
					(mdp_md_ntsc,
					 (uint16_t*)render_info->mdScreen,
					 render_info->srcPitch / 2,
					 render_info->width, render_info->height,
					 (uint16_t*)render_info->destScreen,
					 render_info->destPitch);
			T_double_scan_image<uint16_t, 0x0821, 0x18E3>
					((uint16_t*)render_info->destScreen,
					 render_info->width, render_info->height,
					 render_info->destPitch,
					 mdp_md_ntsc_scanline,
					 mdp_md_ntsc_interp);
			break;
		
		case MDP_RENDER_VMODE_RGB_555:
			// 15-bit color.
			T_md_ntsc_blit<uint16_t, 0x7C00, 0x03E0, 0x001F, 14, 9, 4>
					(mdp_md_ntsc,
					 (uint16_t*)render_info->mdScreen,
					 render_info->srcPitch / 2,
					 render_info->width, render_info->height,
					 (uint16_t*)render_info->destScreen,
					 render_info->destPitch);
			T_double_scan_image<uint16_t, 0x0421, 0x0C63>
					((uint16_t*)render_info->destScreen,
					 render_info->width, render_info->height,
					 render_info->destPitch,
					 mdp_md_ntsc_scanline,
					 mdp_md_ntsc_interp);
			break;
		
		case MDP_RENDER_VMODE_RGB_888:
			// 32-bit color.
			T_md_ntsc_blit<uint32_t, (uint32_t)0xFF0000, (uint32_t)0x00FF00, (uint32_t)0x0000FF, 5, 3, 1>
					(mdp_md_ntsc,
					 (uint16_t*)render_info->mdScreen,
					 render_info->srcPitch / 2,
					 render_info->width, render_info->height,
					 (uint32_t*)render_info->destScreen,
					 render_info->destPitch);
			T_double_scan_image<uint32_t, (uint32_t)0x010101, (uint32_t)0x0F0F0F>
					((uint32_t*)render_info->destScreen,
					 render_info->width, render_info->height,
					 render_info->destPitch,
					 mdp_md_ntsc_scanline,
					 mdp_md_ntsc_interp);
			break;
		
		default:
			// Unsupported video mode.
			return -MDP_ERR_RENDER_UNSUPPORTED_VMODE;
	}
	
	return MDP_ERR_OK;
}
