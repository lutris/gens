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

// libgsft includes.
#include "libgsft/gsft_space_elim.h"

// Current format status.
typedef enum
{
	FMT_NORMAL	= 0,
	FMT_IN_FMT	= 1,	// % format code.
	FMT_IN_ESC	= 2,	// Backslash escape sequence.
} fmt_status_t;

typedef enum
{
	ESC_NONE	= 0,
	ESC_HEX_BYTE	= 1,
} esc_status_t;

#define MODIFIER(chr) (1 << ((chr) - 'a'))


/**
 * is_locked_on(): Check if a ROM is locked on at 2 MB.
 * @return True if a ROM is locked on; false otherwise.
 */
static inline bool is_locked_on(void)
{
	uint8_t lockon_hdr[4];
	static const uint8_t lockon_hdr_def[4] = {'S', 'E', 'G', 'A'};
	if (irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, 0x200100, &lockon_hdr[0], sizeof(lockon_hdr)) != MDP_ERR_OK)
		return false;
	
	if (memcmp(lockon_hdr_def, lockon_hdr, sizeof(lockon_hdr)) != 0)
		return false;
	
	// ROM is locked on.
	return true;
}


/**
 * irc_format_S(): System name.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return System name, or "???"/"Unknown" on error.
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
		else //if (modifier & MODIFIER('d'))	// TODO: Get the emulator's region settings.
			strTable = sysIDs_D;
	}
	else
	{
		if (modifier & MODIFIER('d'))
			strTable = sysNames_D;
		else //if (modifier & MODIFIER('o'))	// TODO: Get the emulator's region settings.
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
 * irc_format_T(): ROM title.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return ROM title, or "unknown" on error.
 */
static inline string irc_format_T(int system_id, uint32_t modifier)
{
	switch (system_id)
	{
		case MDP_SYSTEM_MD:
		case MDP_SYSTEM_32X:
		case MDP_SYSTEM_MCD:
		case MDP_SYSTEM_MCD32X:
		case MDP_SYSTEM_PICO:
		{
			// MD or 32X ROM. Read the header.
			// TODO: MCD and MCD32X will show the SegaCD firmware ROM name instead of the
			// actual game name, since MDP v1.0 doesn't support reading data from
			// the CD-ROM. This may be added in MDP v1.1.
			
			uint32_t cc_prio[2];
			if (modifier & MODIFIER('o'))
			{
				// Use Overseas as default.
				cc_prio[0] = 0x120;
				cc_prio[1] = 0x150;
			}
			else //if (modifier & MODIFIER('d'))	// TODO: Get the emulator's region settings.
			{
				// Use Domestic as default.
				cc_prio[0] = 0x150;
				cc_prio[1] = 0x120;
			}
				
			
			if (modifier & MODIFIER('l'))
			{
				// Check if this ROM has another ROM locked on at 2 MB.
				if (!is_locked_on())
					return "none";
				
				// ROM is locked on.
				cc_prio[0] |= 0x200000;
				cc_prio[1] |= 0x200000;
			}
			
			// Attempt to get the ROM name.
			char rom_name_raw[48];
			char rom_name[49];
			
			irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, cc_prio[0], (uint8_t*)rom_name_raw, 0x30);
			gsft_space_elim(rom_name_raw, 0x30, rom_name);
			if (rom_name[0] == 0x00)
			{
				// Name at first address is blank. Try second address.
				irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, cc_prio[1], (uint8_t*)rom_name_raw, 0x30);
				gsft_space_elim(rom_name_raw, 0x30, rom_name);
				if (rom_name[0] == 0x00)
				{
					// Domestic name is blank.
					// TODO: Show the ROM filename.
					return "unknown";
				}
			}
			
			// Return the ROM name.
			return string(rom_name);
		}
		
		default:
			break;
	}
	
	// Couldn't read the ROM title.
	return "unknown";
}


/**
 * irc_format_N(): ROM serial number.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return ROM serial number, or "unknown" on error.
 */
static inline string irc_format_N(int system_id, uint32_t modifier)
{
	uint32_t sn_addr;
	
	switch (system_id)
	{
		case MDP_SYSTEM_MD:
		case MDP_SYSTEM_32X:
		case MDP_SYSTEM_MCD:
		case MDP_SYSTEM_MCD32X:
		case MDP_SYSTEM_PICO:
		{
			// MD or 32X ROM. Read the header.
			// TODO: MCD and MCD32X will show the SegaCD firmware ROM name instead of the
			// actual game name, since MDP v1.0 doesn't support reading data from
			// the CD-ROM. This may be added in MDP v1.1.
			
			sn_addr = 0x180;
			if (modifier & MODIFIER('l'))
			{
				if (!is_locked_on())
					return "none";
				sn_addr |= 0x200000;
			}
			
			char serial_number[15];
			if (irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, sn_addr, (uint8_t*)serial_number, 14) != MDP_ERR_OK)
				return "unknown";
			
			serial_number[sizeof(serial_number)-1] = 0;
			return string(serial_number);
		}
		
		default:
			break;
	}
	
	// Couldn't read the serial number.
	return "unknown";
}


