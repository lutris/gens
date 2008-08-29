#ifndef G_DDRAW_H
#define G_DDRAW_H

#include <time.h>
#include <SDL.h>

extern clock_t Last_Time;
extern clock_t New_Time;
extern clock_t Used_Time;

extern int Flag_Clr_Scr;
extern int Sleep_Time;
extern int FS_VSync;
extern int W_VSync;
extern int row_length;
extern int shift;
extern int Stretch;
extern int Blit_Soft;
extern int Effect_Color;
extern int FPS_Style;
extern int Message_Style;

// Current border color. (16-bit)
extern unsigned short BorderColor_16B;

extern void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern int (*Update_Frame)(void);
extern int (*Update_Frame_Fast)(void);

/*
extern int Kaillera_Error;
int Update_Emulation_Netplay(int player, int num_player);
*/

// SDL screen
extern SDL_Surface *screen;

// SDL flags
extern const int Gens_SDL_Flags;

extern int Dep;
extern int Old_Dep;

int Init_Fail(int hwnd, char *err);
int Init_DDraw();
int Clear_Primary_Screen(void);
int Clear_Back_Screen(void);
void Clear_Screen();
int Update_Emulation(void);
int Update_Emulation_One(void);
void Clear_Screen_MD(void);
int Pause_Screen(void);
void Put_Info(char *Message, int Duree);
int Show_Genesis_Screen(void);
int Flip(void);
void Restore_Primary(void);
void End_DDraw(void);
void Refresh_gl();
void Put_Info(char *,int);
void Refresh_Video(void);
void Set_bpp(int newbpp);
void Change_OpenGL(int newOpenGL);
void Adjust_Stretch(void);

#endif
