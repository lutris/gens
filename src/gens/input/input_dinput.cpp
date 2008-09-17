/**
 * Gens: Input class - SDL
 */


#include "input_dinput.hpp"
#include "input_win32_keys.h"

#include "emulator/g_input.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.h"


const struct KeyMap keyDefault[8] =
{
	// Player 1
	{DIK_RETURN, DIK_RSHIFT,
	DIK_A, DIK_S, DIK_D,
	DIK_Z, DIK_X, DIK_C,
	DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT},
	
	// Players 1B, 1C, 1D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	// Player 2
	{DIK_U, DIK_T,
	DIK_K, DIK_L, DIK_M,
	DIK_I, DIK_O, DIK_P,
	DIK_Y, DIK_H, DIK_H, DIK_J},
	
	// Players 2B, 2C, 2D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


// Axis values.
static const unsigned char JoyAxisValues[2][6] =
{
	// axis value < -10,000
	{0x03, 0x01, 0x07, 0x05, 0x0B, 0x09},
	
	// axis value > 10,000
	{0x04, 0x02, 0x08, 0x06, 0x0C, 0x0A},
};


Input_DInput::Input_DInput()
{
	// TODO: HINSTANCE ghInstance; HWND Gens_hWnd
	
	int i;
	HRESULT rval;
	
	rval = DirectInputCreate(ghInstance, DIRECTINPUT_VERSION, &lpDI, NULL);
	if (rval != DI_OK)
	{
		MessageBox(hWnd, "DirectInput failed ...You must have DirectX 5", "Error", MB_OK);
		// TODO: Error handling.
		return;
	}
	
	m_numJoysticks = 0;
	for (i = 0; i < MAX_JOYS; i++)
	{
		Joy_ID[i] = NULL;
	}
	
	m_CallbackHandleObject = this;
	rval = lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, &InitJoystick, Gens_hWnd, DIEDFL_ATTACHEDONLY);
	if (rval ! DI_OK)
	{
		// TODO: Error handling.
		return;
	}
	
	//rval = lpDI->CreateDevice(GUID_SysMouse, &lpDIDMouse, NULL);
	rval = lpDI->CreateDevice(GUID_SysKeyboard, &lpDIDKeyboard, NULL);
	if (rval != DI_OK)
	{
		// TODO: Error handling.
		return;
	}
	
	//rval = lpDIDMouse->SetCooperativeLevel(Gens_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	rval = lpDIDKeyboard->SetCooperativeLevel(Gens_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (rval != DI_OK)
	{
		// TODO: Error handling.
		return;
	}
	
	//rval = lpDIDMouse->SetDataFormat(&c_dfDIMouse);
	rval = lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (rval != DI_OK)
	{
		// TODO: Error handling.
		return;
	}
	
	//rval = lpDIDMouse->Acquire();
	for(i = 0; i < 10; i++)
	{
		rval = lpDIDKeyboard->Acquire();
		if (rval == DI_OK)
			break;
		GensUI::sleep(10);
	}
	
	// Clear the DirectInput keys array.
	memset(m_DIKeys, 0x00, sizeof(m_DIKeys));
	
	return;
}


Input_DInput::~Input_DInput()
{
	// If any joysticks were opened, close them.
	int i;
	if (!lpDI)
		return;
	
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
	
	for (i = 0; i < MAX_JOYS; i++)
	{
		if (Joy_ID[i])
		{
			Joy_ID[i]->Unacquire();
			Joy_ID[i]->Release();
		}
	}
	
	m_numJoysticks = 0;
	lpDI->Release();
	lpDI = NULL;
}


BOOL CALLBACK Input_DInput::InitJoystick(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	return m_CallbackHandleObject->InitJoystick_int(lpDIIJoy, pvRef);
}


