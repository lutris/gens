/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_unused.h: Unused Parameter macro.                                  *
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

#ifndef __GSFT_UNUSED_H
#define __GSFT_UNUSED_H

/**
 * A macro to suppress some compilers' "Parameter is not used" warnings.
 * Copied from Subversion.
 */
#define GSFT_UNUSED_PARAMETER(x) ((void)(x))

#endif /* __GSFT_UNUSED_H */
