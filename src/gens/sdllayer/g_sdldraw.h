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
extern int Kaillera_Error;

extern void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern int (*Update_Frame)(void);
extern int (*Update_Frame_Fast)(void);

int Init_Fail(int hwnd, char *err);
int Init_DDraw();
int Clear_Primary_Screen(void);
int Clear_Back_Screen(void);
void Clear_Screen();
int Update_Gens_Logo(void);
int Update_Crazy_Effect(void);
int Update_Emulation(void);
int Update_Emulation_One(void);
int Update_Emulation_Netplay(int player, int num_player);
int Eff_Screen(void);
int Pause_Screen(void);
void Put_Info(char *Message, int Duree);
int Show_Genesis_Screen(void);
int Flip(void);
void Restore_Primary(void);
void End_DDraw(void);
void Refresh_gl();
void Put_Info(char *,int);
void Refresh_video();
void Set_GL_Resolution(int w,int h);
void Set_Bpp(int newbpp);
void Change_OpenGL(int newOpenGL);
void Adjust_Stretch();
extern SDL_Surface *surface;

#endif
