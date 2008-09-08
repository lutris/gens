#if 0

#ifndef GENS_SDLINPUT_H
#define GENS_SDLINPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <SDL/SDL.h>

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

#ifdef __cplusplus
extern SDL_Joystick* joy[6];
#else
extern struct SDL_Joystick* joy[6];
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif
