#ifndef GYM_H
#define GYM_H

#include <stdio.h>

extern int GYM_Dumping;
extern FILE *GYM_File;
	
int Update_GYM_Dump(int v0, int v1, unsigned char v2);
int Start_GYM_Dump(void);
int Stop_GYM_Dump(void);
int Start_Play_GYM(void);
int Play_GYM(void);
int Stop_Play_GYM(void);

#endif
