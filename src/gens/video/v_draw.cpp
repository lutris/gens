/**
 * Gens: Video Drawing base class.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "v_draw.hpp"

#include "emulator/g_md.hpp"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/misc.h"
#include "gens_core/misc/fastblur.h"
#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "emulator/g_palette.h"
#include "emulator/ui_proxy.hpp"

#include "port/timer.h"

// Miscellaneous Effects.
#include "v_effects.hpp"

// TODO: Add a wrapper call to sync the GraphicsMenu.
#include "ui/gtk/gens/gens_window_sync.hpp"


VDraw::VDraw()
{
	// Initialize variables.
	m_shift = 0;
	Blit_FS = NULL;
	Blit_W = NULL;
	
	// Initialize the FPS counter.
	m_FPS = 0.0f;
	for (int i = 0; i < 8; i++)
		m_FPS_Frames[i] = 0.0f;
	
	m_FPSEnabled = false;
	m_FPS_OldTime = 0;
	m_FPS_ViewFPS = 0;
	m_FPS_IndexFPS = 0;
	m_FPS_FreqCPU[0] = 0;
	m_FPS_FreqCPU[1] = 0;
	m_FPS_NewTime[0] = 0;
	m_FPS_NewTime[1] = 0;
	m_FPSStyle = 0;
	
	// Initialze the onscreen message.
	m_MsgEnabled = true;
	m_MsgText = "";
	m_MsgVisible = false;
	m_MsgTime = 0;
	m_MsgStyle = 0;
	
	// Others.
	m_Stretch = false;
	m_IntroEffectColor = 7;
	m_FullScreen = false;
	m_fastBlur = false;
	
	// Calculate the text style.
	calcTextStyle();
}

VDraw::VDraw(VDraw *oldDraw)
{
	// Initialize this VDraw based on an existing VDraw object.
	m_shift = oldDraw->shift();
	Blit_FS = oldDraw->Blit_FS;
	Blit_W = oldDraw->Blit_W;
	
	// Initialize the FPS counter.
	// TODO: Copy FPS variables from the other VDraw?
	m_FPS = 0.0f;
	for (int i = 0; i < 8; i++)
		m_FPS_Frames[i] = 0.0f;
	
	m_FPSEnabled = oldDraw->fpsEnabled();
	m_FPS_OldTime = 0;
	m_FPS_ViewFPS = 0;
	m_FPS_IndexFPS = 0;
	m_FPS_FreqCPU[0] = 0;
	m_FPS_FreqCPU[1] = 0;
	m_FPS_NewTime[0] = 0;
	m_FPS_NewTime[1] = 0;
	m_FPSStyle = oldDraw->fpsStyle();
	
	// Initialize the onscreen message.
	// TODO: Copy message variables from the other VDraw?
	m_MsgEnabled = oldDraw->msgEnabled();
	m_MsgText = "";
	m_MsgVisible = false;
	m_MsgTime = 0;
	m_MsgStyle = oldDraw->msgStyle();
	
	// Others.
	m_Stretch = oldDraw->stretch();
	m_IntroEffectColor = oldDraw->introEffectColor();
	m_FullScreen = oldDraw->fullScreen();
	m_fastBlur = oldDraw->fastBlur();
	
	// Calculate the text style.
	calcTextStyle();
}

VDraw::~VDraw()
{
}


/**
 * Init_Fail(): Called by a child class when an error occurs during initializatio.
 * @param err Error message.
 */
void VDraw::Init_Fail(const char *err)
{
	fprintf(stderr, "%s\n", err);
	exit(0);
}


/**
 * Show_Genesis_Screen(): Show the Genesis screen.
 */
int VDraw::Show_Genesis_Screen(void)
{
	Do_VDP_Only();
	//Flip();
	
	return 1;
}


/**
 * Flip(): Flip the screen buffer.
 * @return 1 on success; 0 on error.
 */
