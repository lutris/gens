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

#ifndef GENS_INI_HPP
#define GENS_INI_HPP

#ifdef __cplusplus

#include <string.h>
#include <stdint.h>

#include <string>
#include <map>
#include <utility>

// Case-insensitive search from http://www.experts-exchange.com/Programming/Languages/CPP/Q_23025401.html
class __lesscasecmp
{
	public:
		bool operator() (const std::string& a, const std::string& b) const
		{
			return (strcasecmp(a.c_str(), b.c_str()) < 0);
		}
};

typedef std::map<std::string, std::string, __lesscasecmp> iniSection;
typedef std::pair<std::string, std::string> pairIniSection;

typedef std::map<std::string, iniSection, __lesscasecmp> iniFile;
typedef std::pair<std::string, iniSection> pairIniFile;

class INI
{
	public:
		INI();
		INI(const std::string& filename);
		~INI();
		
		// Load an INI file.
		void load(const std::string& filename);
		
		// Save the INI file.
		void save(const std::string& filename);
		
		// Clear the loaded INI file.
		void clear(void);
		
		// Get settings from the loaded INI file.
		int getInt(const std::string& section, const std::string& key, const int def);
		bool getBool(const std::string& section, const std::string& key, const bool def);
		std::string getString(const std::string& section, const std::string& key, const std::string& def);
		void getString(const std::string& section, const std::string& key, const std::string& def,
			       char *buf, unsigned int size);
		
		// Write settings to the loaded INI file.
		void writeInt(const std::string& section, const std::string& key, const int value,
			      const bool hex = false, const uint8_t hexFieldWidth = 0);
		void writeBool(const std::string& section, const std::string& key, const bool value);
		void writeString(const std::string& section, const std::string& key, const std::string& value);
		
		// Delete entry.
		void deleteEntry(const std::string& section, const std::string& key);
		
		// Section functions.
		bool sectionExists(const std::string& section);
		iniSection getSection(const std::string& section);
	
	protected:
		iniFile m_INI;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* GENS_INI_HPP */
