#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "port/port.h"
#include "g_sdlinput.h"
#include "gens_core/io/io.h"
#include "emulator/g_main.h"
#include "emulator/g_input.h"


#include "gtk-misc.h"
#include "gens/gens_window.h"


//char Phrase[1024];
static int Nb_Joys = 0;
long MouseX, MouseY;

static int gdk_to_sdl_keyval(int);

unsigned char Keys[1024];

unsigned char Kaillera_Keys[16];
unsigned char joystate[0x530];

struct SDL_Joystick *joy[6] = { NULL, NULL, NULL, NULL, NULL, NULL };

void
End_Input ()
{
  //close_joystick();
}

void
open_joystick ()
{
  int Cur_Joy = 0;
  Nb_Joys = 0;
  SDL_JoystickEventState (SDL_ENABLE);
  
  while (Cur_Joy < 6)
    {
      joy[Cur_Joy] = SDL_JoystickOpen (Cur_Joy);
      if (joy[Cur_Joy])
    Nb_Joys++;
      Cur_Joy++;
    }
}

void
close_joystick ()
{
  int Cur_Joy = 0;
  
  while (Cur_Joy < 6)
    {
      if (SDL_JoystickOpened (Cur_Joy))
    {
          SDL_JoystickClose (joy[Cur_Joy]);
          joy[Cur_Joy] = NULL;
        }
      Cur_Joy++;
    }
}


/**
 * keysnoop(): Keysnooping callback event for GTK+.
 * @param grab_widget Widget this key was snooped from.
 * @param event Event information.
 * @param func_data User data.
 * @return TRUE to stop processing this event; FALSE to allow GTK+ to process this event.
 */
static gint keysnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data)
{
	SDL_Event sdlev;
	
	// Only grab keys from the Gens window.
	if (grab != gens_window)
		return FALSE;
	
	switch(event->type)
	{
		case GDK_KEY_PRESS:
			sdlev.type = SDL_KEYDOWN;
			sdlev.key.state = SDL_PRESSED;
			break;
		case GDK_KEY_RELEASE:
			sdlev.type = SDL_KEYUP;
			sdlev.key.state = SDL_RELEASED;
			break;
		default:
			fputs("Can't happen: keysnoop got a bad type\n", stderr);
			return 0;
	}
	
	// Convert this keypress from GDK to SDL.
	// TODO: Use GENS key defines instead.
	sdlev.key.keysym.sym = gdk_to_sdl_keyval(event->keyval);
	if (sdlev.key.keysym.sym != -1)
		SDL_PushEvent(&sdlev);
	
	return FALSE;
}

int
Init_Input (void)
{
  gtk_key_snooper_install(keysnoop, NULL);
  memset (joystate, 0, sizeof (joystate));
  if (SDL_NumJoysticks () > 0)
    {
      open_joystick ();
    }
  return 1;
}


void
Restore_Input ()
{
}

int
Check_Key_Pressed (unsigned int key)
{
  int Num_Joy;
  if (key < 1024)
    {
      if (Keys[key])
	return (1);
    }
  else
    {
      Num_Joy = ((key >> 8) & 0xF);

      if ((Num_Joy < 6) && joy[Num_Joy])
	{
	  if (key & 0x80)	// Test POV Joys
	    {
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

	    }
	  else if (key & 0x70)	// Test Button Joys
	    {
	      if (joystate[0x10 + 0x100 * Num_Joy + ((key & 0xFF) - 0x10)])
		return 1;
	    }
	  else
	    {
	      switch (key & 0xF)
		{
		case 1:
		  if (joystate[0x100 * Num_Joy + 0x1])
		    return 1;
		  break;

		case 2:
		  if (joystate[0x100 * Num_Joy + 0x2])
		    return 1;
		  break;

		case 3:
		  if (joystate[0x100 * Num_Joy + 0x3])
		    return 1;
		  break;

		case 4:
		  if (joystate[0x100 * Num_Joy + 0x4])
		    return 1;
		  break;
		  
		case 5:
		  if (joystate[0x100 * Num_Joy + 0x5])
		    return 1;
		  break;

		case 6:
		  if (joystate[0x100 * Num_Joy + 0x6])
		    return 1;
		  break;

		case 7:
		  if (joystate[0x100 * Num_Joy + 0x7])
		    return 1;
		  break;

		case 8:
		  if (joystate[0x100 * Num_Joy + 0x8])
		    return 1;
		  break;
		  
		case 9:
		  if (joystate[0x100 * Num_Joy + 0x9])
		    return 1;
		  break;

		case 10:
		  if (joystate[0x100 * Num_Joy + 0xA])
		    return 1;
		  break;

		case 11:
		  if (joystate[0x100 * Num_Joy + 0xB])
		    return 1;
		  break;

		case 12:
		  if (joystate[0x100 * Num_Joy + 0xC])
		    return 1;
		  break;
		}
	    }
	}
    }
  return 0;
}

