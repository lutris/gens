/**
 * GENS: Main loop. (Linux specific code)
 */

#include <sys/stat.h>
#include <stdio.h>
#include <SDL.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "gens.h"
#include "g_main.hpp"
#include "g_input.hpp"

GtkWidget *gens_window = NULL;

void SDL_Check_Joystick_Axis(SDL_Event *event);


/**
 * update_SDL_events(): Check for SDL events.
 */
void update_SDL_events(void)
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		/*
			case SDL_VIDEORESIZE:
				surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
				break;
		*/
			case SDL_KEYDOWN:
				Input_KeyDown(event.key.keysym.sym);
				break;
				
			case SDL_KEYUP:
				Input_KeyUp(event.key.keysym.sym);
				break;
			
			case SDL_JOYAXISMOTION:
				SDL_Check_Joystick_Axis(&event);
				break;
			
			case SDL_JOYBUTTONDOWN:
				joystate[0x10 + 0x100 * event.jbutton.which + event.jbutton.button] = 1;
				break;
			
			case SDL_JOYBUTTONUP:
				joystate[0x10 + 0x100 * event.jbutton.which + event.jbutton.button] = 0;
				break;
			
			case SDL_JOYHATMOTION:
				break;
			
			default:
				break;
		}
	}
}


void SDL_Check_Joystick_Axis(SDL_Event *event)
{
	if (event->jaxis.axis < 6)
	{
		if (event->jaxis.value < -10000)
		{
			if (event->jaxis.axis == 0)
			{
				joystate[0x100 * event->jaxis.which + 0x3] = 1;
				joystate[0x100 * event->jaxis.which + 0x4] = 0;
			}
			else if (event->jaxis.axis == 1)
			{
				joystate[0x100 * event->jaxis.which + 0x1] = 1;
				joystate[0x100 * event->jaxis.which + 0x2] = 0;
			}
			else if (event->jaxis.axis == 2)
			{
				joystate[0x100 * event->jaxis.which + 0x7] = 1;
				joystate[0x100 * event->jaxis.which + 0x8] = 0;
			}
			else if (event->jaxis.axis == 3)
			{
				joystate[0x100 * event->jaxis.which + 0x5] = 1;
				joystate[0x100 * event->jaxis.which + 0x6] = 0;
			}
			else if (event->jaxis.axis == 4)
			{
				joystate[0x100 * event->jaxis.which + 0xB] = 1;
				joystate[0x100 * event->jaxis.which + 0xC] = 0;
			}
			else if (event->jaxis.axis == 5)
			{
				joystate[0x100 * event->jaxis.which + 0x9] = 1;
				joystate[0x100 * event->jaxis.which + 0xA] = 0;
			}
		}
		else if (event->jaxis.value > 10000)
		{
			if (event->jaxis.axis == 0)
			{
				joystate[0x100 * event->jaxis.which + 0x3] = 0;
				joystate[0x100 * event->jaxis.which + 0x4] = 1;
			}
			else if (event->jaxis.axis == 1)
			{
				joystate[0x100 * event->jaxis.which + 0x1] = 0;
				joystate[0x100 * event->jaxis.which + 0x2] = 1;
			}
			else if (event->jaxis.axis == 2)
			{
				joystate[0x100 * event->jaxis.which + 0x7] = 0;
				joystate[0x100 * event->jaxis.which + 0x8] = 1;
			}
			else if (event->jaxis.axis == 3)
			{
				joystate[0x100 * event->jaxis.which + 0x5] = 0;
				joystate[0x100 * event->jaxis.which + 0x6] = 1;
			}
			else if (event->jaxis.axis == 4)
			{
				joystate[0x100 * event->jaxis.which + 0xB] = 0;
				joystate[0x100 * event->jaxis.which + 0xC] = 1;
			}
			else if (event->jaxis.axis == 5)
			{
				joystate[0x100 * event->jaxis.which + 0x9] = 0;
				joystate[0x100 * event->jaxis.which + 0xA] = 1;
			}
		}
		else
		{
			if (event->jaxis.axis == 0)
			{
				joystate[0x100 * event->jaxis.which + 0x4] = 0;
				joystate[0x100 * event->jaxis.which + 0x3] = 0;
			}
			else if (event->jaxis.axis == 1)
			{
				joystate[0x100 * event->jaxis.which + 0x2] = 0;
				joystate[0x100 * event->jaxis.which + 0x1] = 0;
			}
			else if (event->jaxis.axis == 2)
			{
				joystate[0x100 * event->jaxis.which + 0x8] = 0;
				joystate[0x100 * event->jaxis.which + 0x7] = 0;
			}
			else if (event->jaxis.axis == 3)
			{
				joystate[0x100 * event->jaxis.which + 0x6] = 0;
				joystate[0x100 * event->jaxis.which + 0x5] = 0;
			}
			else if (event->jaxis.axis == 4)
			{
				joystate[0x100 * event->jaxis.which + 0xC] = 0;
				joystate[0x100 * event->jaxis.which + 0xB] = 0;
			}
			else if (event->jaxis.axis == 5)
			{
				joystate[0x100 * event->jaxis.which + 0xA] = 0;
				joystate[0x100 * event->jaxis.which + 0x9] = 0;
			}
		}
	}
}

/**
 * GENS_Default_Save_Path(): Create the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	strncpy (buf, getenv ("HOME"), n);
	strcat (buf, "/.gens/");
}

/**
 * GENS_Create_Default_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	mkdir(dir, 0700);
}
