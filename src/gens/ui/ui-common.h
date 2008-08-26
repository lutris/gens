/**
 * GENS: Common UI functions.
 */
 
#ifndef UI_COMMON_H
#define UI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	AnyFile		= 0,
	ROMFile		= 1,
	SavestateFile	= 2,
	CDImage		= 3,
	ConfigFile	= 4,
	GYMFile		= 5,
} FileFilterType;


// The following functions are implemented by the OS-specific ui-common_*.c file.
void UI_Set_Window_Title(const char* title);
void UI_Set_Window_Visibility (int visibility);

void UI_Hide_Embedded_Window(void);
void UI_Show_Embedded_Window(int w, int h);
int UI_Get_Embedded_WindowID(void);

void UI_MsgBox(const char* msg, const char* title);
int UI_OpenFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile);
int UI_SaveFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile);
int UI_SelectDir(const char* title, const char* initDir, char* retSelectedDir);


// The following functions are implemented by ui-common.c.
void UI_Set_Window_Title_Idle(void);
void UI_Set_Window_Title_Game(const char* system, const char* game);
void UI_Set_Window_Title_Init(const char* system, int reinit);


#ifdef __cplusplus
}
#endif

#endif
