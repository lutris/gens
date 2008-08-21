#ifndef G_MAIN_H
#define G_MAIN_H

#include "port.h"

// TODO: Make this conditional based on operating system.
#include "g_main_linux.h"

#define UPDATE_GTK 0
#define UPDATE_GTK_SOUNDOFF 1
#define UPDATE_GTK_SOUNDON 2

extern int Paused;
extern int Net_Play;
extern int Current_State;
extern int Full_Screen;
extern int Fast_Blur;
extern int Render_Mode;
extern int Opengl;
extern int Width_gl;
extern int Height_gl;
extern int Bpp;
//extern int gl_linear_filter;
extern int Show_FPS;
extern int Show_Message;
extern int Show_LED;
extern int Auto_Pause;
extern int Auto_Fix_CS;
extern int Language;
extern int Country;
extern int Country_Order[3];
extern int Kaillera_Client_Running;
extern int Intro_Style;
extern int SegaCD_Accurate;
extern int Active;
extern int Quick_Exit;
extern int FS_Minimised;

extern POINT Window_Pos;

#define GENS_PATH_MAX 1024
extern char Start_Rom[GENS_PATH_MAX];
extern char Str_Tmp[GENS_PATH_MAX];
extern char Gens_Path[GENS_PATH_MAX];
extern char Language_Path[GENS_PATH_MAX];
extern char CGOffline_Path[GENS_PATH_MAX];
extern char Manual_Path[GENS_PATH_MAX];

extern char **language_name;
	
extern unsigned char Keys[];
extern unsigned char joystate[];

int Set_Render(int Full, int Num, int Force);
int Change_68K_Type(int hWnd, int Num, int Reset_SND);
//extern void Read_To_68K_Space(int adr);

void End_All(void);
void Sleep(int i);
void close_gens();

#include <gtk/gtk.h>
extern GtkWidget* gens_window;

#define MESSAGE_L(str, def, time) \
{\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	Put_Info(Str_Tmp, (time));\
}



#define MESSAGE_NUM_L(str, def, num, time)\
{\
	char mes_tmp[1024];\
	Str_Tmp[0]='\0';\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	sprintf(mes_tmp, Str_Tmp, (num));\
	Put_Info(mes_tmp, (time));\
}

#define MINIMIZE \
{\
	if (Sound_Initialised) Clear_Sound_Buffer();\
	if (Full_Screen)\
	{\
		FS_Minimised = 1;\
	}\
}
//		Set_Render(0, -1, 1);

#if 0

#define MENU_L(smenu, pos, flags, id, str, suffixe, def)\
GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
strcat(Str_Tmp, (suffixe));\
InsertMenu((smenu), (pos), (flags), (id), Str_Tmp);

#define WORD_L(id, str, suffixe, def)\
GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
strcat(Str_Tmp, (suffixe));\
SetDlgItemText(hDlg, id, Str_Tmp);

#endif


#endif
