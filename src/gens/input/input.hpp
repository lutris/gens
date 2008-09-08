/**
 * Gens: Input base class.
 */


#ifndef GENS_INPUT_HPP
#define GENS_INPUT_HPP

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
		
		// Check if the specified key is pressed.
		bool checkKeyPressed(unsigned int key);
		
		// Update the controller bitfields.
		void updateControllers(void);
		
		// Get a key. (Used for controller configuration.)
		virtual unsigned int getKey(void) = 0;
		
		// Key Up/Down functions.
		void keyDown(unsigned int key)
		{
			if (key > 1024)
				return;
			m_keys[key] = true;
		}
		void keyUp(unsigned int key)
		{
			if (key > 1024)
				return;
			m_keys[key] = false;
		}
		
		// Key mappings
		// TODO: Make these protected.
		KeyMap m_keyMap[8];
		
		// Joystick state
		// TODO: Make this protected.
		bool m_joyState[0x530];
	
	protected:
		bool m_keys[1024];
		
		// Functions that need to be implemented by child classes.
		virtual bool joyExists(int joyNum) = 0;
};

#endif
