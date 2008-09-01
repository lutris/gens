#ifndef GENS_SDLDRAW_H
#define GENS_SDLDRAW_H

#if 0

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <SDL.h>

extern clock_t Last_Time;
extern clock_t New_Time;
extern clock_t Used_Time;

extern int Flag_Clr_Scr;
extern int row_length;
extern int shift;
extern int Stretch;
extern int Blit_Soft;
extern int FPS_Style;
extern int Message_Style;

// Current border color. (16-bit)
extern unsigned short BorderColor_16B;

extern void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);

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

#ifdef __cplusplus
}
#endif

#endif

#endif /* GENS_SDLDRAW_H */