static int
gdk_to_sdl_keyval (int gdk_key)
{
  switch (gdk_key)
    {
    case GDK_BackSpace:
      return SDLK_BACKSPACE;
    case GDK_Tab:
      return SDLK_TAB;
    case GDK_Clear:
      return SDLK_CLEAR;
    case GDK_Return:
      return SDLK_RETURN;
    case GDK_Pause:
      return SDLK_PAUSE;
    case GDK_Escape:
      return SDLK_ESCAPE;
    case GDK_KP_Space:
      return SDLK_SPACE;
    case GDK_exclamdown:
      return SDLK_EXCLAIM;
    case GDK_quotedbl:
      return SDLK_QUOTEDBL;
    case GDK_numbersign:
      return SDLK_HASH;
    case GDK_dollar:
      return SDLK_DOLLAR;
    case GDK_ampersand:
      return SDLK_AMPERSAND;
    case GDK_quoteright:
      return SDLK_QUOTE;
    case GDK_parenleft:
      return SDLK_LEFTPAREN;
    case GDK_parenright:
      return SDLK_RIGHTPAREN;
    case GDK_asterisk:
      return SDLK_ASTERISK;
    case GDK_plus:
      return SDLK_PLUS;
    case GDK_comma:
      return SDLK_COMMA;
    case GDK_minus:
      return SDLK_MINUS;
    case GDK_period:
      return SDLK_PERIOD;
    case GDK_slash:
      return SDLK_SLASH;
    case GDK_0:
      return SDLK_0;
    case GDK_1:
      return SDLK_1;
    case GDK_2:
      return SDLK_2;
    case GDK_3:
      return SDLK_3;
    case GDK_4:
      return SDLK_4;
    case GDK_5:
      return SDLK_5;
    case GDK_6:
      return SDLK_6;
    case GDK_7:
      return SDLK_7;
    case GDK_8:
      return SDLK_8;
    case GDK_9:
      return SDLK_9;
    case GDK_colon:
      return SDLK_COLON;
    case GDK_semicolon:
      return SDLK_SEMICOLON;
    case GDK_less:
      return SDLK_LESS;
    case GDK_equal:
      return SDLK_EQUALS;
    case GDK_greater:
      return SDLK_GREATER;
    case GDK_question:
      return SDLK_QUESTION;
    case GDK_at:
      return SDLK_AT;
    case GDK_bracketleft:
      return SDLK_LEFTBRACKET;
    case GDK_backslash:
      return SDLK_BACKSLASH;
    case GDK_bracketright:
      return SDLK_RIGHTBRACKET;
    case GDK_asciicircum:
      return SDLK_CARET;
    case GDK_underscore:
      return SDLK_UNDERSCORE;
    case GDK_quoteleft:
      return SDLK_BACKQUOTE;
    case GDK_a:
      return SDLK_a;
    case GDK_b:
      return SDLK_b;
    case GDK_c:
      return SDLK_c;
    case GDK_d:
      return SDLK_d;
    case GDK_e:
      return SDLK_e;
    case GDK_f:
      return SDLK_f;
    case GDK_g:
      return SDLK_g;
    case GDK_h:
      return SDLK_h;
    case GDK_i:
      return SDLK_i;
    case GDK_j:
      return SDLK_j;
    case GDK_k:
      return SDLK_k;
    case GDK_l:
      return SDLK_l;
    case GDK_m:
      return SDLK_m;
    case GDK_n:
      return SDLK_n;
    case GDK_o:
      return SDLK_o;
    case GDK_p:
      return SDLK_p;
    case GDK_q:
      return SDLK_q;
    case GDK_r:
      return SDLK_r;
    case GDK_s:
      return SDLK_s;
    case GDK_t:
      return SDLK_t;
    case GDK_u:
      return SDLK_u;
    case GDK_v:
      return SDLK_v;
    case GDK_w:
      return SDLK_w;
    case GDK_x:
      return SDLK_x;
    case GDK_y:
      return SDLK_y;
    case GDK_z:
      return SDLK_z;
    case GDK_Delete:
      return SDLK_DELETE;
    case GDK_KP_0:
      return SDLK_KP0;
    case GDK_KP_1:
      return SDLK_KP1;
    case GDK_KP_2:
      return SDLK_KP2;
    case GDK_KP_3:
      return SDLK_KP3;
    case GDK_KP_4:
      return SDLK_KP4;
    case GDK_KP_5:
      return SDLK_KP5;
    case GDK_KP_6:
      return SDLK_KP6;
    case GDK_KP_7:
      return SDLK_KP7;
    case GDK_KP_8:
      return SDLK_KP8;
    case GDK_KP_9:
      return SDLK_KP9;
    case GDK_KP_Decimal:
      return SDLK_KP_PERIOD;
    case GDK_KP_Divide:
      return SDLK_KP_DIVIDE;
    case GDK_KP_Multiply:
      return SDLK_KP_MULTIPLY;
    case GDK_KP_Subtract:
      return SDLK_KP_MINUS;
    case GDK_KP_Add:
      return SDLK_KP_PLUS;
    case GDK_KP_Enter:
      return SDLK_KP_ENTER;
    case GDK_KP_Equal:
      return SDLK_KP_EQUALS;
    case GDK_Up:
      return SDLK_UP;
    case GDK_Down:
      return SDLK_DOWN;
    case GDK_Right:
      return SDLK_RIGHT;
    case GDK_Left:
      return SDLK_LEFT;
    case GDK_Insert:
      return SDLK_INSERT;
    case GDK_Home:
      return SDLK_HOME;
    case GDK_End:
      return SDLK_END;
    case GDK_Page_Up:
      return SDLK_PAGEUP;
    case GDK_Page_Down:
      return SDLK_PAGEDOWN;
    case GDK_F1:
      return SDLK_F1;
    case GDK_F2:
      return SDLK_F2;
    case GDK_F3:
      return SDLK_F3;
    case GDK_F4:
      return SDLK_F4;
    case GDK_F5:
      return SDLK_F5;
    case GDK_F6:
      return SDLK_F6;
    case GDK_F7:
      return SDLK_F7;
    case GDK_F8:
      return SDLK_F8;
    case GDK_F9:
      return SDLK_F9;
    case GDK_F10:
      return SDLK_F10;
    case GDK_F11:
      return SDLK_F11;
    case GDK_F12:
      return SDLK_F12;
    case GDK_F13:
      return SDLK_F13;
    case GDK_F14:
      return SDLK_F14;
    case GDK_F15:
      return SDLK_F15;
    case GDK_Num_Lock:
      return SDLK_NUMLOCK;
    case GDK_Caps_Lock:
      return SDLK_CAPSLOCK;
    case GDK_Scroll_Lock:
      return SDLK_SCROLLOCK;
    case GDK_Shift_R:
      return SDLK_RSHIFT;
    case GDK_Shift_L:
      return SDLK_LSHIFT;
    case GDK_Control_R:
      return SDLK_RCTRL;
    case GDK_Control_L:
      return SDLK_LCTRL;
    case GDK_Alt_R:
      return SDLK_RALT;
    case GDK_Alt_L:
      return SDLK_LALT;
    case GDK_Meta_R:
      return SDLK_RMETA;
    case GDK_Meta_L:
      return SDLK_LMETA;
    case GDK_Super_L:
      return SDLK_LSUPER;
    case GDK_Super_R:
      return SDLK_RSUPER;
    case GDK_Mode_switch:
      return SDLK_MODE;
      //case GDK_ : return    SDLK_COMPOSE;
    case GDK_Help:
      return SDLK_HELP;
    case GDK_Print:
      return SDLK_PRINT;
    case GDK_Sys_Req:
      return SDLK_SYSREQ;
    case GDK_Break:
      return SDLK_BREAK;
    case GDK_Menu:
      return SDLK_MENU;
      //case GDK_ : return    SDLK_POWER;
    case GDK_EuroSign:
      return SDLK_EURO;
      //case GDK_Undo : return        SDLK_UNDO;

    default:
      //fprintf (stderr, "unknown gdk key\n");
      return -1;
    }
}

