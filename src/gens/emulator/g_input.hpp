/**
 * GENS: Input handler.
 */

#ifndef GENS_G_INPUT_HPP
#define GENS_G_INPUT_HPP

#ifdef __cplusplus
extern "C" {
#endif

// OS-specific includes.
#if (defined(__linux__))
#include "input/input_sdl_keys.h"
#elif (defined(__WIN32__))
#error TODO: Add Win32 support.
#else
#error Unsupported operating system.
#endif


// Controller keymapping.
struct K_Def
{
	unsigned int Start, Mode;
	unsigned int A, B, C;
	unsigned int X, Y, Z;
	unsigned int Up, Down, Left, Right;
};
extern unsigned char Kaillera_Keys[16];
extern struct K_Def Keys_Def[8];
extern const struct K_Def Keys_Default[8];


void Input_KeyDown(int key);
void Input_KeyUp(int key);

#ifdef __cplusplus
}
#endif

#endif
