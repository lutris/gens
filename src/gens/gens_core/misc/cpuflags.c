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

#include "cpuflags.h"

// IA32 CPU flags
// Intel: http://download.intel.com/design/processor/applnots/24161832.pdf
// AMD: http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf

// CPUID function 1: Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EDX_MMX		(1 << 23)
#define CPUFLAG_IA32_EDX_SSE		(1 << 25)
#define CPUFLAG_IA32_EDX_SSE2		(1 << 26)

// Flags stored in the ecx register.
#define CPUFLAG_IA32_ECX_SSE3		(1 << 0)
#define CPUFLAG_IA32_ECX_SSSE3		(1 << 9)
#define CPUFLAG_IA32_ECX_SSE41		(1 << 19)
#define CPUFLAG_IA32_ECX_SSE42		(1 << 20)

// CPUID function 0x80000001: Extended Family & Features

// Flags stored in the edx register.
#define CPUFLAG_IA32_EXT_EDX_MMXEXT	(1 << 22)
#define CPUFLAG_IA32_EXT_EDX_3DNOW	(1 << 31)
#define CPUFLAG_IA32_EXT_EDX_3DNOWEXT	(1 << 30)

// Flags stored in the ecx register.
#define CPUFLAG_IA32_EXT_ECX_SSE4A	(1 << 6)
#define CPUFLAG_IA32_EXT_ECX_SSE5	(1 << 11)


// CPU Flags
unsigned int CPU_Flags = 0;


/**
 * getCPUFlags(): Get the CPU flags.
 * @return CPU flags.
 */
unsigned int getCPUFlags(void)
{
#if !defined(GENS_X86_ASM)
	
	// x86 asm code has been turned off.
	// Don't check for any CPU flags.
	return 0;
	
#elif defined(__i386__) || defined(__amd64__)
	// IA32/x86_64.
	
	// Check if cpuid is supported.
	unsigned int _eax;
	
	__asm__ (
		"pushfl\n"
		"popl %%eax\n"
		"movl %%eax, %%ebx\n"
		"xorl $0x200000, %%eax\n"
		"pushl %%eax\n"
		"popfl\n"
		"pushfl\n"
		"popl %%eax\n"
		"xorl %%ebx, %%eax\n"
		"andl $0x200000, %%eax"
		:	"=a" (_eax)	// Output
		:			// Input
		:	"ebx"		// Clobber
		);
	
	if (!_eax)
	{
		// CPUID is not supported.
		// This CPU must be a 486 or older.
		return 0;
	}
	
	// CPUID is supported.
	// Check if the CPUID Feature function (Function 1) is supported.
	unsigned int maxFunc;
	__asm__ (
		"xorl %%eax, %%eax\n"
		"cpuid\n"		// Get number of CPUID functions.
		:	"=a" (maxFunc)	// Output
		);
	
	if (!maxFunc)
	{
		// No CPUID functions are supported.
		return 0;
	}
	
	// Get the CPU feature flags.
	unsigned int _edx, _ecx;
	__asm__ (
		"movl $1, %%eax\n"	// CPUID function 1: family and features
		"cpuid\n"
		:	"=d" (_edx), "=c" (_ecx)	// Output
		:					// Input
		:	"eax", "ebx"			// Clobber
		);
	
	// Check the feature flags.
	CPU_Flags = 0;
	
	if (_edx & CPUFLAG_IA32_EDX_MMX)
		CPU_Flags |= CPUFLAG_MMX;
	if (_edx & CPUFLAG_IA32_EDX_SSE)
		CPU_Flags |= CPUFLAG_SSE;
	if (_edx & CPUFLAG_IA32_EDX_SSE2)
		CPU_Flags |= CPUFLAG_SSE2;
	if (_ecx & CPUFLAG_IA32_ECX_SSE3)
		CPU_Flags |= CPUFLAG_SSE3;
	if (_ecx & CPUFLAG_IA32_ECX_SSSE3)
		CPU_Flags |= CPUFLAG_SSSE3;
	if (_ecx & CPUFLAG_IA32_ECX_SSE41)
		CPU_Flags |= CPUFLAG_SSE41;
	if (_ecx & CPUFLAG_IA32_ECX_SSE42)
		CPU_Flags |= CPUFLAG_SSE42;
	
	// Get the extended CPU feature flags.
	if (maxFunc >= 0x80000001)
	{
		__asm__ (
			"movl $0x80000001, %%eax\n"	// CPUID function 0x80000001: extended family and features
			"cpuid\n"
			:	"=d" (_edx), "=c" (_ecx)	// Output
			:					// Input
			:	"eax", "ebx"			// Clobber
			);
		
		// Check the extended feature flags.
		if (_edx & CPUFLAG_IA32_EXT_EDX_MMXEXT)
			CPU_Flags |= CPUFLAG_MMXEXT;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOW)
			CPU_Flags |= CPUFLAG_3DNOW;
		if (_edx & CPUFLAG_IA32_EXT_EDX_3DNOWEXT)
			CPU_Flags |= CPUFLAG_3DNOWEXT;
		if (_ecx & CPUFLAG_IA32_EXT_ECX_SSE4A)
			CPU_Flags |= CPUFLAG_SSE4A;
		if (_ecx & CPUFLAG_IA32_EXT_ECX_SSE5)
			CPU_Flags |= CPUFLAG_SSE5;
	}
	
	// Return the CPU flags.
	return CPU_Flags;
	
#else
	// No flags for this CPU.
	return 0;
	
#endif
}
