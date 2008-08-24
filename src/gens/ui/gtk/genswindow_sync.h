/**
 * GENS: (GTK+) Main Window Synchronization.
 */


#ifndef GENSWINDOW_CALLBACKS_H
#define GENSWINDOW_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	WINDOW_SYNC_ALL		= 0,
	WINDOW_SYNC_SOUND	= 1,
} WindowSyncType;

void Sync_GensWindow(WindowSyncType sync);


#ifdef __cplusplus
}
#endif

#endif
