/**
 * Gens: Fast Blur function
 */


#include "fastblur.h"
#include "emulator/g_main.h"
#include "gens_core/vdp/vdp_rend.h"


static void Fast_Blur_16(void);
static void Fast_Blur_32(void);


/**
 * Fast_Blur: Apply a fast blurring algorithm to the onscreen image.
 */
void Fast_Blur(void)
{
	// TODO: Make it so fast blur doesn't apply to screenshots.
	if (bpp == 15 || bpp == 16)
		Fast_Blur_16();
	else // if (bpp == 32)
		Fast_Blur_32();
}


static void Fast_Blur_16(void)
{
	int pixel;
	unsigned short color = 0, colorRB, colorG;
	unsigned short blurColorRB = 0, blurColorG = 0;
	
	unsigned short maskRB, maskG;
	
	// Check bpp.
	if (bpp == 15)
	{
		maskRB = 0x3C0F;
		maskG = 0x01E0;
	}
	else //if (bpp == 16)
	{
		maskRB = 0x780F;
		maskG = 0x03E0;
	}
	
	for (pixel = 1; pixel < (336 * 240); pixel++)
	{
		color = MD_Screen[pixel] >> 1;
		
		// Split the RB and G components.
		colorRB = color & maskRB;
		colorG = color & maskG;
		
		// Blur the colors with the previous pixels.
		blurColorRB += colorRB;
		blurColorG += colorG;
		
		// Draw the new pixel.
		MD_Screen[pixel - 1] = blurColorRB + blurColorG;
		
		// Save the components for the next pixel.
		blurColorRB = colorRB;
		blurColorG = colorG;
	}
}


static void Fast_Blur_32(void)
{
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
