/***************************************************************************
 * Gens: (Win32) Common UI functions.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "gens/gens_window.h"
#include "ui/win32/gens/gens_menu.hpp"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// Unused Parameter macro.
#include "macros/unused.h"

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>

// Gens Win32 resources.
#include "ui/win32/resource.h"

// Audio Handler.
#include "audio/audio.h"

// Windows
#include "controller_config/cc_window.h"
#include "bios_misc_files/bios_misc_files_window.hpp"
#include "directory_config/dir_window.hpp"
#include "general_options/general_options_window.hpp"
#include "color_adjust/color_adjust_window.h"
#include "select_cdrom/select_cdrom_window.h"
#include "country_code/country_code_window.h"
#include "about/about_window.hpp"
#include "plugin_manager/pmgr_window.hpp"

// Plugins.
#include "plugins/pluginmgr.hpp"

// C++ includes.
#include <list>
#include <string>
using std::list;
using std::string;

// Filename filters.
static const char* UI_Win32_FileFilter_AllFiles =
	"All Files\0*.*\0\0";

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

static const char* UI_Win32_FileFilter_ROMFile =
	"SegaCD / 32X / Genesis ROMs\0*.bin;*.smd;*.gen;*.32x;*.cue;*.iso;*.raw;" ZLIB_EXT LZMA_EXT "*.rar\0"
	"Genesis ROMs\0*.smd;*.bin;*.gen;*.zip;*.zsg;*.gz;*.7z;*.rar\0"
	"32X ROMs\0*.32x;" ZLIB_EXT LZMA_EXT "*.rar"
	"SegaCD Disc Images\0*.cue;*.iso;*.bin;*.raw\0"
	"All Files\0*.*\0\0";

static const char* UI_Win32_FileFilter_SavestateFile =
	"Savestate Files\0*.gs?\0"
	"All Files\0*.*\0\0";

static const char* UI_Win32_FileFilter_CDImage =
	"SegaCD Disc Images\0*.bin;*.iso;*.cue\0"
	"All Files\0*.*\0\0";

static const char* UI_Win32_FileFilter_ConfigFile =
	"Gens Config Files\0*.cfg\0\0"
	"All Files\0*.*\0\0";

static const char* UI_Win32_FileFilter_GYMFile =
	"GYM Files\0*.gym\0\0"
	"All Files\0*.*\0\0";

static string UI_Win32_OpenFile_int(const string& title,
				    const string& initFile,
				    const FileFilterType filterType,
				    HWND owner,
				    const bool openOrSave);

static int CALLBACK selectDir_SetSelProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

// Accelerator table for the main Gens window. [Non-menu commands.]
HACCEL hAccelTable_NonMenu;


/**
 * init(): Initialize the Win32 UI.
 * @param argc main()'s argc. (unused)
 * @param argv main()'s argv. (unused)
 */
void GensUI::init(int *argc, char **argv[])
{
	GENS_UNUSED_PARAMETER(argc);
	GENS_UNUSED_PARAMETER(argv);
	
	// Get the Windows version.
	winVersion.dwOSVersionInfoSize = sizeof(winVersion);
	GetVersionEx(&winVersion);
	
	// Initialize the Common Controls library.
	
	// CommCtrlEx is initially disabled.
	win32_CommCtrlEx = 0;
	
	if (winVersion.dwMajorVersion >= 5 ||
	    (winVersion.dwMajorVersion == 4 && winVersion.dwMinorVersion >= 10))
	{
		// Windows 98, Windows Me, Windows 2000, Windows XP, Windows Vista, or later.
		// Use InitCommonControlsEx().
		INITCOMMONCONTROLSEX iccx;
		iccx.dwSize = sizeof(iccx);
		iccx.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_BAR_CLASSES |
			     ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES;
		
		if (InitCommonControlsEx(&iccx))
		{
			// CommCtrlEx initialized.
			win32_CommCtrlEx = 1;
		}
		else
		{
			// Can't initialize extended common controls. Try regular common controls.
			// TODO: Check the return value.
			InitCommonControls();
		}
	}
	else
	{
		// Windows 95, Windows NT 4.0, or earlier.
		// Use InitCommonControls().
		// TODO: If NT4 or 95 is installed, and IE3 is installed, use InitCommonControlsEx().
		// TODO: Check the return value.
		InitCommonControls();
	}
	
	// Initialize COM.
	CoInitialize(NULL);
	
	// Create the fonts used by the rest of the program.
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	// Main font.
	fntMain = static_cast<HFONT>(CreateFontIndirect(&ncm.lfMessageFont));
	
	// Create the title font.
	// Title font is the main font with bold and italics.
	LOGFONT lf = ncm.lfMessageFont;
	lf.lfItalic = 1;
	lf.lfWeight = FW_BOLD;
	fntTitle = CreateFontIndirect(&lf);
	
	// Load the accelerator table for non-menu commands.
	hAccelTable_NonMenu = LoadAccelerators(ghInstance, MAKEINTRESOURCE(IDR_GENS_WINDOW_ACCEL_NONMENU));
	
	// Create and show the Gens window.
	gens_window_create();
	
	// Set the window title to Idle.
	setWindowTitle_Idle();
	
	// TODO: Gens Priority
#if 0
	switch(Gens_Priority)
	{
		case 0:
			SetThreadPriority(hInst, THREAD_PRIORITY_BELOW_NORMAL);
			break;

		case 2:
			SetThreadPriority(hInst, THREAD_PRIORITY_ABOVE_NORMAL);
			break;

		case 3:
			SetThreadPriority(hInst, THREAD_PRIORITY_HIGHEST);
			break;

		case 5:
			SetThreadPriority(hInst, THREAD_PRIORITY_TIME_CRITICAL);
			break;
	}
#endif
}


