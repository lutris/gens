/**
 * Gens: Input base class.
 */


#ifndef GENS_INPUT_HPP
#define GENS_INPUT_HPP

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
	
	protected:
		bool m_keys[1024];
		bool m_joyState[0x530];
		
		// Functions that need to be implemented by child classes.
		virtual bool joyExists(int joyNum) = 0;
};

#endif
