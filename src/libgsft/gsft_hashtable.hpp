/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_hashtable.hpp: C++ hashtable wrapper.                              *
 *                                                                         *
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

#ifndef __GSFT_HASHTABLE_HPP
#define __GSFT_HASHTABLE_HPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef __cplusplus

#if defined(GSFT_HASHTABLE_TR1_UNORDERED_MAP)

// TR1 unordered_map
#include <tr1/unordered_map>
#define GSFT_HASHTABLE std::tr1::unordered_map

#elif defined(GSFT_HASHTABLE_GNU_HASH_MAP)

// GNU hash_map
#include <ext/hash_map>
#define GSFT_HASHTABLE __gnu_cxx::hash_map

// Fix a bug with hash_map<string, int>
// See http://gcc.gnu.org/ml/libstdc++/2002-04/msg00107.html

// Also includes fix for hash_map<mdp_t*, int>.

#include <string>
#include <stdint.h>
struct _mdp_t;

namespace __gnu_cxx
{
	template<> struct hash<std::string>
	{
		size_t operator() (const std::string& x) const
		{
			return hash<const char*>()(x.c_str());
		}
	};
	
	template<> struct hash<struct _mdp_t*>
	{
		size_t operator() (struct _mdp_t* x) const
		{
			return hash<uintptr_t>()(reinterpret_cast<uintptr_t>(x));
		}
	};
}

#elif defined(GSFT_HASHTABLE_STD_MAP)

// Standard std::map
#include <map>
#define GSFT_HASHTABLE std::map

#else

// No hash table macro defined.
#error No hash table macro was defined.

#endif

// Useful predefined hashtables and pairs.
#include <string>
#include <utility>

// StrToInt: Used to map strings to ints.
typedef GSFT_HASHTABLE<std::string, int> mapStrToInt;
typedef std::pair<std::string, int> pairStrToInt;

#endif /* __cplusplus */

#endif /* __GSFT_HASHTABLE_HPP */
