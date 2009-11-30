/***************************************************************************
 * MDP: IRC Reporter. (Format String Handler)                              *
 *                                                                         *
 * Copyright (c) 2009 by David Korth                                       *
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

#include "irc_format.hpp"
#include "irc.hpp"

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::stringstream;

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_constants.h"
#include "mdp/mdp_mem.h"

// Current format status.
typedef enum
{
	FMT_NORMAL	= 0,
	FMT_IN_FMT	= 1,	// % format code.
	FMT_IN_ESC	= 2,	// Backslash escape sequence.
} fmt_status;

#define MODIFIER(chr) (1 << ((chr) - 'a'))


/**
 * irc_format_S(): System name.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return System name, or empty string on error.
 */
static inline string irc_format_S(int system_id, uint32_t modifier)
{
	static const char *sysIDs_D[] =
	{
		"???", "MD", "MCD",
		"32X", "MCD/32X", "SMS",
		"GG", "SG-1000", "PICO", NULL
	};
	
	static const char *sysIDs_O[] =
	{
		"???", "GEN", "SCD",
		"32X", "SCD/32X", "SMS",
		"GG", "SG-1000", "PICO", NULL
	};
	
	static const char *sysNames_D[] =
	{
		"Unknown", "Mega Drive", "Mega CD",
		"32X", "Mega CD 32X", "Master System",
		"Game Gear", "SG-1000", "Pico", NULL
	};
	
	static const char *sysNames_O[] =
	{
		"Unknown", "Genesis", "Sega CD",
		"32X", "Sega CD 32X", "Master System",
		"Game Gear", "SG-1000", "Pico", NULL
	};
	
	// TODO: Get the emulator's region.
	// For now, default to:
	// - Abbreviation: Domestic (Japanese)
	// - System Name: Overseas (American)
	const char **strTable;
	
	if (modifier & MODIFIER('a'))
	{
		if (modifier & MODIFIER('o'))
			strTable = sysIDs_O;
		else
			strTable = sysIDs_D;
	}
	else
	{
		if (modifier & MODIFIER('d'))
			strTable = sysNames_D;
		else
			strTable = sysNames_O;
	}
	
	if (system_id >= MDP_SYSTEM_UNKNOWN &&
	    system_id < MDP_SYSTEM_MAX)
	{
		return string(strTable[system_id]);
	}
	
	// Unknown system ID.
	return string(strTable[0]);
}


/**
 * irc_format_Z(): ROM size.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return ROM size, or empty string on error.
 */
static inline string irc_format_Z(int system_id, uint32_t modifier)
{
	int rom_size;
	switch (system_id)
	{
		case MDP_SYSTEM_MD:
		case MDP_SYSTEM_MCD:	// TODO: This will get the SegaCD firmware size.
		case MDP_SYSTEM_32X:
		case MDP_SYSTEM_MCD32X:	// TODO: This will get the SegaCD firmware size.
		case MDP_SYSTEM_PICO:
			rom_size = irc_host_srv->mem_size_get(MDP_MEM_MD_ROM);
			break;
		
		default:
			return "unknown";
	}
	
	if (rom_size < 0)
		return "unknown";
	
	// Multiply by 8 for bits.
	rom_size *= 8;
	
	// Check how the size should be formatted.
	string unit_prefix = "mega";
	string unit_suffix = "bit";
	char unit_ap = 'M';
	char unit_as = 'b';
	int rem;
	if (modifier & MODIFIER('k'))
	{
		// Kilobits.
		rem = rom_size % 1024;
		rom_size /= 1024;
		if (rem)
			rom_size++;
		unit_prefix = "kilo";
		unit_ap = 'K';
	}
	else if (modifier & MODIFIER('b'))
	{
		// Bits. Do nothing.
		unit_prefix = "";
		unit_ap = 0;
	}
	else
	{
		// Default. Use megabits.
		rem = rom_size % 1048576;
		rom_size /= 1048576;
		if (rem)
			rom_size++;
	}
	
	double rom_size_D;
	if (modifier & MODIFIER('c'))
	{
		// Bytes.
		rom_size_D = ((double)rom_size / 8.0);
		unit_suffix = "byte";
		unit_as = 'B';
	}
	
	// Build the string.
	stringstream ss;
	if (unit_as == 'B')
		ss << rom_size_D;
	else
		ss << rom_size;
	
	if (modifier & MODIFIER('z'))
	{
		// Unit abbreviations.
		ss << ' ';
		if (unit_ap)
			ss << unit_ap;
		ss << unit_as;
	}
	else if (modifier & MODIFIER('y'))
	{
		// Unit names.
		ss << ' ' << unit_prefix << unit_suffix;
		if (rom_size > 1)
			ss << 's';
	}
	
	return ss.str();
}


/**
 * irc_format_entry(): Process an IRC format entry.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @param chr Character.
 * @return Formatted string, or empty string on error.
 */
static inline string irc_format_entry(int system_id, uint32_t modifier, char chr)
{
	printf("ENTRY: sysid == %d, modifier == 0x%08X, chr == %c\n", system_id, modifier, chr);
	switch (chr)
	{
		case '%':
			// Literal percent sign.
			return "%";
		
		case 'S':
			// System name.
			return irc_format_S(system_id, modifier);
		
		case 'T':
		case 'C':
		case 'N':
			// TODO
			break;
			
		case 'Z':
			// ROM size.
			return irc_format_Z(system_id, modifier);
		
		case 'D':
		case '[':
		case ']':
			// TODO
			break;
		
		default:
			// Unrecognized format character.
			break;
	}
	
	// Unrecognized format character.
	return "";
}

/**
 * irc_format(): Format a string using the IRC Reporter Format Specification.
 * @param system_id System ID.
 * @param str Format string.
 * @return Formatted string.
 */
string irc_format(int system_id, const char *str)
{
	stringstream ss;
	uint32_t modifier = 0;
	fmt_status status = FMT_NORMAL;
	char chr;
	
	while ((chr = *str++))
	{
		switch (status)
		{
			case FMT_NORMAL:
			case FMT_IN_ESC:	// TODO
				if (chr == '%')
				{
					modifier = 0;
					status = FMT_IN_FMT;
				}
				else if (chr == '\\')
					status = FMT_IN_ESC;
				else
					ss << chr;
				break;
			
			case FMT_IN_FMT:
				// In format string.
				if (chr >= 'a' && chr <= 'z')
				{
					// Modifier code.
					modifier |= MODIFIER(chr);
					break;
				}
				else
				{
					// Format code.
					ss << irc_format_entry(system_id, modifier, chr);
					status = FMT_NORMAL;
				}
				break;
			
			default:
				break;
		}
	}
	
	// Return the string.
	return ss.str();
}