int VDraw::flip(void)
{
	// TODO: Print the message and/or FPS counter on the screen buffer only.
	// Don't print it on MD_Screen.
	// Otherwise, messages and the FPS counter show up in screenshots.
	
	// Temporary buffer for sprintf().
	char tmp[64];
	
	// Check if any effects need to be applied.
	// TODO: Make constnats for Intro_Style.
	if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
		if (!Active || Paused)
		{
			// Emulation is paused.
			Pause_Screen();
		}
	}
	else if (Intro_Style == 1)
	{
		// Gens logo effect. (TODO: This is broken!)
		Update_Gens_Logo();
	}
	else if (Intro_Style == 2)
	{
		// "Strange" effect. (TODO: This is broken!)
		Update_Crazy_Effect(m_IntroEffectColor);
	}
	else if (Intro_Style == 3)
	{
		// Genesis BIOS. (TODO: This is broken!)
		Do_Genesis_Frame();
	}
	else
	{
		// Blank screen.
		Clear_Screen_MD();
	}
	
	if (m_MsgVisible)
	{
		if (GetTickCount() > m_MsgTime)
		{
			m_MsgVisible = false;
			m_MsgText = "";
		}
	}
	else if (m_FPSEnabled && (Genesis_Started || _32X_Started || SegaCD_Started) && !Paused)
	{
		if (m_FPS_FreqCPU[0] > 1)	// accurate timer ok
		{
			if (++m_FPS_ViewFPS >= 16)
			{
				GetPerformanceCounter((long long*)m_FPS_NewTime);
				if (m_FPS_NewTime[0] != m_FPS_OldTime)
				{
					m_FPS = (float)(m_FPS_FreqCPU[0]) * 16.0f / (float)(m_FPS_NewTime[0] - m_FPS_OldTime);
					sprintf(tmp, "%.1f", m_FPS);
					m_MsgText = tmp;
				}
				else
				{
					// IT'S OVER 9000 FPS!!!111!11!1
					m_MsgText = ">9000";
				}
				
				m_FPS_OldTime = m_FPS_NewTime[0];
				m_FPS_ViewFPS = 0;
			}
		}
		else if (m_FPS_FreqCPU[0] == 1)	// accurate timer not supported
		{
			if (++m_FPS_ViewFPS >= 10)
			{
				m_FPS_NewTime[0] = GetTickCount();
				
				if (m_FPS_NewTime[0] != m_FPS_OldTime)
					m_FPS_Frames[m_FPS_IndexFPS] = 10000.0f / (float)(m_FPS_NewTime[0] - m_FPS_OldTime);
				else
					m_FPS_Frames[m_FPS_IndexFPS] = 2000;
				
				m_FPS_IndexFPS++;
				m_FPS_IndexFPS &= 7;
				m_FPS = 0.0f;
				
				for (unsigned char i = 0; i < 8; i++)
					m_FPS += m_FPS_Frames[i];
				
				m_FPS /= 8.0f;
				m_FPS_OldTime = m_FPS_NewTime[0];
				m_FPS_ViewFPS = 0;
			}
			sprintf(tmp, "%.1f", m_FPS);
			m_MsgText = tmp;
		}
		else
		{
			GetPerformanceFrequency((long long *)m_FPS_FreqCPU);
			if (m_FPS_FreqCPU[0] == 0)
				m_FPS_FreqCPU[0] = 1;
			
			// TODO: WTF is this for?
			// Assuming it just clears the string...
			//sprintf(Info_String, "", FPS);
			m_MsgText = "";
		}
	}
	
	// Blur the screen if requested.
	if (m_fastBlur)
		Fast_Blur();
	
	// Check if the display width changed.
	m_HBorder_Old = m_HBorder;
	if ((VDP_Reg.Set4 & 0x1) || (Debug))
		m_HBorder = 0;	// 320x224
	else
		m_HBorder = 64;	// 256x224
	
	if (m_HBorder != m_HBorder_Old)
	{
		// Display width change. Adjust the stretch parameters.
		stretchAdjustInternal();
	}
	
	if (m_HBorder > m_HBorder_Old)
	{
		// New screen width is smaller than old screen width.
		// Clear the screen.
		clearScreen();
	}
	
	// Flip the screen buffer.
	return flipInternal();
}


