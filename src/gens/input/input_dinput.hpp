/**
 * Gens: Input class - DirectInput
 */

#ifndef GENS_INPUT_DINPUT_HPP
#define GENS_INPUT_DINPUT_HPP

#include "input.hpp"

// DirectInput 5 is required for joystick support.
#define DIRECTINPUT_VERSION 0x0500

#include <dinput.h>
//#include <mmsystem.h>

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
		BOOL CALLBACK InitJoystick_int(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef);
		
	protected:
		// Functions required by the Input class.
		bool joyExists(int joyNum);
		
		// Check an SDL joystick axis.
		void checkJoystickAxis(SDL_Event *event);
		
		// Number of joysticks connected
		int m_numJoysticks;
		
		// DirectInput variables
		LPDIRECTINPUT lpDI;
		LPDIRECTINPUTDEVICE lpDIDKeyboard;
		LPDIRECTINPUTDEVICE lpDIDMouse;
		static IDirectInputDevice2 *Joy_ID[MAX_JOYS] = {NULL};
		static DIJOYSTATE Joy_State[MAX_JOYS] = {{0}};
		
		// DirectInput Keys array
		unsigned char m_DIKeys[256];
		
		// Input_DInput being handled by InitJoystick at the moment.
		// TODO: This is a REALLY bad hack.
		Input_DInput *m_CallbackHandleObject;
		
		void restoreInput(void);
};

#endif
