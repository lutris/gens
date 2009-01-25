/***************************************************************************
 * Gens: (GTK+) URI functions, copied from glib-2.18.4.                    *
 *                                                                         *
 * Copyright (C) 2006-2007 Red Hat, Inc.                                   *
 *                                                                         *
 * Gens/GS port Copyright (c) 2009 by David Korth                          *
 *                                                                         *
 * This library is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This library is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA           *
 * 02110-1301, USA.                                                        *
 *                                                                         *
 * Author: Alexander Larsson <alexl@redhat.com>                            *
 ***************************************************************************/

#ifndef GENS_GTK_URI_H
#define GENS_GTK_URI_H

#ifdef __cplusplus
extern "C" {
#endif

char* gens_g_uri_unescape_string(const char *escaped_string,
				 const char *illegal_characters);

#ifdef __cplusplus
}
#endif

#endif /* GENS_GTK_URI_H */