template<typename pixel>
static inline void drawChar_1x(pixel *screen, int x, int y, const int w,
			       pixel dotColor, bool transparent,
			       const pixel transparentMask, const unsigned char ch)
{
	unsigned short cx, cy;
	pixel* screenPos;
	unsigned char cRow;
	
	screenPos = &screen[y*w + x];
	for (cy = 0; cy < 8; cy++)
	{
		// Each character is 8 bytes, with each row representing 8 dots.
		// A 1 indicates the dot is opaque, while a 0 indicates the dot is transparent.
		cRow = C64_charset[ch][cy];
		for (cx = 0; cx < 8; cx++)
		{
			if (cRow & 0x80)
			{
				// Dot is opaque. Draw it.
				// TODO: Original asm version had transparency in a separate function for performance.
				// See if that would actually help.
				if (!transparent)
					*screenPos = dotColor;
				else
					*screenPos = ((dotColor & transparentMask) >> 1) + ((*screenPos & transparentMask) >> 1);
			}
			cRow <<= 1;
			screenPos++;
		}
		screenPos += (w - 8);
	}
}


template<typename pixel>
static inline void drawChar_2x(pixel *screen, const int x, const int y, const int w,
			       pixel dotColor, bool transparent,
			       const pixel transparentMask, const unsigned char ch)
{
	unsigned short cx, cy;
	pixel* screenPos;
	unsigned char cRow;
	
	screenPos = &screen[y*w + x];
	for (cy = 0; cy < 8; cy++)
	{
		// Each character is 8 bytes, with each row representing 8 dots.
		// A 1 indicates the dot is opaque, while a 0 indicates the dot is transparent.
		cRow = C64_charset[ch][cy];
		for (cx = 0; cx < 8; cx++)
		{
			if (cRow & 0x80)
			{
				// Dot is opaque. Draw it.
				// TODO: Original asm version had transparency in a separate function for performance.
				// See if that would actually help.
				if (!transparent)
				{
					*screenPos = dotColor;
					*(screenPos + 1) = dotColor;
					*(screenPos + w) = dotColor;
					*(screenPos + w + 1) = dotColor;
				}
				else
				{
					pixel trPix = (dotColor & transparentMask) >> 1;
					*screenPos = trPix + ((*screenPos & transparentMask) >> 1);
					*(screenPos + 1) = trPix + ((*(screenPos + 1) & transparentMask) >> 1);
					*(screenPos + w) = trPix + ((*(screenPos + w) & transparentMask) >> 1);
					*(screenPos + w + 1) = trPix + ((*(screenPos + w + 1) & transparentMask) >> 1);
				}
			}
			cRow <<= 1;
			screenPos += 2;
		}
		screenPos += (w*2 - 16);
	}
}


