/******************************************************************************
 * MDP: Game Genie. (strtok_r() replacement)                                  *
 *                                                                            *
 * MDP implementation Copyright (c) 2008-2009 by David Korth                  *
 *                                                                            *
 * Original implementation derived from the GNU C Library.                    *
 * Copyright (C) 1991,1996-1999,2001,2004,2007 Free Software Foundation, Inc. *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify it    *
 * under the terms of the GNU General Public License as published by the      *
 * Free Software Foundation; either version 2 of the License, or (at your     *
 * option) any later version.                                                 *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.              *
 ******************************************************************************/

#ifndef _MDP_MISC_GAME_GENIE_STRTOK_R_H
#define _MDP_MISC_GAME_GENIE_STRTOK_R_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRTOK_R

/* System has strtok_r(). */
#define gg_strtok_r(str, delim, saveptr) strtok_r(str, delim, saveptr)

#else /* !HAVE_STRTOK_R */

/* System does not have strtok_r(). */

#include "mdp/mdp_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_LOCAL char* MDP_FNCALL gg_strtok_r(char *str, const char *delim, char **save_ptr);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_STRTOK_R */

#endif /* _MDP_MISC_GAME_GENIE_STRTOK_R_H */