unsigned int
Get_Key (void)
{
  GdkEvent *event;
  SDL_Event sdl_event;
  SDL_Joystick *js[6];
  SDL_JoystickEventState (SDL_ENABLE);

  js[0] = SDL_JoystickOpen (0);
  js[1] = SDL_JoystickOpen (1);
  js[2] = SDL_JoystickOpen (2);
  js[3] = SDL_JoystickOpen (3);
  js[4] = SDL_JoystickOpen (4);
  js[5] = SDL_JoystickOpen (5);

  while (gtk_events_pending ())
    gtk_main_iteration ();

  while (1)
    {
      while (SDL_PollEvent (&sdl_event))
	{
	  switch (sdl_event.type)
	    {
	    case SDL_JOYAXISMOTION:
	      if (sdl_event.jaxis.value < -10000)
		{
		  if (sdl_event.jaxis.axis == 0)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x3));
		    }
		  else if (sdl_event.jaxis.axis == 1)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x1));
		    }
		  else if (sdl_event.jaxis.axis == 2)
		{
		  return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x7));
		}
		  else if (sdl_event.jaxis.axis == 3)
		{
		  return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x5));
		}
		  else if (sdl_event.jaxis.axis == 4)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0xB));
		    }
		  else if (sdl_event.jaxis.axis == 5)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x9));
		    }
		}
	      else if (sdl_event.jaxis.value > 10000)
		{
		  if (sdl_event.jaxis.axis == 0)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x4));
		    }
		  else if (sdl_event.jaxis.axis == 1)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x2));
		    }
		  else if (sdl_event.jaxis.axis == 2)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x8));
		    }
		  else if (sdl_event.jaxis.axis == 3)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0x6));
		    }
		  else if (sdl_event.jaxis.axis == 4)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0xC));
		    }
		  else if (sdl_event.jaxis.axis == 5)
		    {
		      return (0x1000 +
			      (0x100 * sdl_event.jaxis.which + 0xA));
		    }
		}
	      else
		{
		  return (Get_Key ());
		}
	      break;

	    case SDL_JOYBUTTONUP:
	      return (0x1010 + (0x100 * sdl_event.jbutton.which) +
		      sdl_event.jbutton.button);
	      break;

	      //case SDL_JOYHATMOTION:                                        
