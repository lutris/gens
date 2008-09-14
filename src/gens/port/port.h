#ifndef __PORT_H__
#define __PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _POINT {
	int x;
	int y;
} POINT;

void SetCurrentDirectory(const char *directory);
int GetCurrentDirectory(int size,char* buf);
unsigned int GetTickCount(void);

#ifdef __cplusplus
}
#endif

#endif
