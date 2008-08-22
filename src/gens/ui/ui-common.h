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
	ROMFile		= 0,
	SavestateFile	= 1,
	CDImage		= 2,
	ConfigFile	= 3,
	GYMFile		= 4,
} FileFilterType;

// Include the OS-specific common UI file.
#if (defined(__linux__))
#include "ui-common_gtk.h"
#elif (defined(__WIN32__))
#error TODO: Add Win32 support.
#else
#error Unsupported operating system.
#endif


#ifdef __cplusplus
}
#endif

#endif