//                                      return(0xdeadbeef + (0x100 * sdl_event.jhat.which) + sdl_event.jhat.hat + sdl_event.jhat.value);
	    }
	}

      event = gdk_event_get ();
      if (event && event->type == GDK_KEY_PRESS)
	{
	  return gdk_to_sdl_keyval (event->key.keyval);
	}
    }
}


void Update_Controllers(void)
{
	CHECK_PLAYER_PAD (0, 1);
	CHECK_PLAYER_PAD (1, 2);
	
	if (Controller_1_Type & 0x10)
	{
		// TEAMPLAYER PORT 1
		CHECK_PLAYER_PAD (2, 1B);
		CHECK_PLAYER_PAD (3, 1C);
		CHECK_PLAYER_PAD (4, 1D);
	}
	
	if (Controller_2_Type & 0x10);
	{
		// TEAMPLAYER PORT 2
		CHECK_PLAYER_PAD (5, 2B);
		CHECK_PLAYER_PAD (6, 2C);
		CHECK_PLAYER_PAD (7, 2D);
	}
}


void
Scan_Player_Net (int Player)
{
#if 0
  if (!Player)
    return;

  Update_Input ();

  if (Check_Key_Pressed (Keys_Def[0].Up))
    {
      Kaillera_Keys[0] &= ~0x08;
      Kaillera_Keys[0] |= 0x04;
    }
  else
    {
      Kaillera_Keys[0] |= 0x08;
      if (Check_Key_Pressed (Keys_Def[0].Down))
	Kaillera_Keys[0] &= ~0x04;
      else
	Kaillera_Keys[0] |= 0x04;
    }

  if (Check_Key_Pressed (Keys_Def[0].Left))
    {
      Kaillera_Keys[0] &= ~0x02;
      Kaillera_Keys[0] |= 0x01;
    }
  else
    {
      Kaillera_Keys[0] |= 0x02;
      if (Check_Key_Pressed (Keys_Def[0].Right))
	Kaillera_Keys[0] &= ~0x01;
      else
	Kaillera_Keys[0] |= 0x01;
    }

  if (Check_Key_Pressed (Keys_Def[0].Start))
    Kaillera_Keys[0] &= ~0x80;
  else
    Kaillera_Keys[0] |= 0x80;

  if (Check_Key_Pressed (Keys_Def[0].A))
    Kaillera_Keys[0] &= ~0x40;
  else
    Kaillera_Keys[0] |= 0x40;

  if (Check_Key_Pressed (Keys_Def[0].B))
    Kaillera_Keys[0] &= ~0x20;
  else
    Kaillera_Keys[0] |= 0x20;

  if (Check_Key_Pressed (Keys_Def[0].C))
    Kaillera_Keys[0] &= ~0x10;
  else
    Kaillera_Keys[0] |= 0x10;

  if (Controller_1_Type & 1)
    {
      if (Check_Key_Pressed (Keys_Def[0].Mode))
	Kaillera_Keys[1] &= ~0x08;
      else
	Kaillera_Keys[1] |= 0x08;

      if (Check_Key_Pressed (Keys_Def[0].X))
	Kaillera_Keys[1] &= ~0x04;
      else
	Kaillera_Keys[1] |= 0x04;

      if (Check_Key_Pressed (Keys_Def[0].Y))
	Kaillera_Keys[1] &= ~0x02;
      else
	Kaillera_Keys[1] |= 0x02;

      if (Check_Key_Pressed (Keys_Def[0].Z))
	Kaillera_Keys[1] &= ~0x01;
      else
	Kaillera_Keys[1] |= 0x01;
    }
#endif
}


