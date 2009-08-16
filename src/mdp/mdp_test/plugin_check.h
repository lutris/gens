/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * plugin_init.c: Plugin Check Function.                                   *
 *                                                                         *
 * Copyright (c) 2008-2009 by David Korth.                                 *
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

#ifndef __MDP_TEST_PLUGIN_CHECK_H
#define __MDP_TEST_PLUGIN_CHECK_H

#ifdef __cplusplus
extern "C" {
#endif

int plugin_check(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* __MDP_TEST_PLUGIN_CHECK_H */
