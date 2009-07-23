/***************************************************************************
 * Gens: Signal handler.                                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "sighandler.h"

// Message logging.
#include "macros/log_msg.h"

// C includes.
#include <stdlib.h>
#include <signal.h>

static void gens_sighandler(int signum);


typedef struct _gens_signal_t
{
	int signum;
	const char *signame;
	const char *sigdesc;
} gens_signal_t;

static const gens_signal_t gens_signals[] =
{
	// Signal ordering from /usr/include/bits/signum.h (glibc-2.9_p20081201-r2)
	
#ifdef SIGHUP
	{SIGHUP, "SIGHUP", "Hangup"},
#endif
#ifdef SIGILL
	{SIGILL, "SIGILL", "Illegal instruction"},
#endif
#ifdef SIGABRT
	{SIGABRT, "SIGABRT", "Aborted"},
#endif
#ifdef SIGBUS
	{SIGBUS, "SIGBUS", "Bus error"},
#endif
#ifdef SIGFPE
	{SIGFPE, "SIGFPE", "Floating-point exception"},
#endif
#ifdef SIGUSR1
	{SIGUSR1, "SIGUSR1", "User-defined signal 1"},
#endif
#ifdef SIGSEGV
	{SIGSEGV, "SIGSEGV", "Segmentation fault"},
#endif
#ifdef SIGUSR2
	{SIGUSR2, "SIGUSR2", "User-defined signal 2"},
#endif
#ifdef SIGPIPE
	{SIGPIPE, "SIGPIPE", "Broken pipe"},
#endif
#ifdef SIGALRM
	{SIGALRM, "SIGALRM", "Alarm clock"},
#endif
#ifdef SIGSTKFLT
	{SIGSTKFLT, "SIGSTKFLT", "Stack fault"},
#endif
#ifdef SIGXCPU
	{SIGXCPU, "SIGXCPU", "CPU limit exceeded"},
#endif
#ifdef SIGXFSZ
	{SIGXFSZ, "SIGXFSZ", "File size limit exceeded"},
#endif
#ifdef SIGVTALRM
	{SIGVTALRM, "SIGVTALRM", "Virtual alarm clock"},
#endif
#ifdef SIGPWR
	{SIGPWR, "SIGPWR", "Power failure"},
#endif
#ifdef SIGSYS
	{SIGSYS, "SIGSYS", "Bad system call"},
#endif
};


/**
 * gens_sighandler_init(): Initialize the signal handler.
 */
void gens_sighandler_init(void)
{
	unsigned int i;
	
	for (i = 0; i < (sizeof(gens_signals) / sizeof(gens_signal_t)); i++)
	{
		signal(gens_signals[i].signum, gens_sighandler);
	}
}


/**
 * gens_sighandler_end(): Shut down the signal handler.
 */
void gens_sighandler_end(void)
{
	unsigned int i;
	
	for (i = 0; i < (sizeof(gens_signals) / sizeof(gens_signal_t)); i++)
	{
		signal(gens_signals[i].signum, SIG_DFL);
	}
}


/**
 * gens_sighandler(): Signal handler.
 * @param signum Signal number.
 */
static void gens_sighandler(int signum)
{
	if (
#ifdef SIGHUP
	    signum == SIGHUP ||
#endif
#ifdef SIGUSR1
	    signum == SIGUSR1 ||
#endif
#ifdef SIGUSR2
	    signum == SIGUSR2 ||
#endif
	    0)
	{
		// SIGHUP, SIGUSR1, SIGUSR2. Ignore this signal.
		const char *signame;
		
#ifdef SIGHUP
		if (signum == SIGHUP)
			signame = "SIGHUP";
		else
#endif
#ifdef SIGUSR1
		if (signum == SIGUSR1)
			signame = "SIGUSR1";
		else
#endif
#ifdef SIGUSR2
		if (signum == SIGUSR2)
			signame = "SIGUSR2";
		else
#endif
		signame = "UNKNOWN";
		
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Signal %d (%s) received; ignoring.", signum, signame);
		return;
	}
	
	// Check what signal this is.
	const char *signame;
	const char *sigdesc;
	unsigned int i;
	for (i = 0; i < (sizeof(gens_signals) / sizeof(gens_signal_t)); i++)
	{
		if (gens_signals[i].signum == signum)
		{
			signame = gens_signals[i].signame;
			sigdesc = gens_signals[i].sigdesc;
			break;
		}
	}
	
	if (i == (sizeof(gens_signals) / sizeof(gens_signal_t)))
	{
		signame = "SIGUNKNOWN";
		sigdesc = "Unknown signal";
	}
	
	// This uses LOG_MSG_LEVEL_ERROR in order to suppress the message box.
	LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
		"Signal %d (%s) received. Shutting down.", signum, signame);
	
	// Show a message box.
	char buf[512];
	snprintf(buf, sizeof(buf),
		 "Gens/GS has crashed with Signal %d.\n%s: %s.\n\n"
		 "Please report this error to GerbilSoft (gerbilsoft@verizon.net).\n"
		 "Be sure to include detailed instructions about what you were\n"
		 "doing when this error occurred.",
		 signum, signame, sigdesc);
	buf[sizeof(buf)-1] = 0x00;
	
	log_msgbox(buf, "Gens/GS Error");
	
	exit(0);
}
