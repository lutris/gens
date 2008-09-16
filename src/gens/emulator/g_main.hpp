#ifndef G_MAIN_HPP
#define G_MAIN_HPP

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gens.hpp"
#include "port/port.h"

// OS-specific includes.
#if (defined(__linux__))
#include "g_main_linux.hpp"
#elif (defined(__WIN32__))
#error TODO: Add Win32 support.
#else
#error Unsupported operating system.
#endif


// Gens settings
struct Gens_Settings_t
{
};


// Gens Pathnames
struct Gens_PathNames_t
{
	char Start_Rom[GENS_PATH_MAX];
	char Gens_Path[GENS_PATH_MAX];
	char Language_Path[GENS_PATH_MAX];
	char Dump_WAV_Dir[GENS_PATH_MAX];
	char Dump_GYM_Dir[GENS_PATH_MAX];
};


// BIOS filenames
struct Gens_BIOS_Filenames_t
{
	char MD_TMSS[GENS_PATH_MAX];
	char SegaCD_US[GENS_PATH_MAX];
	char MegaCD_EU[GENS_PATH_MAX];
	char MegaCD_JP[GENS_PATH_MAX];
	char _32X_MC68000[GENS_PATH_MAX];
	char _32X_MSH2[GENS_PATH_MAX];
	char _32X_SSH2[GENS_PATH_MAX];
};


// Miscellaneous filenames
struct Gens_Misc_Filenames_t
{
	char _7z_Binary[GENS_PATH_MAX];
	char GCOffline[GENS_PATH_MAX];
	char Manual[GENS_PATH_MAX];
};


// Video settings
struct Gens_VideoSettings_t
{
	int Render_W;	// TODO: Make this an enum.
	int Render_FS;	// TODO: Make this an enum.
	int VSync_FS;
	int VSync_W;
#ifdef GENS_OPENGL
	int OpenGL;
	int Width_GL;
	int Height_GL;
	int glLinearFilter;
#endif
};

extern struct Gens_Settings_t Settings;
extern struct Gens_PathNames_t PathNames;
extern struct Gens_BIOS_Filenames_t BIOS_Filenames;
extern struct Gens_Misc_Filenames_t Misc_Filenames;
extern struct Gens_VideoSettings_t Video;

// Bits per pixel.
// This is used by asm functions, so it can't be kept in a struct.
extern unsigned char bpp;

// TODO: Only used for DirectDraw.
extern int Flag_Clr_Scr;

// from gens-rerecording
// TODO: Move somewhere else.
extern int _XRay;
extern unsigned int _Pal32_XRAY[0x10000];

extern int Paused;
extern int Net_Play;
extern int Current_State;
//extern int gl_linear_filter;
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
extern int fast_forward;

extern POINT Window_Pos;

extern char **language_name;

extern unsigned char Keys[];
extern unsigned char joystate[];

// Stupid temporary string needed for some stuff.
extern char Str_Tmp[GENS_PATH_MAX];

int Change_68K_Type(int hWnd, int Num, int Reset_SND);
//extern void Read_To_68K_Space(int adr);

void End_All(void);
int is_gens_running();
void close_gens();

// GENS Re-Recording
int IsAsyncAllowed(void);


// MESSAGE_L functions.
void MESSAGE_L(const char* str, const char* def, int time);
void MESSAGE_NUM_L(const char* str, const char* def, int num, int time);
void MESSAGE_STR_L(const char* str, const char* def, const char* str2, int time);
void MESSAGE_NUM2_L(const char* str, const char* def, int num1, int num2, int time);


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

extern int ice;

// Update Frame function pointers
extern int (*Update_Frame)(void);
extern int (*Update_Frame_Fast)(void);

// Miscellaneous.
void Clear_Screen_MD(void);

#ifdef GENS_OPENGL
void Change_OpenGL(int newOpenGL);
void Set_GL_Resolution(int w, int h);
#endif /* GENS_OPENGL */

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// C++ classes

// New VDraw object
#include "video/v_draw.hpp"
extern VDraw *draw;

// New Input object
#include "input/input.hpp"
extern Input *input;

// New Audio object
#include "audio/audio.hpp"
extern Audio *audio;

#endif

#endif