BOOL CALLBACK Input_DInput::InitJoystick_int(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	HRESULT rval;
	LPDIRECTINPUTDEVICE	lpDIJoy;
	DIPROPRANGE diprg;
	int i;
 
	if (m_numJoysticks >= MAX_JOYS)
		return DIENUM_STOP;
		
	Joy_ID[m_numJoysticks] = NULL;

	rval = lpDI->CreateDevice(lpDIIJoy->guidInstance, &lpDIJoy, NULL);
	if (rval != DI_OK)
	{
		MessageBox(Gens_hWnd, "IDirectInput::CreateDevice FAILED", "erreur joystick", MB_OK);
		return(DIENUM_CONTINUE);
	}

	rval = lpDIJoy->QueryInterface(IID_IDirectInputDevice2, (void **)&Joy_ID[m_numJoysticks]);
	lpDIJoy->Release();
	if (rval != DI_OK)
	{
		MessageBox(Gens_hWnd, "IDirectInputDevice2::QueryInterface FAILED", "erreur joystick", MB_OK);
		Joy_ID[m_numJoysticks] = NULL;
		return(DIENUM_CONTINUE);
	}

	rval = Joy_ID[m_numJoysticks]->SetDataFormat(&c_dfDIJoystick);
	if (rval != DI_OK)
	{
		MessageBox(Gens_hWnd, "IDirectInputDevice::SetDataFormat FAILED", "erreur joystick", MB_OK);
		Joy_ID[m_numJoysticks]->Release();
		Joy_ID[m_numJoysticks] = NULL;
		return(DIENUM_CONTINUE);
	}

	rval = Joy_ID[m_numJoysticks]->SetCooperativeLevel((HWND)pvRef, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	if (rval != DI_OK)
	{ 
		MessageBox(Gens_hWnd, "IDirectInputDevice::SetCooperativeLevel FAILED", "erreur joystick", MB_OK);
		Joy_ID[m_numJoysticks]->Release();
		Joy_ID[m_numJoysticks] = NULL;
		return(DIENUM_CONTINUE);
	}
 
	diprg.diph.dwSize = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj = DIJOFS_X;
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000; 
	diprg.lMax = +1000;
 
	rval = Joy_ID[m_numJoysticks]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT)) 
		MessageBox(Gens_hWnd, "IDirectInputDevice::SetProperty() (X-Axis) FAILED", "erreur joystick", MB_OK);

	diprg.diph.dwSize = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj = DIJOFS_Y;
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000; 
	diprg.lMax = +1000;
 
	rval = Joy_ID[m_numJoysticks]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT)) 
		MessageBox(Gens_hWnd, "IDirectInputDevice::SetProperty() (Y-Axis) FAILED", "erreur joystick", MB_OK);

	for(i = 0; i < 10; i++)
	{
		rval = Joy_ID[m_numJoysticks]->Acquire();
		if (rval == DI_OK)
			break;
		GensUI::sleep(10);
	}

	m_numJoysticks++;

	return(DIENUM_CONTINUE);
}


void Input_DInput::restoreInput(void)
{
	//lpDIDMouse->Acquire();
	lpDIDKeyboard->Acquire();
}


/**
 * joyExists(): Check if the specified joystick exists.
 * @param joyNum Joystick number.
 * @return true if the joystick exists; false if it does not exist.
 */
bool Input_DInput::joyExists(int joyNum)
{
	if (joyNum < 0 || joyNum >= MAX_JOYS)
		return false;
	
	if (Joy_ID[joyNum])
		return true;
	
	// Joystick does not exist.
	return false;
}


/**
 * getKey(): Get a key. (Used for controller configuration.)
 * @return Key value.
 */
