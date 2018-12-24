/***************************************************************************
 * Gens: Input Handler - DirectInput 5 Backend.                            *
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

#include "input.h"
#include "input_dinput.hpp"
#include "input_win32_keys.h"

// C includes.
#include <stdio.h>

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

// DirectInput versions.
#define DIRECTINPUT_VERSION_5 0x0500
#define DIRECTINPUT_VERSION_3 0x0300

// We want DirectInput 5 headers.
#define DIRECTINPUT_VERSION DIRECTINPUT_VERSION_5
#include <dinput.h>

// Wine's headers are missing this definition...
#ifndef IDirectInputDevice2
#ifdef UNICODE
#define IDirectInputDevice2 IDirectInputDevice2W
#else
#define IDirectInputDevice2 IDirectInputDevice2A
#endif /* UNICODE */
#endif /* IDirectInputDevice2 */

// Some old compilers may be missing WM_NCXBUTTONDBLCLK.
// In particular, Cygwin's headers don't have it.
#ifndef WM_NCXBUTTONDBLCLK
#define WM_NCXBUTTONDBLCLK 0x00AD
#endif

#define MAX_JOYS 8

// DirectInput variables.
static LPDIRECTINPUT lpDI = NULL;
static LPDIRECTINPUTDEVICE lpDIDKeyboard = NULL;
static LPDIRECTINPUTDEVICE lpDIDMouse = NULL;
static IDirectInputDevice2 *input_dinput_joy_id[MAX_JOYS];
static DIJOYSTATE input_dinput_joy_state[MAX_JOYS];

// DirectInput version.
static unsigned short input_dinput_version = 0;

// DirectInput Keys array.
static unsigned char input_dinput_keys[256];

