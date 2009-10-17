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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sighandler.h"

#include "macros/log_msg.h"
#include "macros/git.h"

// libgsft includes.
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_szprintf.h"

// C includes.
#include <stdlib.h>
#include <signal.h>


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
	{0, NULL, NULL}
};


#ifdef HAVE_SIGACTION
static void gens_sighandler(int signum, siginfo_t *info, void *context);
#else
static void gens_sighandler(int signum);
#endif


/**
 * gens_sighandler_init(): Initialize the signal handler.
 */
void gens_sighandler_init(void)
{
	unsigned int i;
	
#ifdef HAVE_SIGACTION
	struct sigaction sa;
	sa.sa_sigaction = gens_sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
#endif
	
	for (i = 0; i < (sizeof(gens_signals) / sizeof(gens_signal_t)); i++)
	{
#ifdef HAVE_SIGACTION
		sigaction(gens_signals[i].signum, &sa, NULL);
#else
		signal(gens_signals[i].signum, gens_sighandler);
#endif
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


#ifdef HAVE_SIGACTION
/**
 * gens_get_siginfo(): Get the signal information from a received signal.
 * @param signum	[in] Signal number.
 * @param si_code	[in] Signal information code.
 * @return Pointer to gens_signal_t, or NULL if not found.
 */
static const gens_signal_t *gens_get_siginfo(int signum, int si_code)
{
	// Check if there's any information associated with the signal.
	
#ifdef SIGILL
	// SIGILL information.
	static const gens_signal_t siginfo_SIGILL[] =
	{
		{ILL_ILLOPC,	"ILL_ILLOPC",	"Illegal opcode"},
		{ILL_ILLOPN,	"ILL_ILLOPN",	"Illegal operand"},
		{ILL_ILLADR,	"ILL_ILLADR",	"Illegal addressing mode"},
		{ILL_ILLTRP,	"ILL_ILLTRP",	"Illegal trap"},
		{ILL_PRVOPC,	"ILL_PRVOPC",	"Privileged opcode"},
		{ILL_PRVREG,	"ILL_PRVREG",	"Privileged register"},
		{ILL_COPROC,	"ILL_COPROC",	"Coprocessor error"},
		{ILL_BADSTK,	"ILL_BADSTK",	"Internal stack error"},
		{0, NULL, NULL}
	};
#endif
	
#ifdef SIGFPE
	// SIGFPE information.
	static const gens_signal_t siginfo_SIGFPE[] =
	{
		{FPE_INTDIV,	"FPE_INTDIV",	"Integer divide by zero"},
		{FPE_INTOVF,	"FPE_INTOVF",	"Integer overflow"},
		{FPE_FLTDIV,	"FPE_FLTDIV",	"Floating-point divide by zero"},
		{FPE_FLTOVF,	"FPE_FLTOVF",	"Floating-point overflow"},
		{FPE_FLTUND,	"FPE_FLTUND",	"Floating-point underflow"},
		{FPE_FLTRES,	"FPE_FLTRES",	"Floating-point inexact result"},
		{FPE_FLTINV,	"FPE_FLTINV",	"Floating-point invalid operation"},
		{FPE_FLTSUB,	"FPE_FLTSUB",	"Subscript out of range"},
		{0, NULL, NULL}
	};
#endif
	
#ifdef SIGSEGV
	// SIGSEGV information.
	static const gens_signal_t siginfo_SIGSEGV[] =
	{
		{SEGV_MAPERR,	"SEGV_MAPERR",	"Address not mapped to object"},
		{SEGV_ACCERR,	"SEGV_ACCERR",	"Invalid permissions for mapped object"},
		{0, NULL, NULL}
	};
#endif
	
#ifdef SIGBUS
	// SIGBUS information.
	static const gens_signal_t siginfo_SIGBUS[] =
	{
		{BUS_ADRALN,	"BUS_ADRALN",	"Invalid address alignment"},
		{BUS_ADRERR,	"BUS_ADRERR",	"Nonexistent physical address"},
		{BUS_OBJERR,	"BUS_OBJERR",	"Object-specific hardware error"},
		{0, NULL, NULL}
	};
#endif
	
	const gens_signal_t *siginfo;
	
	switch (signum)
	{
#ifdef SIGILL
		case SIGILL:
			siginfo = &siginfo_SIGILL[0];
			break;
#endif
#ifdef SIGFPE
		case SIGFPE:
			siginfo = &siginfo_SIGFPE[0];
			break;
#endif
#ifdef SIGSEGV
		case SIGSEGV:
			siginfo = &siginfo_SIGSEGV[0];
			break;
#endif
#ifdef SIGBUS
		case SIGBUS:
			siginfo = &siginfo_SIGBUS[0];
			break;
#endif
		default:
			siginfo = NULL;
			break;
	}
	
	if (!siginfo)
		return NULL;
	
	// Check for signal information.
	for (; siginfo->signum != 0; siginfo++)
	{
		if (siginfo->signum == si_code)
			return siginfo;
	}
	
	// No signal information was found.
	return NULL;
}
#endif


/**
 * gens_sighandler(): Signal handler.
 * @param signum	[in] Signal number.
 * @param info		[in] Signal information. (ONLY if sigaction() is available.)
 * @param context	[in] Context. (ONLY if sigaction() is available.)
 */
#ifdef HAVE_SIGACTION
static void gens_sighandler(int signum, siginfo_t *info, void *context)
#else
static void gens_sighandler(int signum)
#endif
{
#ifdef HAVE_SIGACTION
	GSFT_UNUSED_PARAMETER(context);
#endif
	
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
		
		switch (signum)
		{
#ifdef SIGHUP
			case SIGHUP:
				signame = "SIGHUP";
				break;
#endif
#ifdef SIGUSR1
			case SIGUSR1:
				signame = "SIGUSR1";
				break;
#endif
#ifdef SIGUSR2
			case SIGUSR2:
				signame = "SIGUSR2";
				break;
#endif
			default:
				signame = "UNKNOWN";
				break;
		}
		
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
	
#ifdef HAVE_SIGACTION
	// Note: If context is NULL, then info is invalid.
	// This may happen if SIGILL is sent to the program via kill/pkill/killall.
	const gens_signal_t *siginfo = NULL;
	if (info && context)
		siginfo = gens_get_siginfo(signum, info->si_code);
	
	char siginfo_buf[256];
	if (siginfo)
	{
		// Signal information specified.
		szprintf(siginfo_buf, sizeof(siginfo_buf),
				"%s: %s.\n",
				siginfo->signame, siginfo->sigdesc);
	}
	else
	{
		// No signal information specified.
		siginfo_buf[0] = 0x00;
	}
#endif
	
	// This uses LOG_MSG_LEVEL_ERROR in order to suppress the message box.
#ifdef HAVE_SIGACTION
	if (siginfo)
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Signal %d (%s: %s) received. Shutting down.",
			signum, signame, siginfo->signame);
	}
	else
#endif
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Signal %d (%s) received. Shutting down.",
			signum, signame);
	}
	
	// Show a message box.
	char msg_buf[1024];
	szprintf(msg_buf, sizeof(msg_buf),
			"Gens/GS has crashed with Signal %d.\n"
			"%s: %s.\n"
#ifdef HAVE_SIGACTION
			"%s"
#endif
			"\n"
			"Build Information:\n"
			"- Platform: " GENS_PLATFORM "\n"
			"- Version: " PACKAGE_VERSION " (" GENS_GS_VERSION ")\n"
#ifdef GENS_GIT_VERSION
			"- " GENS_GIT_VERSION "\n"
#endif
			"\n"
			"Please report this error to GerbilSoft (gerbilsoft@verizon.net).\n"
			"Be sure to include detailed instructions about what you were\n"
			"doing when this error occurred.",
			signum, signame, sigdesc
#ifdef HAVE_SIGACTION
			, siginfo_buf
#endif
			);
	
	log_msgbox(msg_buf, "Gens/GS Error");
	
	exit(EXIT_FAILURE);
}
