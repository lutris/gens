/***************************************************************************
 * Gens: RAR Decompressor. (Win32)                                         *
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

#include "md_rar_win32.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// Win32 includes.
#include "libgsft/w32u/w32u.h"
#include "libgsft/w32u/w32u_libc.h"
#include "libgsft/w32u/w32u_priv.h"
#include <winnls.h>

// C includes.
#include <unistd.h>
#include <stdlib.h>

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_dlopen.h"

// libgsft includes.
#include "libgsft/gsft_strdup.h"
#include "libgsft/gsft_unused.h"
#include "libgsft/gsft_szprintf.h"


// RAR (Win32) decompressor functions.
static int decompressor_rar_win32_detect_format(FILE *zF);
static int decompressor_rar_win32_get_file_info(FILE *zF, const char* filename,
						mdp_z_entry_t** z_entry_out);
static size_t decompressor_rar_win32_get_file(FILE *zF, const char* filename,
						mdp_z_entry_t *z_entry,
						void *buf, const size_t size);

// RAR (Win32) decompressor struct.
const decompressor_t decompressor_rar_win32 =
{
	.detect_format	= decompressor_rar_win32_detect_format,
	.get_file_info	= decompressor_rar_win32_get_file_info,
	.get_file	= decompressor_rar_win32_get_file
};


// UnRAR.dll
#include "unrar.h"
static void *hUnrarDll = NULL;
static int unrar_refcnt = 0;
MAKE_STFUNCPTR(RAROpenArchiveEx);
MAKE_STFUNCPTR(RARCloseArchive);
MAKE_STFUNCPTR(RARReadHeaderEx);
MAKE_STFUNCPTR(RARProcessFile);
MAKE_STFUNCPTR(RARProcessFileW);
MAKE_STFUNCPTR(RARGetDllVersion);

#define InitFuncPtr_unrar(hDll, fn) p##fn = (typeof(p##fn))mdp_dlsym((hDll), #fn)

/**
 * unrar_dll_init(): Initialize UnRAR.dll.
 * @return 0 on success; non-zero on error.
 */
static int unrar_dll_init(void)
{
	if (unrar_refcnt++ != 0)
		return 0;
	
	// Load the DLL.
	hUnrarDll = mdp_dlopen("unrar.dll");
	if (!hUnrarDll)
		return -2;
	
	// Load the function pointers.
	// TODO: Return an error if any of them fail.
	InitFuncPtr_unrar(hUnrarDll, RAROpenArchiveEx);
	InitFuncPtr_unrar(hUnrarDll, RARCloseArchive);
	InitFuncPtr_unrar(hUnrarDll, RARReadHeaderEx);
	InitFuncPtr_unrar(hUnrarDll, RARProcessFile);
	InitFuncPtr_unrar(hUnrarDll, RARProcessFileW);
	InitFuncPtr_unrar(hUnrarDll, RARGetDllVersion);
	
	// Increment the reference counter and return.
	unrar_refcnt++;
	return 0;
}

/**
 * unrar_dll_end(): Shut down UnRAR.dll.
 * @return 0 on success; non-zero on error.
 */
static int unrar_dll_end(void)
{
	if (unrar_refcnt <= 0)
		return 0;
	
	unrar_refcnt--;
	if (unrar_refcnt != 0)
		return 0;
	
	// Unload the DLL.
	mdp_dlclose(hUnrarDll);
	hUnrarDll = NULL;
	
	// Clear the function pointers.
	pRAROpenArchiveEx	= NULL;
	pRARCloseArchive	= NULL;
	pRARReadHeaderEx	= NULL;
	pRARProcessFile		= NULL;
	pRARProcessFileW	= NULL;
	pRARGetDllVersion	= NULL;
	
	return 0;
}


/**
 * decompressor_rar_win32_detect_format(): Detect if this file can be handled by this decompressor.
 * @param zF Open file handle.
 * @return Non-zero if this file can be handled; 0 if it can't be.
 */
int decompressor_rar_win32_detect_format(FILE *zF)
{
	// Magic Number for RAR:
	// First four bytes: "Rar!"
	static const unsigned char rar_magic[] = {'R', 'a', 'r', '!'};
	
	unsigned char buf[sizeof(rar_magic)];
	fseek(zF, 0, SEEK_SET);
	fread(buf, sizeof(unsigned char), sizeof(rar_magic), zF);
	return (memcmp(buf, rar_magic, sizeof(rar_magic)) == 0);
}


/**
 * decompressor_rar_win32_get_file_info(): Get information about all files in the archive.
 * @param zF		[in] Open file handle.
 * @param filename	[in] Filename of the archive.
 * @param z_entry_out	[out] Pointer to mdp_z_entry_t*, which will contain an allocated mdp_z_entry_t.
 * @return MDP error code.
 */
