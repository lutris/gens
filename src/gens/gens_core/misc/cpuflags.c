/***************************************************************************
 * Gens: CPU Flags.                                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "cpuflags.h"

#include <stdint.h>

// MDP CPU flag definitions.
#include "mdp/mdp_cpuflags.h"

// IA32 CPU flags
// Intel: http://download.intel.com/design/processor/applnots/24161832.pdf
// AMD: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf

// CPUID function 1: Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EDX_MMX		((uint32_t)(1 << 23))
#define CPUFLAG_IA32_EDX_SSE		((uint32_t)(1 << 25))
#define CPUFLAG_IA32_EDX_SSE2		((uint32_t)(1 << 26))

// Flags stored in the ecx register.
#define CPUFLAG_IA32_ECX_SSE3		((uint32_t)(1 << 0))
#define CPUFLAG_IA32_ECX_SSSE3		((uint32_t)(1 << 9))
#define CPUFLAG_IA32_ECX_SSE41		((uint32_t)(1 << 19))
#define CPUFLAG_IA32_ECX_SSE42		((uint32_t)(1 << 20))

// CPUID function 0x80000001: Extended Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EXT_EDX_MMXEXT	((uint32_t)(1 << 22))
#define CPUFLAG_IA32_EXT_EDX_3DNOW	((uint32_t)(1 << 31))
#define CPUFLAG_IA32_EXT_EDX_3DNOWEXT	((uint32_t)(1 << 30))

// Flags stored in the ecx register.
#define CPUFLAG_IA32_EXT_ECX_SSE4A	((uint32_t)(1 << 6))

// CPUID functions.
#define CPUID_MAX_FUNCTIONS		((uint32_t)(0x00000000))
#define CPUID_FAMILY_FEATURES		((uint32_t)(0x00000001))
#define CPUID_MAX_EXT_FUNCTIONS		((uint32_t)(0x80000000))
#define CPUID_EXT_FAMILY_FEATURES	((uint32_t)(0x80000001))

// CPUID macro with PIC support.
// See http://gcc.gnu.org/ml/gcc-patches/2007-09/msg00324.html
#if defined(__i386__) && defined(__PIC__)
#define __cpuid(level, a, b, c, d)				\
	__asm__ (						\
		"xchgl	%%ebx, %1\n"				\
		"cpuid\n"					\
		"xchgl	%%ebx, %1\n"				\
		: "=a" (a), "=r" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		)
#else
#define __cpuid(level, a, b, c, d)				\
	__asm__ (						\
		"cpuid\n"					\
		: "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
		: "0" (level)					\
		)
#endif

// CPU Flags
uint32_t CPU_Flags = 0;

// Function to check if the OS supports SSE.
static int check_os_level_sse(void);
#define CPUFLAG_X86_SSE_ALL \
	(MDP_CPUFLAG_X86_SSE2 | \
	 MDP_CPUFLAG_X86_SSE3 | \
	 MDP_CPUFLAG_X86_SSSE3 | \
	 MDP_CPUFLAG_X86_SSE41 | \
	 MDP_CPUFLAG_X86_SSE42 | \
	 MDP_CPUFLAG_X86_SSE4A)

/**
 * getCPUFlags(): Get the CPU flags.
 * @return CPU flags.
 */
