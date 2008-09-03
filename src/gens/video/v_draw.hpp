/**
 * Gens: Video Drawing base class.
 */

#ifndef GENS_V_DRAW_HPP
#define GENS_V_DRAW_HPP

#include "gens_core/gfx/renderers.h"

#ifdef __cplusplus

// C++ includes
#include <string>
using std::string;

// Constants for m_MsgStyle and m_FPSStyle.
// These match the #define's in gens_core/misc/misc.h.
const unsigned char STYLE_EMU_MODE	= 0x01;
const unsigned char STYLE_COLOR_WHITE	= 0x00;
const unsigned char STYLE_COLOR_BLUE	= 0x02;
const unsigned char STYLE_COLOR_GREEN	= 0x04;
const unsigned char STYLE_COLOR_RED	= 0x06;
const unsigned char STYLE_TRANSPARENT	= 0x08;
const unsigned char STYLE_DOUBLESIZE	= 0x10;

class VDraw
{
	public:
		VDraw();
		VDraw(VDraw *oldDraw);
		virtual ~VDraw() = 0;
		
		virtual int Init_Video(void) = 0;
		virtual void End_Video(void) = 0;
		
		// Initialize the graphics subsystem.
		virtual int Init_Subsystem(void) = 0;
		
		// Shut down the graphics subsystem.
		virtual int Shut_Down(void) = 0;
		
		// Clear the screen.
		virtual void clearScreen(void) = 0;
		//virtual void Clear_Primary_Screen(void) = 0;
		//virtual void Clear_Back_Screen(void) = 0;
		
		// Flip the screen buffer.
		int flip(void);
		
		// Sets the bpp value.
		void setBpp(int newBpp);
		
		// Refresh the video subsystem.
		void Refresh_Video(void);
		
		// Set the rendering mode.
		int setRender(int newMode, bool forceUpdate = true);
		
		// Update VSync value.
		virtual void updateVSync(bool data = false) = 0;
		
		// Write text to the screen.
		void writeText(const string msg, int duration);
		
		// Properties
		bool stretch(void);
		void setStretch(bool newStretch);
		bool swRender(void);
		void setSwRender(bool newSwRender);
		int shift(void);
		void setShift(int newShift);
		bool msgEnabled(void);
		void setMsgEnabled(bool newMsgEnable);
		bool fpsEnabled(void);
		void setFPSEnabled(bool newFPSEnable);
		bool fullScreen(void);
		void setFullScreen(bool newFullScreen);
		
		// Style properties
		unsigned char msgStyle(void);
		void setMsgStyle(unsigned char newMsgStyle);
		unsigned char fpsStyle(void);
		void setFPSStyle(unsigned char newFPSStyle);
		unsigned char introEffectColor(void);
		void setIntroEffectColor(unsigned char newIntroEffectColor);
		
		// Renderers
		// TODO: Make these properties.
		BlitFn Blit_FS;
		BlitFn Blit_W;
		
		// TODO: Move these functions out of v_draw.cpp.
		static int Show_Genesis_Screen(void);
	
	protected:
		// Called if initializatio fails.
		void Init_Fail(const char *err);
		
		// Screen shift in 1x rendering mode.
		int m_shift;
		
		// Stretch option.
		bool m_Stretch;
		
		// Flip the screen buffer. (Renderer-specific function)
		virtual int flipInternal(void) = 0;
		
		// Adjust stretch parameters.
		virtual void stretchAdjustInternal(void);
		
		// Update the renderer.
		virtual void updateRenderer(void) = 0;
		
		// FPS counter
		bool m_FPSEnabled; // Is the FPS counter currently enabled by the user?
		float m_FPS, m_FPS_Frames[8];
		unsigned int m_FPS_OldTime, m_FPS_ViewFPS, m_FPS_IndexFPS;
		unsigned int m_FPS_FreqCPU[2], m_FPS_NewTime[2];
		unsigned char m_FPSStyle;
		
		// On-screen message
		bool m_MsgEnabled; // Is the message function enabled by the user?
		string m_MsgText;
		bool m_MsgVisible; // Is the message currently visible onscreen?
		int m_MsgTime;
		unsigned char m_MsgStyle;
		
		// Intro effect color
		unsigned char m_IntroEffectColor;
		
		// Full Screen
		bool m_FullScreen;
		
		// Screen HBorder.
		// Usually 64 in 256x224 mode. (320 - 256 == 64)
		unsigned char m_HBorder, m_HBorder_Old;
		
		// Current border color.
		unsigned short m_BorderColor_16B;
		unsigned int m_BorderColor_32B;
		
		// Software rendering.
		bool m_swRender;
};

#endif

#endif
