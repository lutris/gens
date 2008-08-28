#ifndef G_MAIN_H
#define G_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "port.h"

// OS-specific includes.
#if (defined(__linux__))
#include "g_main_linux.h"
#elif (defined(__WIN32__))
#error TODO: Add Win32 support.
#else
#error Unsupported operating system.
#endif

#define UPDATE_GTK 0
#define UPDATE_GTK_SOUNDOFF 1
#define UPDATE_GTK_SOUNDON 2


// Gens settings
struct Gens_Settings_t
{
};


// Gens Pathnames
#define GENS_PATH_MAX 1024
struct Gens_PathNames_t
{
	char Start_Rom[GENS_PATH_MAX];
	char Gens_Path[GENS_PATH_MAX];
	char Language_Path[GENS_PATH_MAX];
	char CGOffline_Path[GENS_PATH_MAX];
	char Manual_Path[GENS_PATH_MAX];
};


// Video settings
struct Gens_VideoSettings_t
{
	int Full_Screen;
	int Fast_Blur;
	int Render_Mode;	// TODO: Make this an enum.
	int OpenGL;
	int Width_GL;
	int Height_GL;
	int bpp;
};
// from gens-rerecording
// TODO: Move somewhere else.
extern int _XRay;
extern unsigned int _Pal32_XRAY[0x10000];


extern struct Gens_Settings_t Settings;
extern struct Gens_PathNames_t PathNames;
extern struct Gens_VideoSettings_t Video;


extern int Paused;
extern int Net_Play;
extern int Current_State;
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

extern char **language_name;

extern unsigned char Keys[];
extern unsigned char joystate[];

// Stupid temporary string needed for some stuff.
extern char Str_Tmp[GENS_PATH_MAX];

int Set_Render(int Full, int Num, int Force);
int Change_68K_Type(int hWnd, int Num, int Reset_SND);
//extern void Read_To_68K_Space(int adr);

void End_All(void);
void Sleep(int i);
void close_gens();

// GENS Re-Recording
int IsAsyncAllowed(void);

#include <gtk/gtk.h>
extern GtkWidget* gens_window;


/**
 * MESSAGE_L(): Print a localized message.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param time Time to display the message (in milliseconds).
 */
#define MESSAGE_L(str, def, time)							\
{											\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024,	\
				PathNames.Language_Path);			\
	Put_Info(Str_Tmp, (time));							\
}


/**
 * MESSAGE_NUM_L(): Print a localized message with one number in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param num Number.
 * @param time Time to display the message (in milliseconds).
 */
#define MESSAGE_NUM_L(str, def, num, time)						\
{											\
	char mes_tmp[1024];								\
	Str_Tmp[0] = '\0';								\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024,	\
				PathNames.Language_Path);			\
	sprintf(mes_tmp, Str_Tmp, (num));						\
	Put_Info(mes_tmp, (time));							\
}


/**
 * MESSAGE_NUM_2L(): Print a localized message with two numbers in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param num1 First number.
 * @param num2 Second number.
 * @param time Time to display the message (in milliseconds).
 */
#define MESSAGE_NUM_2L(str, def, num1, num2, time)					\
{											\
	char mes_tmp[1024];								\
	Str_Tmp[0] = '\0';								\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024,	\
				PathNames.Language_Path);			\
	sprintf(mes_tmp, Str_Tmp, (num1), (num2));					\
	Put_Info(mes_tmp, (time));							\
}


#define MINIMIZE \
{\
	if (Sound_Initialised) Clear_Sound_Buffer();\
	if (Video.Full_Screen)\
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

#ifdef __cplusplus
}
#endif

#endif