uint32_t getCPUFlags(void)
{
#if !defined(GENS_X86_ASM)
	
	// x86 asm code has been turned off.
	// Don't check for any CPU flags.
	return 0;
	
#elif defined(__i386__) || defined(__amd64__)
	// IA32/x86_64.
	
	// Check if cpuid is supported.
	unsigned int _eax, _ebx, _ecx, _edx;
	
#if defined(__i386__)
	__asm__ (
		"pushfl\n"
		"popl %%eax\n"
		"movl %%eax, %%edx\n"
		"xorl $0x200000, %%eax\n"
		"pushl %%eax\n"
		"popfl\n"
		"pushfl\n"
		"popl %%eax\n"
		"xorl %%edx, %%eax\n"
		"andl $0x200000, %%eax"
		:	"=a" (_eax)	// Output
		);
#else /* defined(__amd64__) */
	__asm__ (
		"pushfq\n"
		"popq %%rax\n"
		"movl %%eax, %%edx\n"
		"xorl $0x200000, %%eax\n"
		"pushq %%rax\n"
		"popfq\n"
		"pushfq\n"
		"popq %%rax\n"
		"xorl %%edx, %%eax\n"
		"andl $0x200000, %%eax"
		:	"=a" (_eax)	// Output
		);
#endif
	
	if (!_eax)
	{
		// CPUID is not supported.
		// This CPU must be a 486 or older.
		return 0;
	}
	
	// CPUID is supported.
	// Check if the CPUID Features function (Function 1) is supported.
	unsigned int maxFunc;
	__cpuid(CPUID_MAX_FUNCTIONS, maxFunc, _ebx, _ecx, _edx);
	
	if (!maxFunc)
	{
		// No CPUID functions are supported.
		return 0;
	}
	
	// Get the CPU feature flags.
	__cpuid(CPUID_FAMILY_FEATURES, _eax, _ebx, _ecx, _edx);
	
	// Check the feature flags.
	CPU_Flags = 0;
	
	if (_edx & CPUFLAG_IA32_EDX_MMX)
		CPU_Flags |= MDP_CPUFLAG_X86_MMX;
	if (_edx & CPUFLAG_IA32_EDX_SSE)
	{
		CPU_Flags |= MDP_CPUFLAG_X86_SSE;
		
		// MMXext is a subset of SSE.
		// See http://www.x86-64.org/pipermail/patches/2005-March/003261.html
		CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;
	}
	if (_edx & CPUFLAG_IA32_EDX_SSE2)
		CPU_Flags |= MDP_CPUFLAG_X86_SSE2;
	if (_ecx & CPUFLAG_IA32_ECX_SSE3)
		CPU_Flags |= MDP_CPUFLAG_X86_SSE3;
	if (_ecx & CPUFLAG_IA32_ECX_SSSE3)
		CPU_Flags |= MDP_CPUFLAG_X86_SSSE3;
	if (_ecx & CPUFLAG_IA32_ECX_SSE41)
		CPU_Flags |= MDP_CPUFLAG_X86_SSE41;
	if (_ecx & CPUFLAG_IA32_ECX_SSE42)
		CPU_Flags |= MDP_CPUFLAG_X86_SSE42;
	
	// Check if the CPUID Extended Features function (Function 0x80000001) is supported.
	__cpuid(CPUID_MAX_EXT_FUNCTIONS, maxFunc, _ebx, _ecx, _edx);
	if (maxFunc >= CPUID_EXT_FAMILY_FEATURES)
	{
		// CPUID Extended Features are supported.
		__cpuid(CPUID_EXT_FAMILY_FEATURES, _eax, _ebx, _ecx, _edx);
		
		// Check the extended feature flags.
		if (_edx & CPUFLAG_IA32_EXT_EDX_MMXEXT)
			CPU_Flags |= MDP_CPUFLAG_X86_MMXEXT;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOW)
			CPU_Flags |= MDP_CPUFLAG_X86_3DNOW;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOWEXT)
			CPU_Flags |= MDP_CPUFLAG_X86_3DNOWEXT;
		if (_ecx & CPUFLAG_IA32_EXT_ECX_SSE4A)
			CPU_Flags |= MDP_CPUFLAG_X86_SSE4A;
	}
	
	// If the CPU claims it supports an SSE instruction set,
	// make sure the operating system supports it, too.
	if (CPU_Flags & CPUFLAG_X86_SSE_ALL)
	{
		if (!check_os_level_sse())
		{
			// Operating system does not support SSE.
			// Disable all SSE flags.
			CPU_Flags &= ~CPUFLAG_X86_SSE_ALL;
		}
	}
	
	// Return the CPU flags.
	return CPU_Flags;
	
#else
	// No flags for this CPU.
	return 0;
	
#endif
}


#if defined(GENS_X86_ASM) && (defined(__i386__) || defined(__amd64__))

// Function to check for OS-level SSE support.

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

/**
 * check_os_level_sse(): Check for OS-level SSE support. (Win32 version.)
 * @return 0 if SSE isn't supported; non-zero if SSE is supported.
 */
static int check_os_level_sse(void)
{
	/* SSE is supported in the following versions of Windows:
	 * - Windows 98 (4.10.1998) and later 9x
	 * - Windows 2000 (5.0.2195)
	 * Hence, checking for Windows older than 4.10 is sufficient.
	 *
	 * TODO: Some sources say Windows 98 First Edition support SSE with
	 * a DirectX upgrade, while others say Windows 98 Second Edition is
	 * required for SSE support. For now, I'll enable it on both versions
	 * of Windows 98.
	 */
	const DWORD dwVersion = GetVersion();
	
	if (LOBYTE(LOWORD(dwVersion)) >= 5 ||
	    (LOBYTE(LOWORD(dwVersion)) == 4 && HIBYTE(LOWORD(dwVersion)) >= 10))
	{
		// Windows 4.10 or later.
		// SSE is supported.
		return 1;
	}
	else
	{
		// Older than Windows 4.10.
		// SSE is not supported.
		return 0;
	}
}

#else /* !defined(_WIN32) */

// POSIX version.
#include <signal.h>

// SSE Error variable.
// If this is set, then the SSE test function failed.
static int SSE_OS_Support;

static void check_os_level_sse_sighandler(int signum)
{
	// If this function is called, it means SIGILL was thrown
	// after an SSE instruction was executed in getCPUFlags().
	// This means the OS doesn't support SSE.
	if (signum == SIGILL)
		SSE_OS_Support = 0;
}

static int check_os_level_sse(void)
{
	// Set the temporary signal handler.
	sighandler_t prev_SIGILL = signal(SIGILL, check_os_level_sse_sighandler);
	
	// Assume SSE is supported initially.
	// If the CPU doesn't support SSE, then the above signal handler
	// will set SSE_OS_Support to 0.
	SSE_OS_Support = 1;
	
	// Attempt to execute an SSE instruction.
	__asm__ (
		"orps	%xmm0, %xmm0"
		);
	
	// Restore the signal handler.
	signal(SIGILL, prev_SIGILL);
	
	return SSE_OS_Support;
}

#endif

#else /* !(defined(GENS_X86_ASM) && (defined(__i386__) || defined(__amd64__))) */

// SSE is not available.

/**
 * check_os_level_sse(): Check for OS-level SSE support. (Win32 version.)
 * @return 0, since SSE isn't available.
 */
static int check_os_level_sse(void)
{
	return 0;
}

#endif /* defined(GENS_X86_ASM) && (defined(__i386__) || defined(__amd64__)) */
