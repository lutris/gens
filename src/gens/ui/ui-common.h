/**
 * GENS: Common UI functions.
 */
 
#ifndef UI_COMMON_H
#define UI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	AnyFile		= 0,
	ROMFile		= 1,
	SavestateFile	= 2,
	CDImage		= 3,
	ConfigFile	= 4,
	GYMFile		= 5,
} FileFilterType;

// Include the OS-specific common UI file.
#if (defined(__linux__))
#include "ui-common_gtk.h"
#elif (defined(__WIN32__))
#error TODO: Add Win32 support.
#else
#error Unsupported operating system.
#endif


void UI_Set_Window_Title_Idle(void);
void UI_Set_Window_Title_Game(const char* system, const char* game);
void UI_Set_Window_Title_Init(const char* system, int reinit);


#ifdef __cplusplus
}
#endif

#endif
