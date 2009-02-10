/***************************************************************************
 * Gens: Input Class - DirectInput 5                                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#ifndef GENS_INPUT_DINPUT_HPP
#define GENS_INPUT_DINPUT_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "macros/bool_m.h"
#include "input.h"

// Function prototypes.
int		input_dinput_init(void);
int		input_dinput_end(void);

int		input_dinput_update(void);
BOOL		input_dinput_check_key_pressed(uint16_t key);
unsigned int	input_dinput_get_key(void);
BOOL		input_dinput_joy_exists(int joy_num);

// Win32-specific functions.
int		input_dinput_set_cooperative_level(HWND hWnd);

// Win32-specific functions not used in input_backend_t.
int		input_dinput_init_joysticks(HWND hWnd);

// Default keymap.
extern const input_keymap_t input_dinput_keymap_default[8];

#if 0
class Input_DInput : public Input
{
	public:
		Input_DInput();
		~Input_DInput();
		
		// DirectInput only acquires joysticks if the window is visible.
		// So, this function is called when the Gens window is made visible.
		void initJoysticks(HWND hWnd);
		
		// Update the input subsystem.
		void update(void);
		
		// Check if the specified key is pressed.
		bool checkKeyPressed(unsigned int key);
		
		// Get a key. (Used for controller configuration.)
		unsigned int getKey(void);
		
		// EnumDevices callback for joysticks.
		static BOOL CALLBACK EnumDevices_Joysticks(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
		BOOL EnumDevices_Joysticks_int(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
		
		// Cooperative level (Win32)
		void setCooperativeLevel(HWND hWnd = NULL);
	
	protected:
		// Functions required by the Input class.
		bool joyExists(int joyNum);
		
		// Number of joysticks connected
		int m_numJoysticks;
		
		// DirectInput versions.
		static const unsigned short DIRECTINPUT_VERSION_5 = 0x0500;
		static const unsigned short DIRECTINPUT_VERSION_3 = 0x0300;
		
		// DirectInput variables
		LPDIRECTINPUT lpDI;
		LPDIRECTINPUTDEVICE lpDIDKeyboard;
		LPDIRECTINPUTDEVICE lpDIDMouse;
		IDirectInputDevice2 *m_joyID[MAX_JOYS];
		DIJOYSTATE m_joyState[MAX_JOYS];
		
		// DirectInput version
		unsigned short m_diVersion;
		
		// DirectInput Keys array
		unsigned char m_DIKeys[256];
		
		void restoreInput(void);
		
		// Joysticks
		void setCooperativeLevel_Joysticks(HWND hWnd = NULL);
		bool joysticksInitialized;
		bool joystickError;
};
#endif

#ifdef __cplusplus
}
#endif

#endif
