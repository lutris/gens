/**
 * popen_wrapper.c: Provides a popen()-like function for Win32.
 * Adapted from http://lists.trolltech.com/qt-interest/1999-09/thread00282-0.html
 */

#include "popen_wrapper.h"

/*------------------------------------------------------------------------------
  Globals for the routines gens_popen() / gens_pclose()
------------------------------------------------------------------------------*/
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// C includes.
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

// libgsft includes.
#include "libgsft/gsft_strdup.h"


static HANDLE my_pipein[2], my_pipeout[2], my_pipeerr[2];
static char   my_popenmode = ' ';
static PROCESS_INFORMATION piProcInfo;

static int my_pipe(HANDLE *readwrite)
{
	SECURITY_ATTRIBUTES sa;
	
	sa.nLength = sizeof(sa);          /* Length in bytes. */
	sa.bInheritHandle = 1;            /* Descriptors to be inherited. */
	sa.lpSecurityDescriptor = NULL;
	
	if (!CreatePipe(&readwrite[0], &readwrite[1], &sa, 1 << 13))
	{
		errno = EMFILE;
		return -1;
	}
	
	return 0;
}


/**
 * gens_popen(): Replacement for popen() on Win32.
 * If "2>&1" is found in cmd, stderr is redirected to stdout.
 * @param cmd Command line.
 * @param mode Mode. ("r" or "w")
 * @return File handle, or NULL on error.
 */
FILE* gens_popen(const char *cmd, const char *mode)
{
	FILE *fptr = NULL;
	STARTUPINFO siStartInfo;
	int success, redirect;

	my_pipein[0]	= INVALID_HANDLE_VALUE;
	my_pipein[1]	= INVALID_HANDLE_VALUE;
	my_pipeout[0]	= INVALID_HANDLE_VALUE;
	my_pipeout[1]	= INVALID_HANDLE_VALUE;
	my_pipeerr[0]	= INVALID_HANDLE_VALUE;
	my_pipeerr[1]	= INVALID_HANDLE_VALUE;
	
	if (!mode || !*mode)
		goto finito;
	
	my_popenmode = *mode;
	if (my_popenmode != 'r' && my_popenmode != 'w')
		goto finito;
	
	/* Should stderr be redirected to stdout? */
	redirect = (strstr(cmd, "2>&1") != 0);
	
	/* Create the pipes. */
	if (my_pipe(my_pipein)  == -1 ||
	    my_pipe(my_pipeout) == -1)
		goto finito;
	if (!redirect && my_pipe(my_pipeerr) == -1)
		goto finito;
	
	/* Create the child process. */
	memset(&siStartInfo, 0x00, sizeof(STARTUPINFO));
	siStartInfo.cb		= sizeof(STARTUPINFO);
	siStartInfo.hStdInput	= my_pipein[0];
	siStartInfo.hStdOutput	= my_pipeout[1];
	if (redirect)
		siStartInfo.hStdError	= my_pipeout[1];
	else
		siStartInfo.hStdError	= my_pipeerr[1];
	siStartInfo.dwFlags	= STARTF_USESTDHANDLES;
	
	/* Copy the command line before passing it to CreateProcess(). */
	char *cmd_dup = strdup(cmd);
	
	success = CreateProcess(NULL,
				cmd_dup,		// command line 
				NULL,			// process security attributes 
				NULL,			// primary thread security attributes 
				TRUE,			// handles are inherited 
				DETACHED_PROCESS,	// creation flags: no window
				NULL,			// use parent's environment 
				NULL,			// use parent's current directory 
				&siStartInfo,		// STARTUPINFO pointer 
				&piProcInfo);		// receives PROCESS_INFORMATION 
	
	/* Free the copied command line. */
	free(cmd_dup);
	
	if (!success)
		goto finito;
	
	/* These handles belong to the child process. */
	CloseHandle(my_pipein[0]);  my_pipein[0]  = INVALID_HANDLE_VALUE;
	CloseHandle(my_pipeout[1]); my_pipeout[1] = INVALID_HANDLE_VALUE;
	CloseHandle(my_pipeerr[1]); my_pipeerr[1] = INVALID_HANDLE_VALUE;
	
	if (my_popenmode == 'r')
		fptr = _fdopen(_open_osfhandle((long)my_pipeout[0], _O_BINARY), "r");
	else
		fptr = _fdopen(_open_osfhandle((long)my_pipein[1], _O_BINARY), "w");
	
finito:
	if (!fptr)
	{
		if (my_pipein[0]  != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipein[0]);
		if (my_pipein[1]  != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipein[1]);
		if (my_pipeout[0] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeout[0]);
		if (my_pipeout[1] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeout[1]);
		if (my_pipeerr[0] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeerr[0]);
		if (my_pipeerr[1] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeerr[1]);
	}
	return fptr;
}


/**
 * gens_popen(): Replacement for pclose() on Win32.
 * @param fle File handle for the pipe.
 * @return 0 on success; non-zero on error.
 */
int gens_pclose(FILE *fle)
{
	if (!fle)
	{
		// Pipe isn't open.
		return -1;
	}
	
	// Close the pipe.
	fclose(fle);
	
	CloseHandle(my_pipeerr[0]);
	if (my_popenmode == 'r')
		CloseHandle(my_pipein[1]);
	else
		CloseHandle(my_pipeout[0]);
	
	// Terminate the child process.
	if (piProcInfo.hProcess)
	{
		TerminateProcess(piProcInfo.hProcess, 0);
		piProcInfo.hProcess = NULL;
	}
	
	return 0;
}