void
Update_Controllers_Net (int num_player)
{
#if 0
  Controller_1_Up = (Kaillera_Keys[0] & 0x08) >> 3;
  Controller_1_Down = (Kaillera_Keys[0] & 0x04) >> 2;
  Controller_1_Left = (Kaillera_Keys[0] & 0x02) >> 1;
  Controller_1_Right = (Kaillera_Keys[0] & 0x01);
  Controller_1_Start = (Kaillera_Keys[0] & 0x80) >> 7;
  Controller_1_A = (Kaillera_Keys[0] & 0x40) >> 6;
  Controller_1_B = (Kaillera_Keys[0] & 0x20) >> 5;
  Controller_1_C = (Kaillera_Keys[0] & 0x10) >> 4;

  if (Controller_1_Type & 1)
    {
      Controller_1_Mode = (Kaillera_Keys[0 + 1] & 0x08) >> 3;
      Controller_1_X = (Kaillera_Keys[0 + 1] & 0x04) >> 2;
      Controller_1_Y = (Kaillera_Keys[0 + 1] & 0x02) >> 1;
      Controller_1_Z = (Kaillera_Keys[0 + 1] & 0x01);
    }

  if (num_player > 2)		// TEAMPLAYER
    {
      Controller_1B_Up = (Kaillera_Keys[2] & 0x08) >> 3;
      Controller_1B_Down = (Kaillera_Keys[2] & 0x04) >> 2;
      Controller_1B_Left = (Kaillera_Keys[2] & 0x02) >> 1;
      Controller_1B_Right = (Kaillera_Keys[2] & 0x01);
      Controller_1B_Start = (Kaillera_Keys[2] & 0x80) >> 7;
      Controller_1B_A = (Kaillera_Keys[2] & 0x40) >> 6;
      Controller_1B_B = (Kaillera_Keys[2] & 0x20) >> 5;
      Controller_1B_C = (Kaillera_Keys[2] & 0x10) >> 4;

      if (Controller_1B_Type & 1)
	{
	  Controller_1B_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
	  Controller_1B_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
	  Controller_1B_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
	  Controller_1B_Z = (Kaillera_Keys[2 + 1] & 0x01);
	}

      Controller_1C_Up = (Kaillera_Keys[4] & 0x08) >> 3;
      Controller_1C_Down = (Kaillera_Keys[4] & 0x04) >> 2;
      Controller_1C_Left = (Kaillera_Keys[4] & 0x02) >> 1;
      Controller_1C_Right = (Kaillera_Keys[4] & 0x01);
      Controller_1C_Start = (Kaillera_Keys[4] & 0x80) >> 7;
      Controller_1C_A = (Kaillera_Keys[4] & 0x40) >> 6;
      Controller_1C_B = (Kaillera_Keys[4] & 0x20) >> 5;
      Controller_1C_C = (Kaillera_Keys[4] & 0x10) >> 4;

      if (Controller_1C_Type & 1)
	{
	  Controller_1C_Mode = (Kaillera_Keys[4 + 1] & 0x08) >> 3;
	  Controller_1C_X = (Kaillera_Keys[4 + 1] & 0x04) >> 2;
	  Controller_1C_Y = (Kaillera_Keys[4 + 1] & 0x02) >> 1;
	  Controller_1C_Z = (Kaillera_Keys[4 + 1] & 0x01);
	}

      Controller_1D_Up = (Kaillera_Keys[6] & 0x08) >> 3;
      Controller_1D_Down = (Kaillera_Keys[6] & 0x04) >> 2;
      Controller_1D_Left = (Kaillera_Keys[6] & 0x02) >> 1;
      Controller_1D_Right = (Kaillera_Keys[6] & 0x01);
      Controller_1D_Start = (Kaillera_Keys[6] & 0x80) >> 7;
      Controller_1D_A = (Kaillera_Keys[6] & 0x40) >> 6;
      Controller_1D_B = (Kaillera_Keys[6] & 0x20) >> 5;
      Controller_1D_C = (Kaillera_Keys[6] & 0x10) >> 4;

      if (Controller_1D_Type & 1)
	{
	  Controller_1D_Mode = (Kaillera_Keys[6 + 1] & 0x08) >> 3;
	  Controller_1D_X = (Kaillera_Keys[6 + 1] & 0x04) >> 2;
	  Controller_1D_Y = (Kaillera_Keys[6 + 1] & 0x02) >> 1;
	  Controller_1D_Z = (Kaillera_Keys[6 + 1] & 0x01);
	}
    }
  else
    {
      Controller_2_Up = (Kaillera_Keys[2] & 0x08) >> 3;
      Controller_2_Down = (Kaillera_Keys[2] & 0x04) >> 2;
      Controller_2_Left = (Kaillera_Keys[2] & 0x02) >> 1;
      Controller_2_Right = (Kaillera_Keys[2] & 0x01);
      Controller_2_Start = (Kaillera_Keys[2] & 0x80) >> 7;
      Controller_2_A = (Kaillera_Keys[2] & 0x40) >> 6;
      Controller_2_B = (Kaillera_Keys[2] & 0x20) >> 5;
      Controller_2_C = (Kaillera_Keys[2] & 0x10) >> 4;

      if (Controller_2_Type & 1)
	{
	  Controller_2_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
	  Controller_2_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
	  Controller_2_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
	  Controller_2_Z = (Kaillera_Keys[2 + 1] & 0x01);
	}
    }
#endif
}
