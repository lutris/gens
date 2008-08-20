#ifndef SAVE_H_
#define SAVE_H_

#include <stdio.h>

#define GENESIS_STATE_FILE_LENGHT    0x22478
#define GENESIS_STATE_FILE_LENGHT_EX 0x25550
#define SEGACD_STATE_FILE_LENGHT     (0x22500 + 0xE0000)
#define G32X_STATE_FILE_LENGHT       (0x22500 + 0x82A00)
#define SCD32X_STATE_FILE_LENGHT     (0x22500 + 0xE0000 + 0x82A00)
#define MAX_STATE_FILE_LENGHT        SCD32X_STATE_FILE_LENGHT

extern char State_Dir[1024];
extern char SRAM_Dir[1024];
extern char BRAM_Dir[1024];

int Change_File_S(char *Dest, char *Dir);
int Change_File_L(char *Dest, char *Dir);
//int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext);
FILE *Get_State_File();
void Get_State_File_Name(char *name);
int Load_State(char *Name);
int Save_State(char *Name);
void Import_Genesis(unsigned char *Data);
void Export_Genesis(unsigned char *Data);
void Import_SegaCD(unsigned char *Data);
void Export_SegaCD(unsigned char *Data);
void Import_32X(unsigned char *Data);
void Export_32X(unsigned char *Data);
int Save_Config(char *File_Name);
int Save_As_Config(void);
int Load_Config(char *File_Name, void *Game_Active);
int Load_As_Config(void *Game_Active);
int Load_SRAM(void);
int Save_SRAM(void);
int Load_BRAM(void);
int Save_BRAM(void);
void Format_Backup_Ram(void);
int Change_File_L_CD (char *Dest, char *Dir);

#endif

