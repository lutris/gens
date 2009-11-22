/***************************************************************************
 * Gens: (Haiku) Common UI functions.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <string.h>

#include "ui/haiku/gens/gens_app.hpp"
#include "gens/gens_window.h"
#include "ui/haiku/gens/gens_menu.hpp"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

// Haiku
#include "controller_config/cc_window.h"
#include "bios_misc_files/bmf_window.hpp"
#include "directory_config/dir_window.hpp"
#include "general_options/genopt_window.hpp"
#include "color_adjust/ca_window.h"
#include "country_code/ccode_window.h"
#include "about/about_window.hpp"
#include "plugin_manager/pmgr_window.hpp"
#include <Application.h>

// Plugins.
#include "plugins/pluginmgr.hpp"

// C++ includes.
#include <list>
#include <string>
using std::list;
using std::string;

// File extensions.
#ifdef GENS_ZLIB
	#define ZLIB_EXT "*.zip;*.zsg;*.gz;"
#else
	#define ZLIB_EXT
#endif
#ifdef GENS_LZMA
	#define LZMA_EXT "*.7z;"
#else
	#define LZMA_EXT
#endif


/**
 * init(): Initialize the Haiku UI.
 * @param argc main()'s argc. (unused)
 * @param argv main()'s argv. (unused)
 */
void GensUI::init(int *argc, char **argv[])
{
	GSFT_UNUSED_PARAMETER(argc);
	GSFT_UNUSED_PARAMETER(argv);
}


/**
 * update(): Update the UI.
 */
void GensUI::update(void)
{
}


/**
 * sleep(): Sleep, but keep the UI active.
 * @param ms Interval to sleep, in milliseconds.
 * @param noUpdate If true, don't check for GUI updates.
 */
static bool sleeping = false;
void GensUI::sleep(const int ms, const bool noUpdate)
{
}


/**
 * wakeup(): Wakeup from sleep.
 */
void GensUI::wakeup(void)
{
}


/**
 * setWindowTitle(): Sets the window title.
 * @param title New window title.
 */
void GensUI::setWindowTitle(const string& title)
{
}


/**
 * setWindowVisibility(): Sets window visibility.
 * @param visibility true to show; false to hide.
 */
void GensUI::setWindowVisibility(const bool visibility)
{
}


/**
 * msgBox(): Show a message box.
 * @param msg Message.
 * @param title Title.
 * @param style Style, such as icons and buttons.
 * @param owner Window that owns this dialog.
 * @return Button pressed.
 */
GensUI::MsgBox_Response GensUI::msgBox(const string& msg, const string& title,
				       const unsigned int style, void* owner)
{
	return static_cast<GensUI::MsgBox_Response>(NULL);
}


/**
 * openFile(): Show the File Open dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename filter to use.
 * @param owner Window that owns this dialog.
 * @return Filename if successful; otherwise, an empty string.
 */
string GensUI::openFile(const string& title, const string& initFile,
			const FileFilterType filterType, void* owner)
{
	return string("Stub");
}


/**
 * saveFile(): Show the File Save dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType of filename filter to use.
 * @param owner Window that owns this dialog.
 * @return Filename if successful; otherwise, an empty string.
 */
string GensUI::saveFile(const string& title, const string& initFile,
			const FileFilterType filterType, void* owner)
{
	return string("Stub");
}


/**
 * selectDir(): Show the Select Directory dialog.
 * @param title Window title.
 * @param initDir Initial directory.
 * @param owner Window that owns this dialog.
 * @return Directory name if successful; otherwise, an empty string.
 */
string GensUI::selectDir(const string& title, const string& initDir, void* owner)
{
	return string("Stub");
}


/**
 * setMousePointer(): Set the mouse pointer. 
 * @param busy True if the pointer should be busy; False if the pointer should be normal.
 */
void GensUI::setMousePointer(bool busy)
{
}


/**
 * launchBrowser(): Launch a web browser with the given URL.
 * @param url URL to load.
 */
void GensUI::LaunchBrowser(const string& url)
{
	// TODO
}


/**
 * fsMinimize(): Full Screen Minimize.
 * @param fst Type of FS Minimization.
 */
void GensUI::fsMinimize(fsMinimize_Type fst)
{
	// TODO
}


/**
 * fsRestore() Full Screen Restore.
 * @param fst Type of FS Minimization.
 */
void GensUI::fsRestore(fsMinimize_Type fst)
{
	// TODO
}
