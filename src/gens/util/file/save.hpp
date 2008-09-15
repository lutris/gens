#ifndef GENS_SAVE_HPP
#define GENS_SAVE_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "emulator/gens.hpp"

#define GENESIS_LENGTH_EX1		0x2247C
#define GENESIS_LENGTH_EX2		0x11ED2
#define GENESIS_STATE_LENGTH		(GENESIS_LENGTH_EX1 + GENESIS_LENGTH_EX2)
#define GENESIS_V6_LENGTH_EX2		0x7EC
#define GENESIS_V6_STATE_LENGTH		(GENESIS_LENGTH_EX1 + GENESIS_V6_LENGTH_EX2)
#define SEGACD_LENGTH_EX1		0xE19A4
#define SEGACD_LENGTH_EX2		0x1238B
#define SEGACD_LENGTH_EX		(SEGACD_LENGTH_EX1 + SEGACD_LENGTH_EX2)
#define G32X_LENGTH_EX			0x849BF
#define MAX_STATE_FILE_LENGTH		(GENESIS_STATE_LENGTH + SEGACD_LENGTH_EX + G32X_LENGTH_EX)

/*
// Old GENS v5 savestate offsets.
#define GENESIS_STATE_FILE_LENGTH    0x22478
#define GENESIS_STATE_FILE_LENGTH_EX 0x25550
#define SEGACD_STATE_FILE_LENGTH     (0x22500 + 0xE0000)
#define G32X_STATE_FILE_LENGTH       (0x22500 + 0x82A00)
#define SCD32X_STATE_FILE_LENGTH     (0x22500 + 0xE0000 + 0x82A00)
#define MAX_STATE_FILE_LENGTH        SCD32X_STATE_FILE_LENGTH
*/

extern char State_Dir[GENS_PATH_MAX];
extern char SRAM_Dir[GENS_PATH_MAX];
extern char BRAM_Dir[GENS_PATH_MAX];

int Change_File_S(char *Dest, char *Dir);
int Change_File_L(char *Dest, char *Dir);
//int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext);
FILE *Get_State_File();
void Get_State_File_Name(char *name);
int Load_State(char *Name);
int Save_State(char *Name);
int Import_Genesis(unsigned char *Data);
void Export_Genesis(unsigned char *Data);
void Import_SegaCD(unsigned char *Data);
void Export_SegaCD(unsigned char *Data);
void Import_32X(unsigned char *Data);
void Export_32X(unsigned char *Data);
int Load_SRAM(void);
int Save_SRAM(void);
int Load_BRAM(void);
int Save_BRAM(void);
void Format_Backup_Ram(void);
int Change_File_L_CD (char *Dest, char *Dir);

#ifdef __cplusplus
}
#endif

#endif /* GENS_SAVE_H */