// Joysticks.
static BOOL input_dinput_joystick_initialized;
static BOOL input_dinput_joystick_error;
static int input_dinput_num_joysticks;	// Number of joysticks connected
static int input_dinput_callback_init_joysticks_enum_counter;	// Number of times the enumeration function has been called.
static BOOL CALLBACK input_dinput_callback_init_joysticks_enum(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
int input_dinput_set_cooperative_level_joysticks(HWND hWnd);

// Miscellaneous DirectInput functions.
static inline void input_dinput_restore_input(void);

// Used for the Controller Configuration window.
static BOOL CALLBACK input_dinput_callback_add_joysticks_to_listbox(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
static int input_dinput_add_joysticks_count;

// Default keymap.
const input_keymap_t input_dinput_keymap_default[8] =
{
	// Player 1
	{{DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT,
	DIK_RETURN, DIK_A, DIK_S, DIK_D,
	DIK_RSHIFT, DIK_Q, DIK_W, DIK_E}},
	
	// Player 2
	{{DIK_Y, DIK_H, DIK_G, DIK_J,
	DIK_U, DIK_K, DIK_L, DIK_M,
	DIK_T, DIK_I, DIK_O, DIK_P}},
	
	// Players 1B, 1C, 1D
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	
	// Players 2B, 2C, 2D
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

#define MOD_NONE 0
#define VK_NONE 0
#define ID_NONE 0


/**
 * input_dinput_init(): Initialize the DirectInput subsystem.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_init(void)
{
	int i;
	HRESULT rval;
	
	// Attempt to initialize DirectInput 5.
	input_dinput_version = 0;
	rval = DirectInputCreate(ghInstance, DIRECTINPUT_VERSION_5, &lpDI, NULL);
	if (rval == DI_OK)
	{
		// DirectInput 5 initialized.
		LOG_MSG(input, LOG_MSG_LEVEL_INFO,
			"Initialized DirectInput 5.");
		input_dinput_version = DIRECTINPUT_VERSION_5;
	}
	else
	{
		// Attempt to initialize DirectInput 3.
		rval = DirectInputCreate(ghInstance, DIRECTINPUT_VERSION_3, &lpDI, NULL);
		if (rval == DI_OK)
		{
			// DirectInput 3 initialized.
			LOG_MSG(input, LOG_MSG_LEVEL_INFO,
				"Initialized DirectInput 3.");
			input_dinput_version = DIRECTINPUT_VERSION_3;
		}
		else
		{
			// DirectInput could not be initialized.
			LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
				"Could not initialize DirectInput 3 or DirectInput 5.\n\nYou must have DirectX 3 or later.");
			return -1;
		}
	}
	
	input_dinput_joystick_initialized = false;
	input_dinput_joystick_error = false;
	input_dinput_num_joysticks = 0;
	memset(input_dinput_joy_id, 0x00, sizeof(input_dinput_joy_id));
	
	//rval = lpDI->CreateDevice(GUID_SysMouse, &lpDIDMouse, NULL);
	lpDIDMouse = NULL;
	rval = lpDI->CreateDevice(GUID_SysKeyboard, &lpDIDKeyboard, NULL);
	if (rval != DI_OK)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"lpDI->CreateDevice() failed. (Keyboard)");
		
		// TODO: Use cross-platform error numbers, not just DirectInput return values.
		return -2;
	}
	
	// Set the cooperative level.
	input_dinput_set_cooperative_level(NULL);
	
	//rval = lpDIDMouse->SetDataFormat(&c_dfDIMouse);
	rval = lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (rval != DI_OK)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"lpDIDKeyboard->SetDataFormat() failed.");
		
		// TODO: Use cross-platform error numbers, not just DirectInput return values.
		return -1;
	}
	
	//rval = lpDIDMouse->Acquire();
	for(i = 0; i < 10; i++)
	{
		rval = lpDIDKeyboard->Acquire();
		if (rval == DI_OK)
			break;
		GensUI::sleep(10);
	}
	
	// Clear the DirectInput arrays.
	memset(input_dinput_keys, 0x00, sizeof(input_dinput_keys));
	memset(input_dinput_joy_id, 0x00, sizeof(input_dinput_joy_id));
	memset(input_dinput_joy_state, 0x00, sizeof(input_dinput_joy_state));
	
	// DirectInput initialized.
	return 0;
}


/**
 * input_dinput_end(): Shut down the DirectInput subsystem.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_end(void)
{
	// If any joysticks were opened, close them.
	if (!lpDI)
		return 0;
	
	if (lpDIDMouse)
	{
		lpDIDMouse->Release();
		lpDIDMouse = NULL;
	}
	
	if (lpDIDKeyboard)
	{
		lpDIDKeyboard->Release();
		lpDIDKeyboard = NULL;
	}
	
	for (int i = 0; i < MAX_JOYS; i++)
	{
		if (input_dinput_joy_id[i])
		{
			input_dinput_joy_id[i]->Unacquire();
			input_dinput_joy_id[i]->Release();
			input_dinput_joy_id[i] = NULL;
		}
	}
	
	input_dinput_num_joysticks = 0;
	lpDI->Release();
	lpDI = NULL;
	
	// Clear the DirectInput version variable.
	input_dinput_version = 0;
	
	// DirectInput shut down.
	return 0;
}


/**
 * input_dinput_init_joysticks(): Initialize joysticks.
 * @param hWnd Window handle where the joysticks should be acquired initially.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_init_joysticks(HWND hWnd)
{
	// TODO: Check if joysticks work with DirectInput 3.
	if (input_dinput_joystick_error)
		return -1;
	
	if (input_dinput_joystick_initialized)
	{
		// Close and re-initialize joysticks.
		for (int i = 0; i < MAX_JOYS; i++)
		{
			if (input_dinput_joy_id[i])
			{
				input_dinput_joy_id[i]->Unacquire();
				input_dinput_joy_id[i]->Release();
				input_dinput_joy_id[i] = NULL;
			}
		}
		
		input_dinput_joystick_error = false;
		input_dinput_num_joysticks = 0;
		memset(input_dinput_joy_id, 0x00, sizeof(input_dinput_joy_id));
		
		// Set the cooperative level.
		// TODO: If set to DISCL_FOREGROUND, run setCooperativeLevel_Joysticks().
		// TODO: If set to DISCL_BACKGROUND, don't run setCooperativeLevel_Joysticks().
		// Currently hard-coded for DISCL_BACKGROUND.
		//setCooperativeLevel_Joysticks(hWnd);
	}
	
	// Joysticks are being initialized.
	input_dinput_joystick_initialized = true;
	input_dinput_callback_init_joysticks_enum_counter = 0;
	
	HRESULT rval;
	rval = lpDI->EnumDevices(DIDEVTYPE_JOYSTICK,
				 &input_dinput_callback_init_joysticks_enum,
				 hWnd, DIEDFL_ATTACHEDONLY);
	if (rval != DI_OK)
	{
		input_dinput_joystick_error = true;
		
		// Close all joysticks.
		for (int i = 0; i < MAX_JOYS; i++)
		{
			if (input_dinput_joy_id[i])
			{
				input_dinput_joy_id[i]->Unacquire();
				input_dinput_joy_id[i]->Release();
				input_dinput_joy_id[i] = NULL;
			}
		}
		
		// Reset number of joysticks.
		input_dinput_num_joysticks = 0;
		
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"lpDI->EnumDevices() failed. Joysticks will be unavailable.");
		
		// TODO: Error handling.
		return -2;
	}
	
	// Joysticks initialized.
	return 0;
}


/**
 * input_dinput_callback_init_joysticks_enum(): EnumDevices callback for initializing joysticks.
 * @param lpDIIJoy Joystick information.
 * @param pvRef hWnd of the Gens window.
 * @return DIENUM_CONTINUE to continue the enumeration; DIENUM_STOP to stop the enumeration.
 */
static BOOL CALLBACK input_dinput_callback_init_joysticks_enum(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	HRESULT rval;
	LPDIRECTINPUTDEVICE lpDIJoy;
 
	if (!lpDIIJoy || input_dinput_num_joysticks >= MAX_JOYS ||
	    input_dinput_callback_init_joysticks_enum_counter >= MAX_JOYS)
		return DIENUM_STOP;
	
	// Number of times this function has been called.
	int cur_joystick = input_dinput_callback_init_joysticks_enum_counter;
	input_dinput_callback_init_joysticks_enum_counter++;
	
	rval = lpDI->CreateDevice(lpDIIJoy->guidInstance, &lpDIJoy, NULL);
	if (rval != DI_OK)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"lpDI->CreateDevice() failed. (Joystick %d)", cur_joystick);
		return DIENUM_CONTINUE;
	}
	
	IDirectInputDevice2 *pJoyDevice = NULL;
	input_dinput_joy_id[input_dinput_num_joysticks] = NULL;
	rval = lpDIJoy->QueryInterface(IID_IDirectInputDevice2, (void**)&pJoyDevice);
	
	lpDIJoy->Release();
	if (rval != DI_OK || !pJoyDevice)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"lpDIJoy->QueryInterface() failed. (Joystick %d)", cur_joystick);
		return DIENUM_CONTINUE;
	}
	
	rval = pJoyDevice->SetDataFormat(&c_dfDIJoystick);
	if (rval != DI_OK)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"pJoyDevice->SetDatFormat(&c_dfDIJoystick) failed. (Joystick %d)", cur_joystick);
		pJoyDevice->Release();
		return DIENUM_CONTINUE;
	}
	
	// TODO: Add an option to specify DISCL_FOREGROUND so the joysticks only work when the Gens window is active.
	rval = pJoyDevice->SetCooperativeLevel((HWND)pvRef, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	
	if (rval != DI_OK)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_WARNING,
			"pJoyDevice->SetCooperativeLevel() failed. (Joystick %d)", cur_joystick);
		pJoyDevice->Release();
		return DIENUM_CONTINUE;
	}
	
	// Attempt to acquire the joystick device.
	for (unsigned int i = 10; i != 0; i--)
	{
		rval = pJoyDevice->Acquire();
		if (rval == DI_OK)
			break;
		GensUI::sleep(10);
	}
	
	input_dinput_joy_id[input_dinput_num_joysticks] = pJoyDevice;
	input_dinput_num_joysticks++;
	
	return DIENUM_CONTINUE;
}


