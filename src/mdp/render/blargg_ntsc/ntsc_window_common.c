/***************************************************************************
 * MDP: Blargg's NTSC Filter. (Window Code) (Common Data)                  *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

#include "ntsc_window_common.h"

#include <math.h>

#ifdef GENS_UI_GTK
#define NTSC_MNEMONIC_CHAR "_"
#else
#define NTSC_MNEMONIC_CHAR "&"
#endif

// Presets.
const ntsc_preset_t ntsc_presets[NTSC_PRESETS_COUNT+1] =
{
	{"Composite",	&md_ntsc_composite},
	{"S-Video",	&md_ntsc_svideo},
	{"RGB",		&md_ntsc_rgb},
	{"Monochrome",	&md_ntsc_monochrome},
	{"Custom",	NULL},
	{NULL, NULL}
};

// Adjustment controls.
const ntsc_ctrl_t ntsc_controls[NTSC_CTRL_COUNT+1] =
{
	{"Hue",			NTSC_MNEMONIC_CHAR "Hue",		-180, 180, 1},
	{"Saturation",		NTSC_MNEMONIC_CHAR "Saturation",	0, 200, 5},
	{"Contrast",		NTSC_MNEMONIC_CHAR "Contrast",		-100, 100, 5},
	{"Brightness",		NTSC_MNEMONIC_CHAR "Brightness",	-100, 100, 5},
	{"Sharpness",		"S" NTSC_MNEMONIC_CHAR "harpness",	-100, 100, 5},
	
	// "Advanced" parameters.
	{"Gamma",		NTSC_MNEMONIC_CHAR "Gamma",		50, 150, 5},
	{"Resolution",		NTSC_MNEMONIC_CHAR "Resolution",	-100, 100, 5},
	{"Artifacts",		NTSC_MNEMONIC_CHAR "Artifacts",		-100, 100, 5},
	{"Color Fringing",	"Color " NTSC_MNEMONIC_CHAR "Fringing",	-100, 100, 5},
	{"Color Bleed",		"Color B" NTSC_MNEMONIC_CHAR "leed",	-100, 100, 5},
	
	{NULL, NULL, 0, 0, 0}
};


/**
 * ntsc_internal_to_display(): Convert an internal value to a display value.
 * @param valID
 * @param ntsc_val
 */
int MDP_FNCALL ntsc_internal_to_display(ntsc_value_t valID, double ntsc_val)
{
	switch (valID)
	{
		case NTSC_VALUE_HUE:
			return (int)rint(ntsc_val * 180.0);
		
		case NTSC_VALUE_SATURATION:
			return (int)rint((ntsc_val + 1.0) * 100.0);
		
		case NTSC_VALUE_CONTRAST:
		case NTSC_VALUE_BRIGHTNESS:
		case NTSC_VALUE_SHARPNESS:
			return (int)rint(ntsc_val * 100.0);
		
		case NTSC_VALUE_GAMMA:
			return (int)rint(((ntsc_val / 2.0) + 1.0) * 100.0);
		
		case NTSC_VALUE_RESOLUTION:
		case NTSC_VALUE_ARTIFACTS:
		case NTSC_VALUE_FRINGING:
		case NTSC_VALUE_BLEED:
			return (int)rint(ntsc_val * 100.0);
		
		case NTSC_VALUE_MAX:
		default:
			return 0;
	}
}


/**
 * ntsc_internal_to_display(): Convert a display value to an internal value.
 * @param valID
 * @param disp_val
 */
double MDP_FNCALL ntsc_display_to_internal(ntsc_value_t valID, int disp_val)
{
	double dval;
	if (valID == NTSC_VALUE_HUE)
		dval = rint(disp_val);
	else
		dval = rint(disp_val) / 100.0;
	
	switch (valID)
	{
		case NTSC_VALUE_HUE:
			return (dval / 180.0);
		
		case NTSC_VALUE_SATURATION:
			return (dval - 1.0);
		
		case NTSC_VALUE_CONTRAST:
		case NTSC_VALUE_BRIGHTNESS:
		case NTSC_VALUE_SHARPNESS:
			return dval;
		
		case NTSC_VALUE_GAMMA:
			return (dval - 1.0) * 2.0;
		
		case NTSC_VALUE_RESOLUTION:
		case NTSC_VALUE_ARTIFACTS:
		case NTSC_VALUE_FRINGING:
		case NTSC_VALUE_BLEED:
			return dval;
		
		case NTSC_VALUE_MAX:
		default:
			return 0;
	}
}
