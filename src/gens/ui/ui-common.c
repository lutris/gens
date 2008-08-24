/**
 * GENS: Common UI functions.
 */

#include "ui-common.h"


// Application name.
#define GENS_APPNAME "Gens"


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
void UI_Set_Window_Title_Game(const char* system, const char* game)
{
	char title[128];
	
	// Create the title.
	sprintf(title, GENS_APPNAME " - %s: %s", system, game);
	
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
