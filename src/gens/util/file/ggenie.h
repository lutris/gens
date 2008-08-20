/* Header file for decode functions */
#ifndef _GENIE_DECODE_H__
#define _GENIE_DECODE_H__

struct patch { unsigned int addr, data; };

struct GG_Code
{
	char code[16];
	char name[240];
	unsigned int active;
	unsigned int restore;
	unsigned int addr;
	unsigned short data;
};

extern struct GG_Code Liste_GG[256];
extern char Patch_Dir[1024];

int Load_Patch_File(void);
int Save_Patch_File(void);
void decode(const char* code, struct patch *result);
void Patch_Codes(void);

#endif // _GENIE_DECODE_H__