/**
 * input_dinput_restore_input(): Restore DirectInput.
 */
static inline void input_dinput_restore_input(void)
{
	//lpDIDMouse->Acquire();
	lpDIDKeyboard->Acquire();
}


/**
 * input_dinput_joy_exists(): Check if the specified joystick exists.
 * @param joyNum Joystick number.
 * @return true if the joystick exists; false if it does not exist.
 */
BOOL input_dinput_joy_exists(int joyNum)
{
	if (joyNum < 0 || joyNum >= MAX_JOYS)
		return FALSE;
	
	if (input_dinput_joy_id[joyNum])
		return TRUE;
	
	// Joystick does not exist.
	return FALSE;
}


/**
 * input_dinput_get_key(): Get a key. (Used for controller configuration.)
 * @return Key value.
 */
uint16_t input_dinput_get_key(void)
{
	bool prevReady = false;
	
	bool prevDiKeys[256];
	bool prevJoyKeys[48*6];
	
	bool curDiKeys[256];
	bool curJoyKeys[48*6];
	
	while (true)
	{
		// Compute the current state of all buttons.
		input_dinput_update();
		
		// Current state of DirectInput keys
		for (int key = 0; key < 256; key++)
			curDiKeys[key] = (input_dinput_keys[key] & 0x80);
		
		// Current state of recognized buttons on joypad
		int joyIndex = 0;
		for (int joystick = 0; joystick < input_dinput_num_joysticks; joystick++)
		{
			if (!input_dinput_joy_exists(joystick))
				continue;
			
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lX <= 0x3FFF);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lX >= 0xC000);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lY <= 0x3FFF);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lY >= 0xC000);
			
			// TODO: Determine the correct axis order and the correct axis values.
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lZ <= 0x3FFF);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lZ >= 0xC000);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lRx <= 0x3FFF);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lRx >= 0xC000);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lRy <= 0x3FFF);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lRy >= 0xC000);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lRz <= 0x3FFF);
			curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].lRz >= 0xC000);
			
			for (int povhat = 0; povhat < 4; povhat++)
			{
				curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].rgdwPOV[povhat] == 0);
				curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].rgdwPOV[povhat] == 9000);
				curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].rgdwPOV[povhat] == 18000);
				curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].rgdwPOV[povhat] == 27000);
			}
			
			for (int button = 0; button < 32; button++)
			{
				curJoyKeys[joyIndex++] = (input_dinput_joy_state[joystick].rgbButtons[button]);
			}
		}
		
		// Compare buttons against the previous state to determine
		// what is pressed now that wasn't already pressed before.
		
		if (prevReady)
		{
			// Check for new DirectInput key presses.
			for (int key = 1; key < 255; key++)
			{
				if (curDiKeys[key] && !prevDiKeys[key])
					return key;
			}
			
			// Check for new recognized joypad button presses.
			for (int index = 0; index < joyIndex; index++)
			{
				if (curJoyKeys[index] && !prevJoyKeys[index])
				{
					int joyIndex2 = 0;
					for (int joystick = 0; joystick < input_dinput_num_joysticks; joystick++)
					{
						if (!input_dinput_joy_exists(joystick))
							continue;
						
						// X, Y
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 0, INPUT_JOYSTICK_AXIS_NEGATIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 0, INPUT_JOYSTICK_AXIS_POSITIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 1, INPUT_JOYSTICK_AXIS_NEGATIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 1, INPUT_JOYSTICK_AXIS_POSITIVE);
						
						// Z, Rx, Ry, Rz
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 2, INPUT_JOYSTICK_AXIS_NEGATIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 2, INPUT_JOYSTICK_AXIS_POSITIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 3, INPUT_JOYSTICK_AXIS_NEGATIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 3, INPUT_JOYSTICK_AXIS_POSITIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 4, INPUT_JOYSTICK_AXIS_NEGATIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 4, INPUT_JOYSTICK_AXIS_POSITIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 5, INPUT_JOYSTICK_AXIS_NEGATIVE);
						if (index == joyIndex2++)
							return INPUT_GETKEY_AXIS(joystick, 5, INPUT_JOYSTICK_AXIS_POSITIVE);
						
						// POV hats
						for (int povhat = 0; povhat < 4; povhat++)
						{
							if (index == joyIndex2++)
								return INPUT_GETKEY_POVHAT_DIRECTION(joystick, povhat, INPUT_JOYSTICK_POVHAT_UP);
							if (index == joyIndex2++)
								return INPUT_GETKEY_POVHAT_DIRECTION(joystick, povhat, INPUT_JOYSTICK_POVHAT_RIGHT);
							if (index == joyIndex2++)
								return INPUT_GETKEY_POVHAT_DIRECTION(joystick, povhat, INPUT_JOYSTICK_POVHAT_DOWN);
							if (index == joyIndex2++)
								return INPUT_GETKEY_POVHAT_DIRECTION(joystick, povhat, INPUT_JOYSTICK_POVHAT_LEFT);
						}
						
						// Buttons
						for (int button = 0; button < 32; button++)
						{
							if (index == joyIndex2++)
								return INPUT_GETKEY_BUTTON(joystick, button);
						}
					}
				}
			}
		}
		
		// Update previous state.
		memcpy(prevDiKeys, curDiKeys, sizeof(prevDiKeys));
		memcpy(prevJoyKeys, curJoyKeys, sizeof(prevJoyKeys));
		prevReady = true;
		
		// Sleep for 1 ms.
		Sleep(1);
		
		// Check for messages.
		// GensUI::update() isn't used here because some messages
		// need to be filtered out.
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				close_gens();
			
			// Ignore key input and mouse input.
			if (msg.message == WM_COMMAND ||
			    (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
			    (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
			{
				// Ignore the message.
				continue;
			}
			
			// Ignore menu clicks.
			if ((msg.message >= WM_NCMOUSEMOVE && msg.message <= WM_NCXBUTTONDBLCLK) &&
			    (msg.wParam == HTMENU || msg.wParam == HTSYSMENU))
			{
				// Ignore the message.
				continue;
			}
			
			// Process the message.
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


/**
 * input_dinput_update(): Update the input subsystem.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_update(void)
{
	//DIMOUSESTATE MouseState;
	HRESULT rval;
	
	rval = lpDIDKeyboard->GetDeviceState(256, &input_dinput_keys);
	
	// HACK because DirectInput is totally wacky about recognizing the PAUSE/BREAK key
	// still not perfect with this, but at least it goes above a 25% success rate
	if(GetAsyncKeyState(VK_PAUSE)) // normally this should have & 0x8000, but apparently this key is too special for that to work
		input_dinput_keys[0xC5] |= 0x80;
	
	if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
		input_dinput_restore_input();
	
	for (int i = 0; i < input_dinput_num_joysticks; i++)
	{
		if (input_dinput_joy_id[i])
		{
			input_dinput_joy_id[i]->Poll();
			rval = input_dinput_joy_id[i]->GetDeviceState(sizeof(input_dinput_joy_state[i]), &input_dinput_joy_state[i]);
			if (rval != DI_OK) input_dinput_joy_id[i]->Acquire();
		}
	}
	
	return 0;
	
	// TODO: Figure out what to do with the rest of this function.
	
	//rval = lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState);
	
	//if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
	//	Restore_Input();

	//MouseX = MouseState.lX;
	//MouseY = MouseState.lY;
	
	// TODO: This is from Gens/Rerecording's hotkey remapping system.
#if 0
	int numInputButtons = GetNumHotkeys();
	for(int i = 0; i < numInputButtons; i++)
	{
		InputButton& button = s_inputButtons[i];

		BOOL pressed = button.diKey ? Check_Key_Pressed(button.diKey) : FALSE;

		if(button.virtKey || button.modifiers)
		{
			bool pressed2 = button.virtKey ? !!(GetAsyncKeyState(button.virtKey) & 0x8000) : true;

			pressed2 &= !(button.modifiers & MOD_CONTROL) == !(GetAsyncKeyState(VK_CONTROL) & 0x8000);
			pressed2 &= !(button.modifiers & MOD_SHIFT) == !(GetAsyncKeyState(VK_SHIFT) & 0x8000);
			pressed2 &= !(button.modifiers & MOD_ALT) == !(GetAsyncKeyState(VK_MENU) & 0x8000);
			pressed2 &= !(button.modifiers & MOD_WIN) == !((GetAsyncKeyState(VK_LWIN)|GetAsyncKeyState(VK_RWIN)) & 0x8000);

			if(!button.diKey)
				pressed = TRUE;

			if(!pressed2)
				pressed = FALSE;
		}

		if (button.alias)
			*button.alias = pressed;

		BOOL oldPressed = button.heldNow;
		button.heldNow = pressed;

		if (pressed && !oldPressed && button.eventID && !button.ShouldUseAccelerator())
			SendMessage(gens_window, WM_COMMAND, button.eventID, 0);
	}
#endif
}


/**
 * input_dinput_check_key_pressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return TRUE if the key is pressed; FALSE if the key is not pressed.
 */
BOOL input_dinput_check_key_pressed(uint16_t key)
{
	// If the key value is <256, it's a keyboard key.
	if (key < 0x100)
		return ((input_dinput_keys[key] & 0x80) ? TRUE : FALSE);
	
	// Joystick "key" check.
	
	// Determine which joystick we're looking for.
	int joyNum = ((key >> 8) & 0xF);
	
	// Check that this joystick exists.
	if (!input_dinput_joy_exists(joyNum))
		return FALSE;
	
	// Joystick exists. Check the state.
	switch (INPUT_JOYSTICK_GET_TYPE(key))
	{
		case INPUT_JOYSTICK_TYPE_AXIS:
			// Joystick axis.
			// TODO: Determine the correct axis order and the correct axis values.
			switch (key & 0xFF)
			{
				case 0:
					if (input_dinput_joy_state[joyNum].lX <= 0x3FFF)
						return TRUE;
					break;
				case 1:
					if (input_dinput_joy_state[joyNum].lX >= 0xC000)
						return TRUE;
					break;
				case 2:
					if (input_dinput_joy_state[joyNum].lY <= 0x3FFF)
						return TRUE;
					break;
				case 3:
					if (input_dinput_joy_state[joyNum].lY >= 0xC000)
						return TRUE;
					break;
				case 4:
					if (input_dinput_joy_state[joyNum].lZ <= 0x3FFF)
						return TRUE;
					break;
				case 5:
					if (input_dinput_joy_state[joyNum].lZ >= 0xC000)
						return TRUE;
					break;
				case 6:
					if (input_dinput_joy_state[joyNum].lRx <= 0x3FFF)
						return TRUE;
					break;
				case 7:
					if (input_dinput_joy_state[joyNum].lRx >= 0xC000)
						return TRUE;
					break;
				case 8:
					if (input_dinput_joy_state[joyNum].lRy <= 0x3FFF)
						return TRUE;
					break;
				case 9:
					if (input_dinput_joy_state[joyNum].lRy >= 0xC000)
						return TRUE;
					break;
				case 10:
					if (input_dinput_joy_state[joyNum].lRz <= 0x3FFF)
						return TRUE;
					break;
				case 11:
					if (input_dinput_joy_state[joyNum].lRz >= 0xC000)
						return TRUE;
					break;
				default:
					// Unknown axis.
					// TODO: Add support for rglSlider (formerly u-axis and v-axis).
					break;
			}
			
			break;
		
		case INPUT_JOYSTICK_TYPE_BUTTON:
			// Joystick button.
			if (input_dinput_joy_state[joyNum].rgbButtons[key & 0xFF])
				return TRUE;
			break;
		
		case INPUT_JOYSTICK_TYPE_POVHAT:
		{
			// Joystick POV hat.
			unsigned int povAngle = input_dinput_joy_state[joyNum].rgdwPOV[(key >> 2) & 0x3F];
			
			// A value of -1 or 65,535 indicates the POV hat switch is centered.
			if (LOWORD(povAngle) == 0xFFFF)
				return FALSE;
			
			// Check the angles based on the key value.
			switch (key & 0x03)
			{
				case INPUT_JOYSTICK_POVHAT_UP:
					if (povAngle >= 29250 || povAngle <= 6750)
						return TRUE;
					break;
				case INPUT_JOYSTICK_POVHAT_RIGHT:
					if (povAngle >= 2250 && povAngle <= 15750)
						return TRUE;
					break;
				case INPUT_JOYSTICK_POVHAT_DOWN:
					if (povAngle >= 11250 && povAngle <= 24750)
						return TRUE;
					break;
				case INPUT_JOYSTICK_POVHAT_LEFT:
					if (povAngle >= 20250 && povAngle <= 33750)
						return TRUE;
					break;
			}
			
			break;
		}
	}
	
	// Key is not pressed.
	return FALSE;
}


/**
 * input_dinput_set_cooperative_level(): Sets the cooperative level.
 * @param hWnd Window to set the cooperative level on.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_set_cooperative_level(HWND hWnd)
{
	// If no hWnd was specified, use the Gens window.
	if (!hWnd)
		hWnd = gens_window;
	
	if (!hWnd || !lpDIDKeyboard /*|| lpDIDMouse*/)
		return -1;
	
	HRESULT rval;
	//rval = lpDIDMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	rval = lpDIDKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (rval != DI_OK)
	{
		LOG_MSG(input, LOG_MSG_LEVEL_WARNING,
			"lpDIDKeyboard->SetCooperativeLevel() failed.");
		// TODO: Error handling code.
	}
	else
	{
		LOG_MSG(input, LOG_MSG_LEVEL_INFO,
			"lpDIDKeyboard->SetCooperativeLevel() succeeded.");
	}
	
	return 0;
}


/**
 * input_dinput_set_cooperative_level_joysticks(): Sets the cooperative level on joysticks.
 * @param hWnd Window to set the cooperative level on.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_set_cooperative_level_joysticks(HWND hWnd)
{
	// If no hWnd was specified, use the Gens window.
	if (!hWnd)
		hWnd = gens_window;
	
	HRESULT rval;
	for (int i = 0; i < MAX_JOYS; i++)
	{
		IDirectInputDevice2 *pJoyDevice = input_dinput_joy_id[i];
		if (!pJoyDevice)
			continue;
		
		rval = pJoyDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		
		if (rval != DI_OK)
		{
			LOG_MSG(input, LOG_MSG_LEVEL_WARNING,
				"SetCooperativeLevel() failed on joystick %d.", i);
			pJoyDevice->Release();
			input_dinput_joy_id[i] = NULL;
		}
		else
		{
			LOG_MSG(input, LOG_MSG_LEVEL_INFO,
				"SetCooperativeLevel() succeeded on joystick %d.", i);
		}
	}
	
	return 0;
}


/**
 * input_dinput_add_joysticks_to_listbox(): Enumerate joysticks and add them to a listbox.
 * @param lstBox Listbox to add the joystick information to.
 */
void input_dinput_add_joysticks_to_listbox(HWND lstBox)
{
	if (!lpDI || input_dinput_joystick_error)
		return;
	
	// Initialize the "Add Joysticks" counter.
	input_dinput_add_joysticks_count = 0;
	
	// Enumerate the joysticks to get their names.
	// TODO: Somehow add this to the first EnumDevices().
	lpDI->EnumDevices(DIDEVTYPE_JOYSTICK,
			  &input_dinput_callback_add_joysticks_to_listbox,
			  lstBox, DIEDFL_ATTACHEDONLY);
}


/**
 * input_dinput_callback_add_joysticks_to_listbox(): EnumDevices callback for adding joysticks to a listbox.
 * @param lpDIIJoy Joystick information.
 * @param pvRef Listbox to add the joystick information to.
 * @return DIENUM_CONTINUE to continue the enumeration; DIENUM_STOP to stop the enumeration.
 */
static BOOL CALLBACK input_dinput_callback_add_joysticks_to_listbox(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	TCHAR joy_name[64];
	
	// NOTE: _sntprintf() is the TCHAR version of snprintf().
	if (lpDIIJoy->tszProductName)
	{
		_sntprintf(joy_name, (sizeof(joy_name)/sizeof(TCHAR)),
				TEXT("Joystick %d: %s"),
				input_dinput_add_joysticks_count,
				lpDIIJoy->tszProductName);
	}
	else
	{
		_sntprintf(joy_name, (sizeof(joy_name)/sizeof(TCHAR)),
				TEXT("Joystick %d"),
				input_dinput_add_joysticks_count);
	}
	joy_name[(sizeof(joy_name)/sizeof(TCHAR))-1] = 0x00;
	
	// Add the joystick name to the listbox.
	ListBox_AddString((HWND)pvRef, joy_name);
	
	// Increment the joystick counter.
	input_dinput_add_joysticks_count++;
	
	// If the joystick counter exceeds the maximum number of joysticks, don't enumerate any more.
	if (input_dinput_add_joysticks_count >= MAX_JOYS)
		return DIENUM_STOP;
	
	// Next joystick.
	return DIENUM_CONTINUE;
}
