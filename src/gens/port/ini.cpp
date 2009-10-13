/***************************************************************************
 * Gens: INI file handler class.                                           *
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

#include "ini.hpp"

// Message logging.
#include "macros/log_msg.h"

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

// C includes.
#include <stdlib.h>

// C++ includes.
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
using std::endl;
using std::ios;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::string;


INI::INI()
{
	// Standard constructor. Nothing's needed
}


INI::INI(const string& filename)
{
	// Load an INI file.
	load(filename);
}


INI::~INI()
{
}


/**
 * load(): Load the specified INI file into m_INI.
 * @param filename Filename of the INI file to load.
 */
void INI::load(const string& filename)
{
	// Load the specified INI file.
	string tmp;
	
	// Clear the map.
	m_INI.clear();
	
	// Open the file.
	ifstream cfgFile(filename.c_str());
	
	iniFile::iterator iniSectIter;
	iniSection curSection; string curSectionName;
	iniSection::iterator sectKeyIter;
	
	unsigned int commentPos;
	unsigned int equalsPos;
	string curKey, curValue;
	
	if (!cfgFile.is_open())
	{
		// Error opening the INI file.
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Error opening INI file '%s'.", filename.c_str());
		return;
	}
	
	
	while (!cfgFile.eof())
	{
		getline(cfgFile, tmp);
		
		// Parse the line.
		
		if (tmp.length() == 0)
		{
			// Empty string.
			continue;
		}
		
		// Find the first non-whitespace character.
		commentPos = tmp.find_first_not_of(" \t");
		if (commentPos == string::npos || tmp.at(commentPos) == ';')
		{
			// Comment or blank line found.
			continue;
		}
		
		if (tmp.substr(0, 1) == "[" &&
			tmp.substr(tmp.length() - 1, 1) == "]")
		{
			// Section header.
			
			// If there's a section right now, add it to the map.
			if (curSectionName.length() > 0)
			{
				m_INI.insert(pairIniFile(curSectionName, curSection));
				curSection.clear();
				curSectionName.clear();
			}
			
			// Set the section.
			curSectionName = tmp.substr(1, tmp.length() - 2);
			
			// Check if this section already exists.
			iniSectIter = m_INI.find(curSectionName);
			if (iniSectIter != m_INI.end())
			{
				// Section already exists.
				// Copy it and delete the one in the map.
				curSectionName = (*sectKeyIter).first;
				curSection = (*iniSectIter).second;
				m_INI.erase(iniSectIter);
			}
		}
		else if (curSectionName.length() > 0)
		{
			// Check if there's an "=", indicating a key value.
			equalsPos = tmp.find("=");
			if (equalsPos == string::npos)
			{
				// No key value.
				continue;
			}
			
			// Key value.
			curKey = tmp.substr(0, equalsPos);
			curValue = tmp.substr(equalsPos + 1);
			
			// Check if this key already exists in the current section.
			sectKeyIter = curSection.find(curKey);
			if (sectKeyIter != curSection.end())
			{
				// Already exists. Delete the existing one.
				curSection.erase(sectKeyIter);
			}
			
			// Add the key.
			curSection.insert(pairIniSection(curKey, curValue));
		}
	}
	
	// If a section is still being added, add it to the config.
	if (curSectionName.length() > 0)
	{
		m_INI.insert(pairIniFile(curSectionName, curSection));
		curSection.clear();
		curSectionName.clear();
	}
	
	cfgFile.close();
}


/**
 * INI_Clear(): Clear the loaded INI file.
 */
void INI::clear(void)
{
	m_INI.clear();
}


/**
 * getInt(): Get an integer value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @return Integer value.
 */
int INI::getInt(const string& section, const string& key, const int def)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found.
		return def;
	}
	
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter == (*iniSectIter).second.end())
	{
		// Key not found.
		return def;
	}
	
	// Found the key.
	const string& val = (*sectKeyIter).second;
	if (val.empty())
	{
		// Value is empty. Use the default value.
		return def;
	}
	
	// Value is not empty. Return it.
	return strtol(val.c_str(), NULL, 0);
}


/**
 * getBool(): Get a boolean value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @return Boolean value.
 */
bool INI::getBool(const string& section, const string& key, const bool def)
{
	return getInt(section, key, def);
}


/**
 * getString(): Get a string value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @return String value.
 */
string INI::getString(const string& section, const string& key, const string& def)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found.
		return def;
	}
	
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter == (*iniSectIter).second.end())
	{
		// Key not found.
		return def;
	}
	
	// Found the key.
	return (*sectKeyIter).second;
}


