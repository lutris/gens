/**
 * Gens: Input class - DirectInput
 */

#ifndef GENS_INPUT_DINPUT_HPP
#define GENS_INPUT_DINPUT_HPP

#include "input.hpp"

// DirectInput 5 is required for joystick support.
#define DIRECTINPUT_VERSION 0x0500

#include <windows.h>
#include <dinput.h>
//#include <mmsystem.h>

// WINE headers are missing these definitions...
#ifndef IID_IDirectInputDevice2
#ifdef UNICODE
#define IID_IDirectInputDevice2 IID_IDirectInputDevice2W
#else
#define IID_IDirectInputDevice2 IID_IDirectInputDevice2A
#endif /* UNICODE */
#endif /* IID_IDirectInputDevice2 */

#ifndef IDirectInputDevice2
#ifdef UNICODE
#define IDirectInputDevice2 IDirectInputDevice2W
#else
#define IDirectInputDevice2 IDirectInputDevice2A
#endif /* UNICODE */
#endif /* IDirectInputDevice2 */

#ifndef IDirectInputDevice2Vtbl
#ifdef UNICODE
#define IDirectInputDevice2Vtbl IDirectInputDevice2WVtbl
#else
#define IDirectInputDevice2Vtbl IDirectInputDevice2AVtbl
#endif /* UNICODE */
#endif /* IDirectInputDevice2Vtbl */

#define MAX_JOYS 8

class Input_DInput : public Input
{
	public:
		Input_DInput();
		~Input_DInput();
		
		// Update the input subsystem
		void update(void);
		
		// Check if the specified key is pressed.
		bool checkKeyPressed(unsigned int key);
		
		// Get a key. (Used for controller configuration.)
		unsigned int getKey(void);
		
		// InitJoystick callback
		static BOOL CALLBACK InitJoystick(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
		BOOL InitJoystick_int(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
		
	protected:
		// Functions required by the Input class.
		bool joyExists(int joyNum);
		
		// Number of joysticks connected
		int m_numJoysticks;
		
		// DirectInput variables
		LPDIRECTINPUT lpDI;
		LPDIRECTINPUTDEVICE lpDIDKeyboard;
		LPDIRECTINPUTDEVICE lpDIDMouse;
		IDirectInputDevice2 *m_joyID[MAX_JOYS];
		DIJOYSTATE m_joyState[MAX_JOYS];
		
		// DirectInput Keys array
		unsigned char m_DIKeys[256];
		
		void restoreInput(void);
		
		// Cooperative level (Win32)
		void setCooperativeLevel(void);
		
};

#endif
