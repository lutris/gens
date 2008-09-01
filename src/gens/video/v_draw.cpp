/**
 * Gens: Video Drawing base class.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "v_draw.hpp"

#include "emulator/g_md.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/misc.h"
#include "gens_core/misc/fastblur.h"
#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"

#include "port/timer.h"

// Miscellaneous Effects.
#include "v_effects.hpp"


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
		Update_Crazy_Effect();
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
		else
		{
			Print_Text(m_MsgText.c_str(), m_MsgText.length(), 10, 210, m_MsgStyle);
		}
	}
	else if (Show_FPS && (Genesis_Started || _32X_Started || SegaCD_Started) && !Paused)
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
		
		Print_Text(m_MsgText.c_str(), m_MsgText.length(), 10, 210, m_FPSStyle);
	}
	
	// Blur the screen if requested.
	if (Video.Fast_Blur)
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
		Adjust_Stretch();
	}
	
	if (m_HBorder > m_HBorder_Old)
	{
		// New screen width is smaller than old screen width.
		// Clear the screen.
		Clear_Screen();
	}
	
	// Flip the screen buffer.
	// TODO: VSync.
	return flipInternal();
}


/**
 * writeText(): Write text to the screen.
 * @param msg Message to write.
 * @param duration Duration for the message to appear, in milliseconds.
 */
void VDraw::writeText(string msg, int duration)
{
	if (!m_MsgEnabled)
		return;
	m_MsgText = msg;
	m_MsgTime = GetTickCount() + duration;
	m_MsgVisible = true;
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
	Adjust_Stretch();
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
	Adjust_Stretch();
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
	
	// TODO: Figure out what to do here...
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
	
	// TODO: Figure out what to do here...
}
