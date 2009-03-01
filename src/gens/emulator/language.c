/***************************************************************************
 * Gens: Language handler. (DEPRECATED)                                    *
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

#include "language.h"
#include "g_main.hpp"

#ifndef GENS_OS_WIN32
// Old INI handling functions are still needed for now.
#include "port/ini_old.h"
#endif /* !GENS_OS_WIN32 */

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Video handler.
#include "video/vdraw.h"

int Language = 0;


// TODO: Rewrite the language system so it doesn't depend on the old INI functions.
int Build_Language_String(void)
{
	unsigned long nb_lue = 1;
	int sec_alloue = 1, poscar = 0;
	enum etat_sec
	{
		DEB_LIGNE,
		SECTION,
		NORMAL,
	} etat = DEB_LIGNE;
	
	FILE *LFile;
	
	char c;
	
	if (language_name)
	{
		free(language_name);
		language_name = NULL;
	}
	
	language_name = (char **)malloc(sec_alloue * sizeof(char*));
	language_name[0] = NULL;
	
	LFile = fopen(PathNames.Language_Path, "r");
	if (!LFile)
	{
		LFile = fopen(PathNames.Language_Path, "w");
	}
	
	while (nb_lue)
	{
		nb_lue = fread (&c, 1, 1, LFile);
		switch (etat)
		{
			case DEB_LIGNE:
				switch (c)
				{
					case '[':
						etat = SECTION;
						sec_alloue++;
						language_name =	(char**)realloc(language_name, sec_alloue * sizeof(char*));
						language_name[sec_alloue - 2] = (char*)malloc(32 * sizeof(char));
						language_name[sec_alloue - 1] = NULL;
						poscar = 0;
						break;
					
					case '\n':
						break;
					
					default:
						etat = NORMAL;
						break;
				}
				break;
			
			case NORMAL:
				switch (c)
				{
					case '\n':
						etat = DEB_LIGNE;
						break;
					
					default:
						break;
				}
				break;
			
			case SECTION:
				switch (c)
				{
					case ']':
						language_name[sec_alloue - 2][poscar] = 0;
						etat = DEB_LIGNE;
						break;
					
					default:
						if (poscar < 32)
							language_name[sec_alloue - 2][poscar++] = c;
						break;
				}
				break;
		}
	}
	
	fclose(LFile);
	
	if (sec_alloue == 1)
	{
		language_name = (char**)realloc(language_name, 2 * sizeof(char *));
		language_name[0] = (char*)malloc(32 * sizeof(char));
		strcpy(language_name[0], "English");
		language_name[1] = NULL;
		WritePrivateProfileString("English", "Menu Language", "&English menu", PathNames.Language_Path);
	}
	
	return 0;
}


/**
 * MESSAGE_L(): Print a localized message.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_L(const char* str, const char* def, int time)
{
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	vdraw_write_text(buf, time);
}


/**
 * MESSAGE_NUM_L(): Print a localized message with one number in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param num Number.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_NUM_L(const char* str, const char* def, int num, int time)
{
	char msg_tmp[1024];
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	sprintf(msg_tmp, buf, num);
	vdraw_write_text(msg_tmp, time);
}


/**
 * MESSAGE_STR_L(): Print a localized message with a substring in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param str2 Substring.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_STR_L(const char* str, const char* def, const char* str2, int time)
{
	char msg_tmp[1024];
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	sprintf(msg_tmp, buf, str2);
	vdraw_write_text(msg_tmp, time);
}


/**
 * MESSAGE_NUM_2L(): Print a localized message with two numbers in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param num1 First number.
 * @param num2 Second number.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_NUM_2L(const char* str, const char* def, int num1, int num2, int time)
{
	char msg_tmp[1024];
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	sprintf(msg_tmp, buf, num1, num2);
	vdraw_write_text(msg_tmp, time);
}
