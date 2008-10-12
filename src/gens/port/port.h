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

unsigned int GetTickCount(void);
#endif /* __WIN32__ */

#ifdef __cplusplus
}
#endif

#endif