unsigned int Input_DInput::getKey(void)
{
	InputButton tempButton;
	tempButton.diKey = 0;
	
	int i, j, joyIndex;
	
	bool prevReady = false;
	
	int prevMod;
	BOOL prevDiKeys[256];
	BOOL prevVirtKeys[256];
	BOOL prevJoyKeys[256];
	
	int curMod;
	BOOL curDiKeys[256];
	BOOL curVirtKeys[256];
	BOOL curJoyKeys[256];
	
	while (true)
	{
		// Compute the current state of all buttons.
		update();
		
		// Current state of modifier keys.
		curMod = 0;
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			curMod |= MOD_CONTROL;
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			curMod |= MOD_SHIFT;
		if (GetAsyncKeyState(VK_MENU) & 0x8000)
			curMod |= MOD_ALT;
		if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000)
			curMod |= MOD_WIN;
		
		// Current state of virtual Windows keys
		for(i = 0; i < 256; i++)
			curVirtKeys[i] = (GetAsyncKeyState(i) & 0x8000);
		
		// Current state of DirectInput keys
		for(i = 0; i < 256; i++)
			curDiKeys[i] = (m_DIKeys[i] & 0x80);
		
		// Current state of recognized buttons on joypad
		joyIndex = 0;
		for (i = 0; i < m_numJoysticks; i++)
		{
			if (!joyExists(i))
				continue;
			
			curJoyKeys[joyIndex++] = (Joy_State[i].lY < -500);
			curJoyKeys[joyIndex++] = (Joy_State[i].lY > +500);
			curJoyKeys[joyIndex++] = (Joy_State[i].lX < -500);
			curJoyKeys[joyIndex++] = (Joy_State[i].lX > +500);
			
			for (j = 0; j < 4; j++)
			{
				curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 0);
				curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 9000);
				curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 18000);
				curJoyKeys[joyIndex++] = (Joy_State[i].rgdwPOV[j] == 27000);
			}
			
			for (j = 0; j < 32; j++)
			{
				curJoyKeys[joyIndex++] = (Joy_State[i].rgbButtons[j]);
			}
			
			curJoyKeys[joyIndex++] = (Joy_State[i].lRx < 0x3FFF);
			curJoyKeys[joyIndex++] = (Joy_State[i].lRx > 0xBFFF);
			curJoyKeys[joyIndex++] = (Joy_State[i].lRy < 0x3FFF);
			curJoyKeys[joyIndex++] = (Joy_State[i].lRy > 0xBFFF);
			curJoyKeys[joyIndex++] = (Joy_State[i].lZ < 0x3FFF);
			curJoyKeys[joyIndex++] = (Joy_State[i].lZ > 0xBFFF);
		}
		
		// Compare buttons against the previous state to determine
		// what is pressed now that wasn't already pressed before.
		
		if (prevReady)
		{
			// Check for new virtual key presses.
			for (i = 1; i < 255; i++)
			{
				if (curVirtKeys[i] && !prevVirtKeys[i] /*&& allowVirtual*/)
				{
					if (i == VK_CONTROL || i == VK_SHIFT || i == VK_MENU ||
					    i == VK_LWIN || i == VK_RWIN || i == VK_LSHIFT ||
					    i == VK_RSHIFT || i == VK_LCONTROL || i == VK_RCONTROL ||
					    i == VK_LMENU || i == VK_RMENU)
					{
						// Don't allow modifier keys here.
						continue;
					}
					button.SetAsVirt(i, curMod);
					return button.diKey;
				}
				
				// Check for new DirectInput key presses.
				for (i = 1; i < 255; i++)
				{
					if (curDiKey[i] && !prevDiKeys[i])
					{
						if (/*allowVirtual &&*/ (i == DIK_LWIN || i == DIK_RWIN ||
						    i == DIK_LSHIFT || i == DIK_RSHIFT || i == DIK_LCONTROL ||
						    i == DIK_RCONTROL || i == DIK_LMENU || i == DIK_RMENU))
						{
							// Don't allow modifier keys here.
							continue;
						}
						button.SetAsDIK(i, curMod);
						return button.diKey;
					}
				}
				
				// Check for modifier key releases.
				// This allows a modifier key to be used as a hotkey on its own, as some people like to do.
				if (!curMod && prevMod /*&& allowVirtual*/)
				{
					button.SetAsVirt(VK_NONE, prevMod);
					return button.diKey;
				}
				
				// Check for new recognized joypad button presses.
				for (int index = 0; index < joyIndex; index++)
				{
					if (curJoyKeys[index] && !prevJoyKeys[index])
					{
						int joyIndex2 = 0;
						for (i = 0; i < m_numJoysticks; i++)
						{
							if (!joyExists(i))
								continue;
							
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x1, curMod);
								return button.diKey;
							}
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x2, curMod);
								return button.diKey;
							}
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x3, curMod);
								return button.diKey;
							}
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x4, curMod);
								return button.diKey;
							}
							
							for (j = 0; j < 4; j++)
							{
								if (index == joyIndex2++)
								{
									button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x1, curMod);
									return button.diKey;
								}
								if (index == joyIndex2++)
								{
									button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x2, curMod);
									return button.diKey;
								}
								if (index == joyIndex2++)
								{
									button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x3, curMod);
									return button.diKey;
								}
								if (index == joyIndex2++)
								{
									button.SetAsDIK(0x1080 + (0x100 * i) + (0x10 * j) + 0x4, curMod);
									return button.diKey;
								}
							}
							
							for (j = 0; j < 32; j++)
							{
								if (index == joyIndex2++)
								{
									button.SetAsDIK(0x1010 + (0x100 * i) + j, curMod);
									return button.diKey;
								}
							}
							
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x5, curMod);
								return button.diKey;
							}
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x6, curMod);
								return button.diKey;
							}
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x7, curMod);
								return button.diKey;
							}
							if (index == joyIndex2++)
							{
								button.SetAsDIK(0x1000 + (0x100 * i) + 0x8, curMod);
								return button.diKey;
							}
						}
					}
				}
			}
		}
		
		// Update previous state.
		memcpy(prevVirtKeys, curVirtKeys, sizeof(prevVirtKeys));
		memcpy(prevDiKeys, curDiKeys, sizeof(prevDiKeys));
		memcpy(prevJoyKeys, curJoyKeys, sizeof(prevJoyKeys));
		prevMod = curMod;
		prevReady = true;
	}
}


