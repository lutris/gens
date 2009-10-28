/***************************************************************************
 * Gens: Message Logging.                                                  *
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

#ifndef GENS_LOG_MSG_H
#define GENS_LOG_MSG_H

/**
 * Log levels:
 * LOG_MSG_LEVEL_CRITICAL	== critical error. A message box is shown.
 * LOG_MSG_LEVEL_ERROR		== error.
 * LOG_MSG_LEVEL_WARNING	== warning.
 * LOG_MSG_LEVEL_INFO		== information.
 * LOG_MSG_LEVEL_DEBUG1		== debug level 1.
 * LOG_MSG_LEVEL_DEBUG2		== debug level 2.
 * LOG_MSG_LEVEL_DEBUG3		== debug level 3.
 * LOG_MSG_LEVEL_DEBUG4		== debug level 4.
 */

#define LOG_MSG_LEVEL_CRITICAL	0
#define LOG_MSG_LEVEL_ERROR	1
#define LOG_MSG_LEVEL_WARNING	2
#define LOG_MSG_LEVEL_INFO	3
#define LOG_MSG_LEVEL_DEBUG1	4
#define LOG_MSG_LEVEL_DEBUG2	5
#define LOG_MSG_LEVEL_DEBUG3	6
#define LOG_MSG_LEVEL_DEBUG4	7

#define LOG_MSG_LEVEL_NONE	LOG_MSG_LEVEL_CRITICAL

/**
 * Message channels.
 * The channels are #define'd to the maximum log level.
 * A value of LOG_MSG_LEVEL_NONE is the same as LOG_MSG_LEVEL_CRITICAL,
 * and will only show critical errors.
 */

#define LOG_MSG_CHANNEL_gens	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_stub	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_video	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_audio	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_input	LOG_MSG_LEVEL_INFO
#define LOG_MSG_CHANNEL_z	LOG_MSG_LEVEL_INFO

#define LOG_MSG_CHANNEL_mdp	LOG_MSG_LEVEL_INFO

// CPUs.
#define LOG_MSG_CHANNEL_68k	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_z80	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_sh2	LOG_MSG_LEVEL_NONE

// Sound chip emulation.
#define LOG_MSG_CHANNEL_ym2612	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_psg	LOG_MSG_LEVEL_NONE
#define LOG_MSG_CHANNEL_pcm	LOG_MSG_LEVEL_NONE

// SegaCD emulation.
#define LOG_MSG_CHANNEL_lc89510	LOG_MSG_LEVEL_NONE

// Actual MSG_LOG() code is below.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * log_msgbox(): Function used to show a message box for messages with level LOG_MSG_LEVEL_CRITICAL.
 * @param msg Message.
 * @param title Title.
 */
void log_msgbox(const char* msg, const char* title);

#ifdef __cplusplus
}
#endif

/**
 * STUB: Indicates that this function is a stub.
 */
#define STUB() LOG_MSG(stub, LOG_MSG_LEVEL_WARNING, "STUB function.")

/**
 * debug_window_log(): Log a message to the debug window.
 * @param channel Debug channel. (string)
 * @param level Debug level. (integer)
 * @param msg Message.
 * @param ... Parameters.
 */
#if defined(GENS_OS_WIN32) && !defined(GENS_WIN32_CONSOLE)
#define DBG_WIN_LOG(channel, level, msg, ...) \
do { \
	char log_msg[512]; \
	snprintf(log_msg, sizeof(log_msg), "%s:%d:%s(): " msg "\r\n", #channel, level, __func__, ##__VA_ARGS__); \
	log_msg[sizeof(log_msg)-1] = 0x00; \
} while (0)
#else
#define DBG_WIN_LOG(channel, level, msg, ...)
#endif

/**
 * LOG_MSG(): Output a debug message.
 * @param channel Debug channel. (string)
 * @param level Debug level. (integer)
 * @param msg Message.
 * @param ... Parameters.
 */
#define LOG_MSG(channel, level, msg, ...) \
do { \
	if (LOG_MSG_CHANNEL_ ##channel >= level) \
	{ \
		fprintf(stderr, "%s:%d:%s(): " msg "\n", #channel, level, __func__, ##__VA_ARGS__); \
		DBG_WIN_LOG(channel, level, msg, ##__VA_ARGS__); \
		if (level == 0) \
		{ \
			char box_msg[512], box_title[512]; \
			snprintf(box_msg, sizeof(box_msg), "%s(): " msg, __func__, ##__VA_ARGS__); \
			box_msg[sizeof(box_msg)-1] = 0x00; \
			snprintf(box_title, sizeof(box_msg), "Gens Critical Error: %s", #channel); \
			box_title[sizeof(box_title) - 1] = 0x00; \
			log_msgbox(box_msg, box_title); \
		} \
	} \
} while (0)

/**
 * LOG_MSG_ONCE(): Output a debug message one time only.
 * @param channel Debug channel. (string)
 * @param level Debug level. (integer)
 * @param msg Message.
 * @param ... Parameters.
 */
#define LOG_MSG_ONCE(channel, level, msg, ...)	\
do { \
	static unsigned char __warned = 0; \
	if (!__warned) \
	{ \
		LOG_MSG(channel, level, msg, ##__VA_ARGS__); \
		__warned = 1; \
	} \
} while (0)

#endif /* GENS_LOG_MSG_H */
