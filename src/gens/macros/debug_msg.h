/***************************************************************************
 * Gens: Debug Messages.                                                   *
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

#ifndef GENS_DEBUG_MSG_H
#define GENS_DEBUG_MSG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#define DEBUG_CHANNEL_gens	1
#define DEBUG_CHANENL_video	1
#define DEBUG_CHANNEL_audio	1
#define DEBUG_CHANNEL_input	1

#define DEBUG_CHANNEL_mdp	1

// Sound chip emulation.
#define DEBUG_CHANNEL_ym2612	0
#define DEBUG_CHANNEL_psg	0
#define DEBUG_CHANNEL_pcm	0

/**
 * Debug levels:
 * 0 == error.
 * 1 == informative.
 * 2 and higher == debugging. (TODO: Clarify this.)
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * debug_msgbox(): Function used to show a message box for Level 0 messages.
 * @param msg Message.
 * @param title Title.
 */
void debug_msgbox(const char* msg, const char* title);

#ifdef __cplusplus
}
#endif

/**
 * DEBUG_MSG(): Output a debug message.
 * @param channel Debug channel. (string)
 * @param level Debug level. (integer)
 * @param msg Message.
 * @param ... Parameters.
 */
#define DEBUG_MSG(channel, level, msg, ...)		\
{							\
	if (DEBUG_CHANNEL_ ##channel >= level)		\
	{						\
		fprintf(stderr, "%s:%d:%s(): " msg "\n", #channel, level, __func__, ##__VA_ARGS__);	\
		if (level == 0)										\
		{											\
			char box_msg[256], box_title[256];						\
			snprintf(box_msg, sizeof(box_msg), "%s(): " msg, __func__, ##__VA_ARGS__);	\
			box_msg[sizeof(box_msg) - 1] = 0x00;						\
			snprintf(box_title, sizeof(box_msg), "Gens Error: %s", #channel);		\
			box_title[sizeof(box_msg) - 1] = 0x00;						\
			debug_msgbox(box_msg, box_title);						\
		}											\
	}												\
}

#endif /* GENS_DEBUG_MSG_H */