template<typename pixel>
void VDraw::drawText_int(pixel *screen, const int w, const int h, const char *msg,
			 const pixel dotColor, const bool doubleSize,
			 bool transparent, const pixel transparentMask)
{
	int msgLength, cPos;
	unsigned short linebreaks, msgWidth;
	unsigned short x, y, cx, cy;
	unsigned char charSize;
	
	// The message must be specified.
	if (!msg)
		return;
	
	// Character size
	if (doubleSize)
		charSize = 16;
	else
		charSize = 8;
	
	// Bottom-left of the screen.
	x = m_HBorder + 8;
	y = h - (((240 - VDP_Num_Vis_Lines) / 2) << m_shift);
	
	// Character size is 8x8 normal, 16x16 double.
	y -= (8 + charSize);
	
	// Get the message length.
	msgLength = strlen(msg);
	
	// Determine how many linebreaks are needed.
	msgWidth = w - 16 - (m_HBorder * 2);
	linebreaks = ((msgLength - 1) * charSize) / msgWidth;
	y -= (linebreaks * charSize);
	
	cx = x; cy = y;
	for (cPos = 0; cPos < msgLength; cPos++)
	{
		if (doubleSize)
		{
			// TODO: Make text shadow an option.
			drawChar_2x(screen, cx+1, cy+1, w, (pixel)0, transparent, transparentMask, (unsigned char)msg[cPos]);
			drawChar_2x(screen, cx-1, cy-1, w, dotColor, transparent, transparentMask, (unsigned char)msg[cPos]);
		}
		else
		{
			// TODO: Make text shadow an option.
			drawChar_1x(screen, cx+1, cy+1, w, (pixel)0, transparent, transparentMask, (unsigned char)msg[cPos]);
			drawChar_1x(screen, cx, cy, w, dotColor, transparent, transparentMask, (unsigned char)msg[cPos]);
		}
		
		cx += charSize;
		if (cx - x >= msgWidth)
		{
			cx = x;
			cy += charSize;
		}
	}
}


void VDraw::drawText(void *screen, const int w, const int h, const char *msg,
		     const unsigned int color, const bool doubleSize, const bool transparent)
{
	if (bpp == 15 || bpp == 16)
	{
		// 15/16-bit color.
		drawText_int((unsigned short*)screen, w, h, msg, (unsigned short)color,
			     doubleSize, transparent, (unsigned short)m_Transparency_Mask);
	}
	else // if (bpp == 32)
	{
		// 32-bit color.
		drawText_int((unsigned int*)screen, w, h, msg, color,
			      doubleSize, transparent, m_Transparency_Mask);
	}
}


static inline void calcTextStyle_int(const unsigned char style,
				     unsigned int *color,
				     bool *doubleSize, bool *transparent)
{
	// Calculate the dot color.
	
	if (bpp == 15 || bpp == 16)
	{
		if (bpp == 15)
		{
			if ((style & 0x07) == STYLE_COLOR_RED)
				*color = 0x7C00;
			else if ((style & 0x07) == STYLE_COLOR_GREEN)
				*color = 0x03E0;
			else if ((style & 0x07) == STYLE_COLOR_BLUE)
				*color = 0x001F;
			else //if ((style & 0x07) == STYLE_COLOR_WHITE)
				*color = 0x7FFF;
		}
		else // if (bpp == 16)
		{
			if ((style & 0x07) == STYLE_COLOR_RED)
				*color = 0xF800;
			else if ((style & 0x07) == STYLE_COLOR_GREEN)
				*color = 0x07E0;
			else if ((style & 0x07) == STYLE_COLOR_BLUE)
				*color = 0x001F;
			else //if ((style & 0x07) == STYLE_COLOR_WHITE)
				*color = 0xFFFF;
		}
	}
	else //if (bpp == 32)
	{
		if ((style & 0x07) == STYLE_COLOR_RED)
			*color = 0xFF0000;
		else if ((style & 0x07) == STYLE_COLOR_GREEN)
			*color = 0x00FF00;
		else if ((style & 0x07) == STYLE_COLOR_BLUE)
			*color = 0x0000FF;
		else //if ((style & 0x07) == STYLE_COLOR_WHITE)
			*color = 0xFFFFFF;
	}
	
	*doubleSize = (style & STYLE_DOUBLESIZE);
	*transparent = (style & STYLE_TRANSPARENT);
}


/**
 * calcTextStyle(): Calculates the text style values
 */
void VDraw::calcTextStyle(void)
{
	// Calculate the transparency mask.
	if (bpp == 15)
		m_Transparency_Mask = 0x7BDE;
	else if (bpp == 16)
		m_Transparency_Mask = 0xF7DE;
	else //if (bpp == 32)
		m_Transparency_Mask = 0xFEFEFE;
	
	// Calculate the style values for FPS and Msg.
	calcTextStyle_int(m_FPSStyle, &m_FPSColor, &m_FPSDoubleSize, &m_FPSTransparent);
	calcTextStyle_int(m_MsgStyle, &m_MsgColor, &m_MsgDoubleSize, &m_MsgTransparent);
}


