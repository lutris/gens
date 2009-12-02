/***************************************************************************
 * Gens: argc/argv conversion functions.                                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * Original code from the CMake mailing list:                              *
 * http://www.cmake.org/pipermail/cmake/2004-June/005172.html              *
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

#include "argc_argv.h"

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"

#include <stdlib.h>
#include <string.h>


/**
 * convertCmdLineToArgv: Convert lpCmdLine to argc and argv.
 * Originally from http://www.cmake.org/pipermail/cmake/2004-June/005172.html
 * @param lpCmdLine Command line.
 * @param arg Pointer to argc_argv struct.
 * @param add_exe If non-zero, add the EXE filename as argv[0].
 */
void convertCmdLineToArgv(const char *lpCmdLine, argc_argv* arg, int add_exe)
{
	if (!arg)
		return;
	
	unsigned int	i;
	int		j;
	unsigned int	cmdLen = strlen(lpCmdLine);
	
	// parse a few of the command line arguments 
	// a space delimites an argument except when it is inside a quote 
	
	arg->c = (add_exe ? 1 : 0);
	int pos = 0; 
	for (i = 0; i < cmdLen; i++) 
	{ 
		while (lpCmdLine[i] == ' ' && i < cmdLen) 
		{
			i++; 
		}
		if (lpCmdLine[i] == '\"')
		{
			i++;
			while (lpCmdLine[i] != '\"' && i < cmdLen)
			{
				i++;
				pos++;
			}
			arg->c++;
			pos = 0;
		}
		else
		{
			while (lpCmdLine[i] != ' ' && i < cmdLen)
			{
				i++;
				pos++;
			}
			arg->c++;
			pos = 0;
		}
	}
	
	arg->v = (char**)malloc(sizeof(char*)*(arg->c + 1));
	
	if (add_exe)
	{
		arg->v[0] = (char*)malloc(1024);
		pGetModuleFileNameU(NULL, arg->v[0], 1024); 
	}
	
	for (j = (add_exe ? 1 : 0); j < arg->c; j++)
	{
		arg->v[j] = (char*)malloc(cmdLen + 10);
	}
	arg->v[arg->c] = NULL;
	
	arg->c = (add_exe ? 1 : 0);
	pos = 0;
	for (i = 0; i < cmdLen; i++)
	{
		while (lpCmdLine[i] == ' ' && i < cmdLen)
		{
			i++;
		}
		if (lpCmdLine[i] == '\"')
		{
			i++;
			while (lpCmdLine[i] != '\"' && i < cmdLen)
			{ 
				arg->v[arg->c][pos] = lpCmdLine[i];
				i++;
				pos++;
			}
			arg->v[arg->c][pos] = '\0';
			arg->c++;
			pos = 0;
		}
		else 
		{
			while (lpCmdLine[i] != ' ' && i < cmdLen)
			{
				arg->v[arg->c][pos] = lpCmdLine[i];
				i++;
				pos++;
			}
			arg->v[arg->c][pos] = '\0';
			arg->c++;
			pos = 0;
		}
	}
	arg->v[arg->c] = NULL;
	
	return;
}


/**
 * deleteArgcArgv(): Delete all arguments in an argc_argv struct.
 * @param arg Pointer to argc_argv struct.
 */
void deleteArgcArgv(argc_argv* arg)
{
	if (!arg)
		return;
	
	// Delete all parameters.
	int i;
	for (i = 0; i < arg->c; i++)
	{
		free(arg->v[i]);
	}
	
	free(arg->v);
	
	arg->c = 0;
}
