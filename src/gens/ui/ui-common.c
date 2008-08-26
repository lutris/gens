/**
 * GENS: Common UI functions.
 */


#include "gens.h"
#include "ui-common.h"


/**
 * UI_Set_Window_Title_Idle(): Set the window title to "Idle".
 */
void UI_Set_Window_Title_Idle(void)
{
	UI_Set_Window_Title(GENS_APPNAME " - Idle");
}


/**
 * UI_Set_Window_Title_Game(): Set the window title to the system name, followed by the game name.
 * @param system System name.
 * @param game Game name.
 */
void UI_Set_Window_Title_Game(const char* systemName, const char* gameName)
{
	char title[128];
	char condGameName[64];
	int gni, cgni = 0;
	
	// Condense the game name by removing excess spaces.
	for (gni = 0; gni < 64; gni++)
	{
		if (cgni != 0 && condGameName[cgni - 1] == ' ' && gameName[gni] == ' ')
			continue;
		else if (cgni == 0 && gameName[gni] == ' ')
			continue;
		condGameName[cgni] = gameName[gni];
		cgni++;
	}
	condGameName[cgni] = 0x00;
	
	// Create the title.
	sprintf(title, GENS_APPNAME " - %s: %s", systemName, condGameName);
	
	// Set the title.
	UI_Set_Window_Title(title);
}


/**
 * UI_Set_Window_Title_Init(): Set the window title to the system name, followed by "Initializing, please wait..."
 * @param system System name.
 * @param reinit If non-zero, sets the title to "Re-initializing" instead of "Initializing".
 */
void UI_Set_Window_Title_Init(const char* system, int reinit)
{
	char title[128];
	
	// Create the title.
	sprintf(title, GENS_APPNAME " - %s: %s, please wait...", system,
		(reinit ? "Reinitializing" : "Initializing"));
	
	// Set the title.
	UI_Set_Window_Title(title);
}