/**
 * setBpp(): Sets the bpp value.
 * @param newbpp New bpp value.
 */
void VDraw::setBpp(int newBpp)
{
	if (bpp == newBpp)
		return;
	
	bpp = newBpp;
	End_Video();
	Init_Video();
	
	// Reset the renderer.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	if (!setRender(rendMode))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (!setRender(0))
		{
			// Cannot initialize normal mode.
			fprintf(stderr, "%s: FATAL ERROR: Cannot initialize any renderers.\n", __func__);
			exit(1);
		}
	}
	
	// Recalculate palettes.
	Recalculate_Palettes();
	
	// Calculate the text style.
	calcTextStyle();
	
	// Synchronize the Graphics menu.
	Sync_Gens_Window_GraphicsMenu();
	
	// TODO: After switching color depths, the screen buffer isn't redrawn
	// until something's updated. Figure out how to trick the renderer
	// into updating anyway.
	
	// NOTE: This only seems to be a problem with 15-to-16 or 16-to-15 at the moment.
	
	// TODO: Figure out if 32-bit rendering still occurs in 15/16-bit mode and vice-versa.
}


/**
 * writeText(): Write text to the screen.
 * @param msg Message to write.
 * @param duration Duration for the message to appear, in milliseconds.
 */
void VDraw::writeText(string msg, unsigned short duration)
{
	if (!m_MsgEnabled)
		return;
	m_MsgText = msg;
	m_MsgTime = GetTickCount() + duration;
	m_MsgVisible = true;
}


/**
 * stretchAdjustInternal(): Adjust the stretch parameters.
 */
void VDraw::stretchAdjustInternal(void)
{
	// VDraw doesn't do anything here by itself...
}


/**
 * Refresh_Video(): Refresh the video subsystem.
 */
void VDraw::Refresh_Video(void)
{
	// Reset the border color to make sure it's redrawn.
	m_BorderColor_16B = ~MD_Palette[0];
	m_BorderColor_32B = ~MD_Palette32[0];
	
	End_Video();
	Init_Video();
	stretchAdjustInternal();
}


/**
 * setRender(): Set the rendering mode.
 * @param newMode Rendering mode / filter.
 * @param forceUpdate If true, forces a renderer update.
 */
int VDraw::setRender(int newMode, bool forceUpdate)
{
	int Old_Rend, *Rend;
	BlitFn *Blit, testBlit;
	
	if (m_FullScreen)
	{
		Blit = &Blit_FS;
		Rend = &Video.Render_FS;
		Old_Rend = Video.Render_FS;
	}
	else
	{
		Blit = &Blit_W;
		Rend = &Video.Render_W;
		Old_Rend = Video.Render_W;
	}
	
	// Checks if an invalid mode number was passed.
	if (newMode < 0 || newMode >= Renderers_Count)
	{
		// Invalid mode number.
		MESSAGE_NUM_L("Error: Render mode %d is not available.",
			      "Error: Render mode %d is not available.", newMode, 1500);
		return 0;
	}
	
	// Check if a blit function exists for this renderer.
	if (bpp == 32)
		testBlit = (Have_MMX ? Renderers[newMode].blit_32_mmx : Renderers[newMode].blit_32);
	else if (bpp == 15 || bpp == 16)
		testBlit = (Have_MMX ? Renderers[newMode].blit_16_mmx : Renderers[newMode].blit_16);
	else
	{
		// Invalid bpp.
		fprintf(stderr, "Invalid bpp: %d\n", bpp);
		return 0;
	}
	
	if (!testBlit)
	{
		// Renderer function not found.
		if (Renderers[newMode].name)
		{
			MESSAGE_STR_L("Error: Render mode %s is not available.",
				      "Error: Render mode %s is not available.", Renderers[newMode].name, 1500);
		}
		return 0;
	}
	
	// Renderer function found.
	if (*Rend != newMode)
		MESSAGE_STR_L("Render Mode: %s", "Render Mode: %s", Renderers[newMode].name, 1500);
	*Rend = newMode;
	*Blit = testBlit;
	
	setShift(newMode == 0 ? 0 : 1);
	
	//if (Num>3 || Num<10)
	//Clear_Screen();
	// if( (Old_Rend==NORMAL && Num==DOUBLE)||(Old_Rend==DOUBLE && Num==NORMAL) ||Opengl)
	// this doesn't cover hq2x etc. properly. Let's just always refresh.
	
	// Update the renderer.
	if (forceUpdate && is_gens_running())
		updateRenderer();
	
	return 1;
}


