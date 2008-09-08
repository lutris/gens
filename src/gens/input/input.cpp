/**
 * Gens: Input base class.
 */


#include <string.h>

#include "input.hpp"
#include "emulator/g_input.hpp"
#include "gens_core/io/io.h"


Input::Input()
{
	// Initialize m_keys and m_joyState.
	memset(m_keys, 0x00, sizeof(m_keys));
	memset(m_joyState, 0x00, sizeof(m_joyState));
}


Input::~Input()
{
}


/**
 * checkKeyPressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return True if the key is pressed.
 */
bool Input::checkKeyPressed(unsigned int key)
{
	// If the key value is <1024, it's a keyboard key.
	if (key < 1024)
		return m_keys[key];
	
	// Joystick "key" check.
	
	// Determine which joystick we're looking for.
	int joyNum = ((key >> 8) & 0xF);
	
	// Only 6 joysticks are supported right now.
	if (joyNum >= 6)
		return false;
	
	// Check that this joystick exists.
	if (!joyExists(joyNum))
		return false;
	
	// Joystick exists. Check the state.
	if (key & 0x80)
	{
		// Joystick POV
		// TODO: This doesn't seem to be implemented in Gens/Linux for some reason...
		/*
		switch (key & 0xF)
		{
			case 1:
				//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 0) return(1); break;
				//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 0) return(1); break;
			case 2:
				//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
				//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
			case 3:
				//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
				//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
			case 4:
				//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
				//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
			default:
				break;
		}
		*/
	}
	else if (key & 0x70)
	{
		// Joystick buttons
		if (m_joyState[0x10 + (0x100 * joyNum) + ((key & 0xFF) - 0x10)])
			return 1;
	}
	else
	{
		// Joystick axes
		if (((key & 0xF) >= 1) && ((key & 0xF) <= 12))
		{
			if (m_joyState[(0x100 * joyNum) + (key & 0xF)])
				return true;
		}
	}
	
	// Key is not pressed.
	return false;
}


#define CHECK_BUTTON(player, ctrl, button, mask)		\
	if (checkKeyPressed(Keys_Def[player].button)) 		\
		Controller_ ## ctrl ## _Buttons &= ~mask;	\
	else Controller_ ## ctrl ## _Buttons |= mask;

#define CHECK_DIR(player, ctrl)						\
	if (checkKeyPressed(Keys_Def[player].Up)) 			\
	{					   			\
		Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_UP;	\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_DOWN;	\
	}								\
	else								\
	{								\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_UP;	\
		CHECK_BUTTON(player, ctrl, Down, CONTROLLER_DOWN)	\
	}								\
	if (checkKeyPressed(Keys_Def[player].Left))			\
	{					   			\
		Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_LEFT;	\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_RIGHT;	\
	}								\
	else								\
	{								\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_LEFT;	\
		CHECK_BUTTON(player, ctrl, Right, CONTROLLER_RIGHT)	\
	}

#define CHECK_ALL_BUTTONS(player, ctrl)					\
	CHECK_BUTTON(player, ctrl, Start, CONTROLLER_START)		\
	CHECK_BUTTON(player, ctrl, A, CONTROLLER_A)			\
	CHECK_BUTTON(player, ctrl, B, CONTROLLER_B)			\
	CHECK_BUTTON(player, ctrl, C, CONTROLLER_C)			\
									\
	if (Controller_ ## ctrl ## _Type & 1)				\
	{								\
		CHECK_BUTTON(player, ctrl, Mode, CONTROLLER_MODE)	\
		CHECK_BUTTON(player, ctrl, X, CONTROLLER_X)		\
		CHECK_BUTTON(player, ctrl, Y, CONTROLLER_Y)		\
		CHECK_BUTTON(player, ctrl, Z, CONTROLLER_Z)		\
	}

#define CHECK_PLAYER_PAD(player, ctrl)					\
	CHECK_DIR(player, ctrl)						\
	CHECK_ALL_BUTTONS(player, ctrl)

/**
 * updateControllers(): Update the controller bitfields.
 */
void Input::updateControllers(void)
{
	CHECK_PLAYER_PAD(0, 1);
	CHECK_PLAYER_PAD(1, 2);
	
	if (Controller_1_Type & 0x10)
	{
		// TEAMPLAYER PORT 1
		CHECK_PLAYER_PAD(2, 1B);
		CHECK_PLAYER_PAD(3, 1C);
		CHECK_PLAYER_PAD(4, 1D);
	}
	
	if (Controller_2_Type & 0x10)
	{
		// TEAMPLAYER PORT 2
		CHECK_PLAYER_PAD(5, 2B);
		CHECK_PLAYER_PAD(6, 2C);
		CHECK_PLAYER_PAD(7, 2D);
	}
}
