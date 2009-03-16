/***************************************************************************
 * Gens: GYM file handler.                                                 *
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

#ifndef GENS_GYM_HPP
#define GENS_GYM_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern int GYM_Dumping;

int gym_dump_start(void);
int gym_dump_stop(void);
int gym_dump_update(uint8_t v0, uint8_t v1, uint8_t v2);

int gym_play_start(void);
int gym_play_stop(void);
int gym_play(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_GYM_HPP */