/**
 * update(): Update the UI.
 */
void GensUI::update(void)
{
	MSG msg;
	
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
			close_gens();
		
		// Check for an accelerator.
		if (gens_window && msg.hwnd == gens_window &&
		    ((hAccelTable_NonMenu && TranslateAccelerator(gens_window, hAccelTable_NonMenu, &msg)) ||
		     (hAccelTable_Menu    && TranslateAccelerator(gens_window, hAccelTable_Menu, &msg))))
		{
			// Accelerator. Don't process it as a regular message.
			continue;
		}
		
		// Check for dialog messages.
		if ((cc_window && IsDialogMessage(cc_window, &msg)) ||
		    (bios_misc_files_window && IsDialogMessage(bios_misc_files_window, &msg)) ||
		    (dir_window && IsDialogMessage(dir_window, &msg)) ||
		    (GeneralOptionsWindow::isOpen() && GeneralOptionsWindow::Instance()->isDialogMessage(&msg)) ||
		    (color_adjust_window && IsDialogMessage(color_adjust_window, &msg)) ||
		    (select_cdrom_window && IsDialogMessage(select_cdrom_window, &msg)) ||
		    (country_code_window && IsDialogMessage(country_code_window, &msg)) ||
		    (about_window && IsDialogMessage(about_window, &msg)) ||
		    (pmgr_window && IsDialogMessage(pmgr_window, &msg)))
		{
			// Dialog message. Don't process it as a regular message.
			continue;
		}
		
		// Check plugin windows for dialog messages.
		bool isDialogMessage = false;
		
		for (list<mdpWindow_t>::iterator lstIter = PluginMgr::lstWindows.begin();
		     lstIter != PluginMgr::lstWindows.end(); lstIter++)
		{
			if (IsDialogMessage((HWND)((*lstIter).window), &msg))
			{
				// Dialog message. Don't process it as a regular message.
				isDialogMessage = true;
				break;
			}
		}
		
		if (isDialogMessage)
		{
			// Dialog message. Don't process it normally.
			continue;
		}
		
		// Not a dialog message.
		
		// Check if this message requires clearing the audio buffer.
		if (msg.message == WM_MENUSELECT ||
		    msg.message == WM_ENTERSIZEMOVE ||
		    msg.message == WM_NCLBUTTONDOWN ||
		    msg.message == WM_NCRBUTTONDOWN)
		{
			// Clear the sound buffer.
			audio_clear_sound_buffer();
		}
		
		// Process the message.
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


/**
 * sleep(): Sleep, but keep the UI active.
 * @param ms Interval to sleep, in milliseconds.
 * @param noUpdate If true, don't check for GUI updates.
 */
void GensUI::sleep(const int ms, const bool noUpdate)
{
	if (noUpdate)
	{
		// Don't check for messages.
		Sleep(ms);
		return;
	}
	
	// Sleep 5 ms, then check for messages.
	int loops = ms / 5;
	for (int i = 0; i < loops; i++)
	{
		Sleep(5);
		update();
	}
}


/**
 * setWindowTitle(): Sets the window title.
 * @param title New window title.
 */
void GensUI::setWindowTitle(const string& title)
{
	SetWindowText(gens_window, title.c_str());
	update();
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
	// TODO: Extend this function.
	// This function is currently merely a copy of the Glade auto-generated open_msgbox() function.
	// (Well, with an added "title" parameter.)
	
	unsigned int msgStyle = 0;
	
	// Determine the Win32 message icon.
	switch (style & MSGBOX_ICON_MASK)
	{
		case MSGBOX_ICON_INFO:
			msgStyle |= MB_ICONINFORMATION;
			break;
		case MSGBOX_ICON_QUESTION:
			msgStyle |= MB_ICONQUESTION;
			break;
		case MSGBOX_ICON_WARNING:
			msgStyle |= MB_ICONWARNING;
			break;
		case MSGBOX_ICON_ERROR:
			msgStyle |= MB_ICONSTOP;
			break;
		case MSGBOX_ICON_NONE:
		default:
			msgStyle |= 0;
			break;
	}
	
	// Determine the Win32 message buttons.
	switch (style & MSGBOX_BUTTONS_MASK)
	{
		case MSGBOX_BUTTONS_OK_CANCEL:
			msgStyle |= MB_OKCANCEL;
			break;
		case MSGBOX_BUTTONS_YES_NO:
			msgStyle |= MB_YESNO;
			break;
		case MSGBOX_BUTTONS_OK:
		default:
			msgStyle |= MB_OK;
			break;
	}
	
	// If no owner was specified, use the Gens window.
	if (!owner)
		owner = static_cast<void*>(gens_window);
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Show the message box.
	int response = MessageBox(static_cast<HWND>(owner), msg.c_str(), title.c_str(), msgStyle);
	
	switch (response)
	{
		case IDYES:
			return MSGBOX_RESPONSE_YES;
		case IDNO:
			return MSGBOX_RESPONSE_NO;
		case IDCANCEL:
			return MSGBOX_RESPONSE_CANCEL;
		case IDOK:
		default:
			return MSGBOX_RESPONSE_OK;
	}
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
	return UI_Win32_OpenFile_int(title, initFile, filterType, static_cast<HWND>(owner), false);
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
	return UI_Win32_OpenFile_int(title, initFile, filterType, static_cast<HWND>(owner), true);
}


/**
 * UI_Win32_OpenFile_int(): Show the File Open/Save dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename filter to use.
 * @param owner Window that owns this dialog.
 * @param openOrSave false for Open; true for Save.
 * @return Filename if successful; otherwise, an empty string.
 */
static string UI_Win32_OpenFile_int(const string& title, const string& initFile,
				    const FileFilterType filterType, HWND owner,
				    const bool openOrSave)
{
	char filename[GENS_PATH_MAX];
	OPENFILENAME ofn;
	
	memset(filename, 0, sizeof(filename));
	memset(&ofn, 0, sizeof(OPENFILENAME));
	
	// If no owner was specified, use the Gens window.
	if (!owner)
		owner = gens_window;
	
	// Open Filename dialog settings
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.hInstance = ghInstance;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = GENS_PATH_MAX - 1;
	ofn.lpstrTitle = title.c_str();
	ofn.lpstrInitialDir = initFile.c_str();
	
	switch (filterType)
	{
		case ROMFile:
			ofn.lpstrFilter = UI_Win32_FileFilter_ROMFile;
			break;
		case SavestateFile:
			ofn.lpstrFilter = UI_Win32_FileFilter_SavestateFile;
			break;
		case CDImage:
			ofn.lpstrFilter = UI_Win32_FileFilter_CDImage;
			break;
		case ConfigFile:
			ofn.lpstrFilter = UI_Win32_FileFilter_ConfigFile;
			break;
		case GYMFile:
			ofn.lpstrFilter = UI_Win32_FileFilter_GYMFile;
			break;
		case AnyFile:
		default:
			ofn.lpstrFilter = UI_Win32_FileFilter_AllFiles;
			break;
	}
	
	ofn.nFilterIndex = 0;
	ofn.lpstrInitialDir = initFile.c_str();
	
	ofn.Flags = OFN_HIDEREADONLY;
	BOOL ret;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	if (!openOrSave)
	{
		// Open Dialog
		ofn.Flags |= OFN_FILEMUSTEXIST;
		ret = GetOpenFileName(&ofn);
	}
	else
	{
		// Save Dialog
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		ret = GetSaveFileName(&ofn);
	}
	
	// Reset the current directory to PathNames.Gens_EXE_Path.
	// (Why do GetOpenFileName() and GetSaveFileName change it?)
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
	
	if (!ret)
		return "";
	
	return ofn.lpstrFile;
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
	char displayName[GENS_PATH_MAX];
	char selDir[GENS_PATH_MAX];
	
	BROWSEINFO bi;
	memset(&bi, 0x00, sizeof(bi));
	
	// If no owner was specified, use the Gens window.
	if (!owner)
		owner = static_cast<void*>(gens_window);
	
	bi.hwndOwner = static_cast<HWND>(owner);
	bi.pidlRoot = NULL;
	bi.pszDisplayName = displayName;
	bi.lpszTitle = title.c_str();
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lParam = NULL;
	bi.iImage = 0;
	bi.lpfn = selectDir_SetSelProc;
	bi.lParam = (LPARAM)(initDir.c_str());
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (!pidl)
	{
		// No directory was selected.
		return "";
	}
	
	bool bRet = SHGetPathFromIDList(pidl, selDir);
	
	// Reset the current directory to PathNames.Gens_EXE_Path.
	// I'm not sure if SHGetPathFromIDList() changes it, but it might.
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
	
	if (!bRet)
		return "";
	
	return selDir;
}


/**
 * selectDir_SetSelProc(): Set the initial directory in GensUI::selectDir().
 * @param hWnd Window handle of the "Browse for Folder" dialog.
 * @param uMsg Message.
 * @param lParam lParam.
 * @param lpData Pointer to C string containing the initial directory.
 * @return 0.
 */
static int CALLBACK selectDir_SetSelProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	GENS_UNUSED_PARAMETER(lParam);
	
	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	
	return 0;
}