int decompressor_rar_win32_get_file_info(FILE *zF, const char* filename, mdp_z_entry_t** z_entry_out)
{
	// Initialize UnRAR.dll.
	if (unrar_dll_init() != 0)
	{
		// Error initializing UnRAR.dll.
		return -MDP_ERR_Z_EXE_NOT_FOUND;
	}
	
	HANDLE hRAR;
	wchar_t *filenameW = NULL;
	BOOL doUnicode = (isSendMessageUnicode && pMultiByteToWideChar && pWideCharToMultiByte);
	
	// Open the RAR file.
	struct RAROpenArchiveDataEx rar_open;
	rar_open.OpenMode = RAR_OM_LIST;
	rar_open.CmtBuf = NULL;
	rar_open.CmtBufSize = 0;
	
	if (doUnicode)
	{
		// Unicode mode.
		filenameW = w32u_mbstowcs(filename);
		rar_open.ArcName = NULL;
		rar_open.ArcNameW = filenameW;
	}
	else
	{
		// ANSI mode.
		// TODO: Make a copy of filename, since rar_open.ArcName isn't const.
		rar_open.ArcName = filename;
		rar_open.ArcNameW = NULL;
	}
	
	hRAR = pRAROpenArchiveEx(&rar_open);
	if (!hRAR)
	{
		// Error opening the RAR file.
		free(filenameW);
		return -MDP_ERR_Z_CANT_OPEN_ARCHIVE;
	}
	
	// File list pointers.
	mdp_z_entry_t *z_entry_head = NULL;
	mdp_z_entry_t *z_entry_end = NULL;
	
	// Process the archive.
	struct RARHeaderDataEx rar_header;
	char utf8_buf[1024*4];
	int ret = 0;
	while ((ret = pRARReadHeaderEx(hRAR, &rar_header)) == 0)
	{
		// Allocate memory for the file.
		mdp_z_entry_t *z_entry_cur = (mdp_z_entry_t*)malloc(sizeof(*z_entry_cur));
		
		// Store the ROM file information.
		// TODO: What do we do if rar_header.UnpSizeHigh is set (indicating >4 GB)?
		z_entry_cur->filesize = rar_header.UnpSize;
		z_entry_cur->next = NULL;
		
		// Check which filename should be used.
		if (doUnicode)
		{
			// Use the Unicode filename. (rar_header.FileNameW)
			pWideCharToMultiByte(CP_UTF8, 0, rar_header.FileNameW, -1,
						utf8_buf, sizeof(utf8_buf), NULL, NULL);
			z_entry_cur->filename = strdup(utf8_buf);
		}
		else
		{
			// Use the ANSI filename. (rar_header.FileName)
			z_entry_cur->filename = strdup(rar_header.FileName);
		}
		
		if (!z_entry_head)
		{
			// List hasn't been created yet. Create it.
			z_entry_head = z_entry_cur;
			z_entry_end = z_entry_cur;
		}
		else
		{
			// Append the file list entry to the end of the list.
			z_entry_end->next = z_entry_cur;
			z_entry_end = z_entry_cur;
		}
		
		// Go to the next file.
		if (pRARProcessFile(hRAR, RAR_SKIP, NULL, NULL) != 0)
			break;
	}
	
	// Close the RAR file.
	pRARCloseArchive(hRAR);
	free(filenameW);
	
	// Shut down UnRAR.dll.
	unrar_dll_end();
	
	// Return the list of files.
	*z_entry_out = z_entry_head;
	return MDP_ERR_OK;
}


/**
 * decompressor_rar_win32_get_file(): Get a file from the archive.
 * @param zF Open file handle. (Unused in the RAR handler.)
 * @param filename Filename of the archive.
 * @param z_entry Pointer to mdp_z_entry_t element to get from the archive.
 * @param buf Buffer to read the file into.
 * @param size Size of buf (in bytes).
 * @return Number of bytes read, or 0 on error.
 */
size_t decompressor_rar_win32_get_file(FILE *zF, const char *filename,
					mdp_z_entry_t *z_entry,
					void *buf, const size_t size)
{
	return -1;
#if 0
	GSFT_UNUSED_PARAMETER(zF);
	
	// All parameters (except zF) must be specified.
	if (!filename || !z_entry || !buf || !size)
		return 0;
	
	// Check that the RAR executable is available.
#if !defined(_WIN32)
	if (access(Misc_Filenames.RAR_Binary, X_OK) != 0)
#else
	if (access(Misc_Filenames.RAR_Binary, R_OK) != 0)
#endif
	{
		// Cannot run the RAR executable.
		// TODO: Show an error message and/or return an error code.
		return 0;
	}
	
	char cmd_line[GENS_PATH_MAX*3 + 256];
	szprintf(cmd_line, sizeof(cmd_line), "\"%s\" p -ierr  \"%s\" \"%s\"%s",
		 Misc_Filenames.RAR_Binary, filename, z_entry->filename,
#if !defined(GENS_OS_WIN32)
		" 2>/dev/null"
#else
		""
#endif
		);
	
	// Open the RAR file.
	FILE *pRAR = popen(cmd_line, "r");
	if (!pRAR)
	{
		// Error opening `rar`.
		// TODO: Show an error message and/or return an error code.
		return 0;
	}
	
	// Read from the pipe.
	size_t extracted_size = 0;
	size_t rv;
	unsigned char bufRAR[4096];
	while ((rv = fread(bufRAR, 1, sizeof(bufRAR), pRAR)))
	{
		if (extracted_size + rv > size)
			break;
		memcpy(&((unsigned char*)buf)[extracted_size], &bufRAR, rv);
		extracted_size += rv;
	}
	pclose(pRAR);
	
	// Return the filesize.
	return extracted_size;
#endif
}