/**
 * irc_format_Z(): ROM size.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return ROM size, or "unknown" on error.
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
 * irc_format_D(): ROM build date.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @return ROM build date, or "unknown" on error.
 */
static inline string irc_format_D(int system_id, uint32_t modifier)
{
	uint32_t date_addr;
	
	switch (system_id)
	{
		case MDP_SYSTEM_MD:
		case MDP_SYSTEM_32X:
		case MDP_SYSTEM_MCD:
		case MDP_SYSTEM_MCD32X:
		case MDP_SYSTEM_PICO:
		{
			// MD or 32X ROM. Read the header.
			// TODO: MCD and MCD32X will show the SegaCD firmware ROM name instead of the
			// actual game name, since MDP v1.0 doesn't support reading data from
			// the CD-ROM. This may be added in MDP v1.1.
			
			date_addr = 0x118;
			if (modifier & MODIFIER('l'))
			{
				if (!is_locked_on())
					return "none";
				date_addr |= 0x200000;
			}
			
			char build_date[9];
			if (irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, date_addr, (uint8_t*)build_date, 8) != MDP_ERR_OK)
				return "unknown";
			
			build_date[sizeof(build_date)-1] = 0;
			return string(build_date);
		}
		
		default:
			break;
	}
	
	// Couldn't read the serial number.
	return "unknown";
}


/**
 * irc_format_code(): Process an IRC format code.
 * @param system_id System ID.
 * @param modifier Modifier.
 * @param chr Character.
 * @return Formatted string, or empty string on error.
 */
static inline string irc_format_code(int system_id, uint32_t modifier, char chr)
{
	switch (chr)
	{
		case '%':
			// Literal percent sign.
			return "%";
		
		case 'S':
			// System name.
			return irc_format_S(system_id, modifier);
		
		case 'T':
			// ROM title.
			return irc_format_T(system_id, modifier);
		
		case 'C':
			// TODO
			break;
			
		case 'N':
			// ROM serial number.
			return irc_format_N(system_id, modifier);
			
		case 'Z':
			// ROM size.
			return irc_format_Z(system_id, modifier);
		
		case 'D':
			// ROM build date.
			return irc_format_D(system_id, modifier);
		
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
	fmt_status_t status = FMT_NORMAL;
	char chr;
	
	// Escapes.
	esc_status_t esc_status = ESC_NONE;
	int esc_counter = 0;
	uint32_t esc_value = 0;
		
	// Set to true if in %[ %] and ROM isn't locked on.
	bool quiet = false;
	
	while ((chr = *str++))
	{
		switch (status)
		{
			case FMT_NORMAL:
				if (chr == '%')
				{
					modifier = 0;
					status = FMT_IN_FMT;
				}
				else if (chr == '\\')
					status = FMT_IN_ESC;
				else if (!quiet)
					ss << chr;
				break;
			
			case FMT_IN_ESC:
				switch (esc_status)
				{
					case ESC_NONE:
						//. Check the escape character.
						if (chr == '\\')
						{
							// Backslash.
							ss << '\\';
							status = FMT_NORMAL;
							break;
						}
						else if (chr == 'x')
						{
							// Hexadecimal byte.
							esc_status = ESC_HEX_BYTE;
							esc_counter = 0;
							esc_value = 0;
							break;
						}
						status = FMT_NORMAL;
						break;
					case ESC_HEX_BYTE:
						// Check the character.
						if (chr >= '0' && chr <= '9')
						{
							// Numeric character.
							esc_value <<= 4;
							esc_value |= (chr - '0');
						}
						else if (chr >= 'A' && chr <= 'F')
						{
							// Uppercase hexadecimal character.
							esc_value <<= 4;
							esc_value |= (chr - 'A' + 10);
						}
						else if (chr >= 'a' && chr <= 'f')
						{
							// Lowercase hexadecimal character.
							esc_value <<= 4;
							esc_value |= (chr - 'a' + 10);
						}
						else
						{
							// Unknown character. Cancel the escape.
							status = FMT_NORMAL;
							esc_status = ESC_NONE;
							break;
						}
						
						// Increment the counter.
						esc_counter++;
						if (esc_counter >= 2)
						{
							// Finished the escape sequence.
							ss << (char)esc_value;
							status = FMT_NORMAL;
							esc_status = ESC_NONE;
						}
						
						break;
				}
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
					if (chr == '[')
					{
						// Beginning of Lock-On Only string.
						if (!is_locked_on())
							quiet = true;
					}
					else if (chr == ']')
					{
						// End of Lock-On Only string.
						quiet = false;
					}
					else if (!quiet)
					{
						// Format Code.
						ss << irc_format_code(system_id, modifier, chr);
					}
					
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
