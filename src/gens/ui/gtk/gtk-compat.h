/***************************************************************************
 * Gens: (GTK+) Compatibility macros.                                      *
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

#ifndef GENS_GTK_COMPAT_H
#define GENS_GTK_COMPAT_H

/* Functions added in GTK+ 2.14. */
#if (GTK_MAJOR_VERSION < 2) || ((GTK_MAJOR_VERSION == 2) && (GTK_MINOR_VERSION < 14))
#define gtk_widget_get_window(widget)			((widget)->window)
#define gtk_selection_data_get_length(selection_data)	((selection_data)->length)
#define gtk_selection_data_get_data(selection_data)	((selection_data)->data)
#endif

#endif /* GENS_GTK_COMPAT_H */
