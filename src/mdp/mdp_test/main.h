/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * main.c: Entry Point.                                                    *
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

#ifndef __MDP_TEST_MAIN_H
#define __MDP_TEST_MAIN_H

// MDP includes.
#include "mdp/mdp.h"
#include "mdp/mdp_version.h"

// MDP interface version this test suite targets.
#define TEST_MDP_INTERFACE_VERSION MDP_VERSION(1, 0, 0)

// Test suite version.
#define TEST_VERSION MDP_VERSION(0, 1, 0)
//#define TEST_VERSION_DEVEL

#ifdef _WIN32
#define DIRSEP_CHR '\\'
#define DIRSEP_STR "\\"
#else
#define DIRSEP_CHR '/'
#define DIRSEP_STR "/"
#endif

// ANSI color escape codes.
#ifdef _WIN32
#define COLOR_FAIL
#define COLOR_WARN
#define COLOR_PASS
#define COLOR_INFO
#define COLOR_OFF
#else
#define COLOR_FAIL "\x1B[01;31m"
#define COLOR_WARN "\x1B[01;33m"
#define COLOR_PASS "\x1B[01;32m"
#define COLOR_INFO "\x1B[01;35m"
#define COLOR_OFF "\x1B[00m"
#endif

// Test fail/pass macros.
#include <stdio.h>

#define TEST_START(test) \
	do { \
		printf("%s: ", test); \
		fflush(NULL); \
	} while (0)
#define TEST_START_ARGS(test, ...) \
	do { \
		printf(test ":", ##__VA_ARGS__); \
		fflush(NULL); \
	} while (0)

#define TEST_FAIL(err) \
	do { \
		printf("\t" COLOR_FAIL "FAILED: %s" COLOR_OFF "\n", err); \
		fflush(NULL); \
	} while (0)
#define TEST_FAIL_ARGS(err, ...) \
	do { \
		printf("\t" COLOR_FAIL "FAILED: " err COLOR_OFF "\n", ##__VA_ARGS__); \
		fflush(NULL); \
	} while (0)
#define TEST_FAIL_MDP(mdp_err) \
	do { \
		printf("\t" COLOR_FAIL "FAILED: MDP error code 0x%08X" COLOR_OFF "\n", -mdp_err); \
		fflush(NULL); \
	} while (0)

#define TEST_WARN(warn) \
	do { \
		printf("\t" COLOR_WARN "WARNING: %s\n", warn); \
		fflush(NULL); \
	} while (0)
#define TEST_WARN_ARGS(warn, ...) \
	do { \
		printf("\t" COLOR_WARN "WARNING: " warn "\n", ##__VA_ARGS__); \
		fflush(NULL); \
	} while (0)

#define TEST_INFO(info) \
	do { \
		printf("\t" COLOR_INFO "%s" COLOR_OFF "\n", info); \
		fflush(NULL); \
	} while (0)
#define TEST_INFO_ARGS(info, ...) \
	do { \
		printf("\t" COLOR_INFO info COLOR_OFF "\n", ##__VA_ARGS__); \
		fflush(NULL); \
	} while (0)

#define TEST_PASS() \
	do { \
		printf("\t" COLOR_PASS "PASSED" COLOR_OFF "\n"); \
		fflush(NULL); \
	} while (0)
#define TEST_PASS_MSG(msg) \
	do { \
		printf("\t" COLOR_PASS "PASSED: %s" COLOR_OFF "\n", msg); \
		fflush(NULL); \
	} while (0)

#endif /* __MDP_TEST_MAIN_H */