/**
 * getString(): Get a string value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @param buf Buffer to store the string value in.
 * @param size Size of the buffer.
 */
void INI::getString(const string& section, const string& key, const string& def,
		    char *buf, unsigned int size)
{
	string tmp;
	tmp = getString(section, key, def);
	strlcpy(buf, tmp.c_str(), size);
}


/**
 * writeInt(): Writes an integer value to the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param value Integer value to write.
 * @param hex If true, writes the value as a hexadecimal number.
 * @param hexFieldWidth Specifies the minimum field width, in characters,for hexadecimal numbers.
 */
void INI::writeInt(const string& section, const string& key, const int value,
		   const bool hex, const uint8_t hexFieldWidth)
{
	stringstream out;
	
	if (hex)
	{
		// Hexadecimal.
		out << "0x" << std::uppercase << std::hex;
		
		// Field width, if requested.
		if (hexFieldWidth > 0)
			out << std::setw(hexFieldWidth) << std::setfill('0');
		
		out << value;
	}
	else
	{
		// Decimal.
		out << value;
	}
	
	writeString(section, key, out.str().c_str());
}


/**
 * writeBool(): Writes a boolean value to the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param value Integer value to write.
 */
void INI::writeBool(const string& section, const string& key, const bool value)
{
	writeString(section, key, (value ? "1" : "0"));
}


/**
 * INI_WriteString(): Writes a string value to the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param value String value to write.
 */
void INI::writeString(const string& section, const string& key, const string& value)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found. Create it.
		iniSection newSection;
		m_INI.insert(pairIniFile(section, newSection));
		iniSectIter = m_INI.find(section);
		if (iniSectIter == m_INI.end())
		{
			// Error creating the new section.
			LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
				"Error creating new INI section '%s'.", section.c_str());
			return;
		}
	}
	
	// Check if this key already exists.
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter != (*iniSectIter).second.end())
	{
		// Key found. Delete it.
		(*iniSectIter).second.erase(sectKeyIter);
	}
	
	// Create the key.
	(*iniSectIter).second.insert(pairIniSection(key, value));
	
	// Found the key.
	return;
}


/**
 * INI::deleteEntry(): Delete an INI entry.
 * @param section Section to delete from.
 * @param key Key to delete.
 */
void INI::deleteEntry(const string& section, const string& key)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	// Search for the section.
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found.
		return;
	}
	
	// // Search for the key.
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter == (*iniSectIter).second.end())
	{
		// Key not found.
		return;
	}
	
	// Key found. Delete it.
	(*iniSectIter).second.erase(sectKeyIter);
	return;
}


/**
 * save(): Save m_INI to the specified INI file.
 * @param filename Filename of the INI file to save to.
 */
void INI::save(const string& filename)
{
	// Load the specified INI file.
	string tmp;
	
	// Open the file.
	ofstream cfgFile(filename.c_str());
	
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	if (!cfgFile.is_open())
	{
		// Error opening the INI file.
		LOG_MSG(gens, LOG_MSG_LEVEL_WARNING,
			"Error opening INI file '%s'.", filename.c_str());
		return;
	}
	
	// Write to the INI file.
	for (iniSectIter = m_INI.begin(); iniSectIter != m_INI.end(); iniSectIter++)
	{
		// Write the section header.
		cfgFile << "[" << (*iniSectIter).first << "]" << endl;
		
		// Write all keys.
		for (sectKeyIter = (*iniSectIter).second.begin();
		     sectKeyIter != (*iniSectIter).second.end();
		     sectKeyIter++)
		{
			cfgFile << (*sectKeyIter).first << "=" << (*sectKeyIter).second << endl;
		}
		
		// Add a newline between sections.
		cfgFile << endl;
	}
	
	// Done.
	cfgFile.close();
}


/**
 * sectionExists(): Check if a section exists.
 * @param section Section to check.
 * @return True if the section exists; false if it doesn't exist.
 */
bool INI::sectionExists(const string& section)
{
	iniFile::iterator iniSectIter;
	
	// Search for the section.
	iniSectIter = m_INI.find(section);
	return (iniSectIter != m_INI.end());
}


iniSection INI::getSection(const string& section)
{
	iniFile::iterator iniSectIter;
	
	// Search for the section.
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// INI section doesn't exist.
		iniSection blank;
		return blank;
	}
	
	// INI section found.
	return (*iniSectIter).second;
}