/**
 * update(): Update the input subsystem.
 */
void Input_DInput::update(void)
{
	//DIMOUSESTATE MouseState;
	HRESULT rval;
	int i;
	
	rval = lpDIDKeyboard->GetDeviceState(256, &m_DIKeys);
	
	// HACK because DirectInput is totally wacky about recognizing the PAUSE/BREAK key
	// still not perfect with this, but at least it goes above a 25% success rate
	if(GetAsyncKeyState(VK_PAUSE)) // normally this should have & 0x8000, but apparently this key is too special for that to work
		Keys[0xC5] |= 0x80;
	
	if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
		Restore_Input();
	
	for (i = 0; i < m_numJoysticks; i++)
	{
		if (Joy_ID[i])
		{
			Joy_ID[i]->Poll();
			rval = Joy_ID[i]->GetDeviceState(sizeof(Joy_State[i]), &Joy_State[i]);
			if (rval != DI_OK) Joy_ID[i]->Acquire();
		}
	}
	
	//rval = lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState);
	
	//if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
	//	Restore_Input();

	//MouseX = MouseState.lX;
	//MouseY = MouseState.lY;
	
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
			SendMessage(Gens_hWnd, WM_COMMAND, button.eventID, 0);
	}
}


/**
 * checkKeyPressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return True if the key is pressed.
 */
bool Input_DInput::checkKeyPressed(unsigned int key)
{
	// If the key value is <256, it's a keyboard key.
	if (key < 0x100)
		return (m_DIKeys[key] & 0x80);
	
	// Joystick "key" check.
	
	// Determine which joystick we're looking for.
	int joyNum = ((key >> 8) & 0xF);
	
	// Check that this joystick exists.
	if (!joyExists(joyNum))
		return false;
	
	// Joystick exists. Check the state.
	if (key & 0x80)
	{
		// Joystick POV
		int value = Joy_State[joyNum].rgdwPOV[(key >> 4) & 3];
		if (value == -1)
			return false;
		
		switch (key & 0xF)
		{
			case 1:
				if (value >= 29250 || value <= 6750)
					return true;
				break;
			case 2:
				if (value >= 2250 && value <= 15750)
					return true;
			case 3:
				if (value >= 11250 && value <= 24750)
					return true;
				break;
			case 4:
				if (value >= 20250 && value <= 33750)
					return true;
				break;
		}
	}
	else if (key & 0x70)
	{
		// Joystick buttons
		if (m_joyState[joyNum].rgbButtons[(key & 0xFF) - 0x10)])
			return true;
	}
	else
	{
		// Joystick axes
		switch (key & 0xF)
		{
			case 1:
				if (Joy_State[joyNum].lY < -500)
					return true;
				break;
			case 2:
				if (Joy_State[joyNum].lY > +500)
					return true;
				break;
			case 3:
				if (Joy_State[joyNum].lX < -500)
					return true;
				break;
			case 4:
				if (Joy_State[joyNum].lX > +500)
					return true;
				break;
			case 5:
				if (Joy_State[joyNum].lRx < 0x3FFF)
					return true;
				break;
			case 6:
				if (Joy_State[joyNum].lRx > 0xBFFF)
					return true;
				break;
			case 7:
				if (Joy_State[joyNum].lRy < 0x3FFF)
					return true;
				break;
			case 8:
				if (Joy_State[joyNum].lRy > 0xBFFF)
					return true;
				break;
			case 9:
				if (Joy_State[joyNum].lZ < 0x3FFF)
					return true;
				break;
			case 10:
				if (Joy_State[joyNum].lZ > 0xBFFF)
					return true;
				break;
		}
	}
	
	// Key is not pressed.
	return false;
}
