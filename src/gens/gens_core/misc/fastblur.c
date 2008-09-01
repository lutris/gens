/**
 * Gens: Fast Blur function
 */

#include "fastblur.h"
#include "emulator/g_main.h"
#include "gens_core/vdp/vdp_rend.h"

/**
 * Fast_Blur: Apply a fast blurring algorithm to the onscreen image.
 */
void Fast_Blur(void)
{
	// TODO: Make it so fast blur doesn't apply to screenshots.
	
	int pixel;
	int color = 0, colorRB, colorG;
	int blurColorRB = 0, blurColorG = 0;
	
	for (pixel = 1; pixel < (336 * 240); pixel++)
	{
		color = MD_Screen32[pixel] >> 1;
		
		// Split the RB and G components.
		colorRB = color & 0xFF00FF;
		colorG = color & 0x00FF00;
		
		// Blur the colors with the previous pixels.
		blurColorRB += colorRB;
		blurColorG += colorG;
		
		// Draw the new pixel.
		MD_Screen32[pixel - 1] = blurColorRB + blurColorG;
		
		// Save the components for the next pixel.
		blurColorRB = colorRB;
		blurColorG = colorG;
	}
}
