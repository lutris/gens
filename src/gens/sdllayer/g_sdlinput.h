#ifndef G_SDLINPUT_H
#define G_SDLINPUT_H

#include <gtk/gtk.h>


unsigned int Get_Key(void);
int Check_Key_Pressed(unsigned int key);
int Setting_Keys(GtkWidget* control_window, int Player, int TypeP);


int Init_Input(void);
void End_Input(void);
void Update_Input(void);
void Update_Controllers(void);
void Scan_Player_Net(int Player);
void Update_Controllers_Net(int num_player);

void open_joystick();
void close_joystick();

extern SDL_Joystick* joy[6];

#define CHECK_BUTTON(player, ctrl, button)				\
	if (Check_Key_Pressed(Keys_Def[player].button)) 	\
		Controller_ ## ctrl ## _ ## button = 0; 		\
	else Controller_ ## ctrl ## _ ## button = 1;

#define CHECK_DIR(player, ctrl)							\
	if (Check_Key_Pressed(Keys_Def[player].Up)) 		\
	{									   				\
		Controller_ ## ctrl ## _Up = 0;			   		\
		Controller_ ## ctrl ## _Down = 1;				\
	}													\
	else												\
	{													\
		Controller_ ## ctrl ## _Up = 1;					\
		CHECK_BUTTON(player, ctrl, Down)				\
	}													\
	if (Check_Key_Pressed(Keys_Def[player].Left)) 		\
	{									   				\
		Controller_ ## ctrl ## _Left = 0;			   	\
		Controller_ ## ctrl ## _Right = 1;				\
	}													\
	else												\
	{													\
		Controller_ ## ctrl ## _Left = 1;				\
		CHECK_BUTTON(player,ctrl, Right)				\
	}
		
#define CHECK_ALL_BUTTONS(player, ctrl)					\
	CHECK_BUTTON(player, ctrl, Start)					\
	CHECK_BUTTON(player, ctrl, A)						\
	CHECK_BUTTON(player, ctrl, B)						\
	CHECK_BUTTON(player, ctrl, C)						\
														\
	if (Controller_ ## ctrl ## _Type & 1)				\
	{													\
		CHECK_BUTTON(player, ctrl, Mode)				\
		CHECK_BUTTON(player, ctrl, X)					\
		CHECK_BUTTON(player, ctrl, Y)					\
		CHECK_BUTTON(player, ctrl, Z)					\
	}
	
#define CHECK_PLAYER_PAD(player, ctrl)	\
	CHECK_DIR(player, ctrl)				\
	CHECK_ALL_BUTTONS(player, ctrl)

#endif
