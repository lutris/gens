/**
 * Gens: Input base class.
 */


#ifndef GENS_INPUT_HPP
#define GENS_INPUT_HPP

// OS-specific includes.
#if (defined(__linux__))
#include "input_sdl_keys.h"
#elif (defined(__WIN32__))
#include "input_win32_keys.h"
#else
#error Unsupported operating system.
#endif

// Controller key mapping.
struct KeyMap
{
	unsigned int Start, Mode;
	unsigned int A, B, C;
	unsigned int X, Y, Z;
	unsigned int Up, Down, Left, Right;
};

// Default key mapping.
extern const KeyMap keyDefault[8];

class Input
{
	public:
		Input();
		virtual ~Input();
		
		// Update the input subsystem
		virtual void update(void) = 0;
		
		// Check if the specified key is pressed.
		bool checkKeyPressed(unsigned int key);
		
		// Update the controller bitfields.
		void updateControllers(void);
		
		// Get a key. (Used for controller configuration.)
		virtual unsigned int getKey(void) = 0;
		
		// Key mappings
		// TODO: Make these protected.
		KeyMap m_keyMap[8];
		
	protected:
		bool m_keys[1024];
		bool m_joyState[0x530];
		
		// Functions that need to be implemented by child classes.
		virtual bool joyExists(int joyNum) = 0;
};

#endif
