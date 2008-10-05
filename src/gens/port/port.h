#ifndef __PORT_H__
#define __PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WIN32__
typedef struct _POINT {
	int x;
	int y;
} POINT;

void SetCurrentDirectory(const char *directory);
int GetCurrentDirectory(int size,char* buf);
unsigned int GetTickCount(void);
#endif /* __WIN32__ */

#ifdef __cplusplus
}
#endif

#endif