bool VDraw::stretch(void)
{
	return m_Stretch;
}
void VDraw::setStretch(bool newStretch)
{
	if (m_Stretch == newStretch)
		return;
	m_Stretch = newStretch;
	stretchAdjustInternal();
}


bool VDraw::swRender(void)
{
	return m_swRender;
}
void VDraw::setSwRender(bool newSwRender)
{
	if (m_swRender == newSwRender)
		return;
	m_swRender = newSwRender;
	
	// TODO: Figure out what to do here...
}


int VDraw::shift(void)
{
	return m_shift;
}
void VDraw::setShift(int newShift)
{
	if (m_shift == newShift)
		return;
	m_shift = newShift;
	
	// TODO: Figure out what to do here...
}


bool VDraw::msgEnabled(void)
{
	return m_MsgEnabled;
}
void VDraw::setMsgEnabled(bool newMsgEnabled)
{
	if (m_MsgEnabled == newMsgEnabled)
		return;
	m_MsgEnabled = newMsgEnabled;
	
	// TODO: Figure out what to do here...
}


bool VDraw::fpsEnabled(void)
{
	return m_FPSEnabled;
}
void VDraw::setFPSEnabled(bool newFPSEnabled)
{
	if (m_FPSEnabled == newFPSEnabled)
		return;
	m_FPSEnabled = newFPSEnabled;
	
	// TODO: Figure out what to do here...
}


// Style properties
unsigned char VDraw::msgStyle(void)
{
	return m_MsgStyle;
}
void VDraw::setMsgStyle(unsigned char newMsgStyle)
{
	if (m_MsgStyle == newMsgStyle)
		return;
	m_MsgStyle = newMsgStyle;
	
	// Calculate the text style.
	calcTextStyle();
}


unsigned char VDraw::fpsStyle(void)
{
	return m_FPSStyle;
}
void VDraw::setFPSStyle(unsigned char newFPSStyle)
{
	if (m_FPSStyle == newFPSStyle)
		return;
	m_FPSStyle = newFPSStyle;
	
	// Calculate the text style.
	calcTextStyle();
}


unsigned char VDraw::introEffectColor(void)
{
	return m_IntroEffectColor;
}
void VDraw::setIntroEffectColor(unsigned char newIntroEffectColor)
{
	if (m_IntroEffectColor == newIntroEffectColor)
		return;
	if (/*newIntroEffectColor < 0 ||*/ newIntroEffectColor > 7)
		return;
	
	m_IntroEffectColor = newIntroEffectColor;
	
	// TODO: Figure out what to do here...
}


bool VDraw::fullScreen(void)
{
	return m_FullScreen;
}
void VDraw::setFullScreen(bool newFullScreen)
{
	if (m_FullScreen == newFullScreen)
		return;
	
	m_FullScreen = newFullScreen;
	
	// Set the renderer.
	int newRend = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	setRender(newRend, false);
	
	// Refresh the video subsystem, if Gens is running.
	if (is_gens_running())
		Refresh_Video();
}


bool VDraw::fastBlur(void)
{
	return m_fastBlur;
}
void VDraw::setFastBlur(bool newFastBlur)
{
	if (m_fastBlur == newFastBlur)
		return;
	
	m_fastBlur = newFastBlur;
}
