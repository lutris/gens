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
#define COLOR_PASS
#define COLOR_OFF
#else
#define COLOR_FAIL "\x1B[01;31m"
#define COLOR_PASS "\x1B[01;32m"
#define COLOR_OFF "\x1B[00m"
#endif

// Test fail/pass macros.
#include <stdio.h>
#define TEST_FAIL(func, err) \
	fprintf(stderr, COLOR_FAIL "FAILED: %s() - %s" COLOR_OFF "\n", func, err);
#define TEST_FAIL_MDP(func, err) \
	fprintf(stderr, COLOR_FAIL "FAILED: %s() - MDP error code 0x%08X" COLOR_OFF "\n", func, -err);
#define TEST_PASS(func) \
	fprintf(stderr, COLOR_PASS "PASSED: %s()" COLOR_OFF "\n", func);
#define TEST_PASS_SUB(func, subfunc) \
	fprintf(stderr, COLOR_PASS "PASSED: %s() - %s" COLOR_OFF "\n", func, err);

#endif /* __MDP_TEST_MAIN_H */
