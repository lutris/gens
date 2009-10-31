/***************************************************************************
 * Gens: Video Drawing - Hailu Backend.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#include "vdraw.h"
#include "vdraw_haiku.hpp"

// Message logging.
#include "macros/log_msg.h"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"

// VDraw C++ functions.
#include "vdraw_cpp.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Gens window.
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// RGB color conversion functions.
#include "vdraw_RGB.h"

// Audio Handler.
#include "audio/audio.h"


// X and Y resolutions.
static int Res_X;
static int Res_Y;


// Function prototypes.
int	vdraw_haiku_init(void);
int	vdraw_haiku_end(void);

void vdraw_haiku_clear_screen(void);

int	vdraw_haiku_flip(void);
void vdraw_haiku_update_renderer(void);


/**
 * vdraw_haiku_init(): Initialize the Haiku video subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_haiku_init(void)
{
	return 0;
}


/**
 * vdraw_haiku_end(): Close the Haiku renderer.
 * @return 0 on success; non-zero on error.
 */
int vdraw_haiku_end(void)
{
	return 0;
}


/**
 * vdraw_haiku_clear_screen(): Clear the screen.
 */
void vdraw_haiku_clear_screen(void)
{
}


/**
 * vdraw_haiku_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
int vdraw_haiku_flip(void)
{
	return 0;
}


/**
 * vdraw_haiku_draw_border(): Draw the border color.
 * Called from vdraw_haiku_flip().
 */
void vdraw_haiku_draw_border(void)
{
}


/**                     
 * vdraw_haiku_reinit_gens_window(): Reinitialize the Gens window. * @return 0 on success; non-zero on error.
 */                     
int vdraw_haiku_reinit_gens_window(void)
{                       
	return vdraw_haiku_init();
}


/**
 * vdraw_haiku_update_renderer(): Update the renderer.
 */
void vdraw_haiku_update_renderer(void)
{
}


/**
 * vdraw_haiku_update_vsync(): Update the VSync value.
 * @param fromInitHaiku If true, this function is being called from vdraw_haiku_init().
 */             
void vdraw_haiku_update_vsync(const BOOL fromInitHaiku)
{                       
        GSFT_UNUSED_PARAMETER(fromInitHaiku);
                        
        // If Full Screen, reinit();
}
