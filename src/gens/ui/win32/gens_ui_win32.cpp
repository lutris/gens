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

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// Win32 common controls
#include <commctrl.h>

// Shell objects
#include <shlobj.h>


// Filename filters.
static const char* UI_Win32_FileFilter_AllFiles =
	"All Files\0*.*\0\0";

static const char* UI_Win32_FileFilter_ROMFile =
	"SegaCD / 32X / Genesis ROMs\0*.bin;*.smd;*.gen;*.32x;*.cue;*.iso;*.raw;*.zip;*.zsg;*.gz;*.7z\0"
	"Genesis ROMs\0*.smd;*.bin;*.gen;*.zip;*.zsg;*.gz;*.7z\0"
	"32X ROMs\0*.32x;*.zip;*.gz;*.7z\0"
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


/**
 * init(): Initialize the Win32 UI.
 * @param argc main()'s argc. (unused)
 * @param argv main()'s argv. (unused)
 */
void GensUI::init(int argc, char *argv[])
{
	GENS_UNUSED_PARAMETER(argc);
	GENS_UNUSED_PARAMETER(argv);
	
	// Initialize the Common Controls library.
	// TODO: InitCommonControls() on 9x; InitCommonControlsEx() on 98 and later
	//InitCommonControls();
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_BAR_CLASSES |
		     ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES;
	// TODO: Check the return value.
	InitCommonControlsEx(&iccx);
	
	// Initialize COM.
	CoInitialize(NULL);
	
	// Create the fonts used by the rest of the program.
	// TODO: Get the theme font instead of the default (MS Sans Serif).
	HFONT fntDefaultGUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	fntMain = fntDefaultGUIFont;
	
	// Create the title font.
	// Title font is the main font with bold and italics.
	LOGFONT lf;
	GetObject(fntDefaultGUIFont, sizeof(LOGFONT), &lf);
	lf.lfItalic = 1;
	lf.lfWeight = FW_BOLD;
	fntTitle = CreateFontIndirect(&lf);
	
	// Create and show the Gens window.
	create_gens_window();
	
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
#if 0
	while (gtk_events_pending())
		gtk_main_iteration_do(FALSE);
#endif
}


/**
 * sleep(): Sleep, but keep the UI active.
 * @param ms Interval to sleep, in milliseconds.
 */
void GensUI::sleep(const int ms)
{
	Sleep(ms);
}


/**
 * setWindowTitle(): Sets the window title.
 * @param title New window title.
 */
void GensUI::setWindowTitle(const string& title)
{
	SetWindowText(Gens_hWnd, title.c_str());
	update();
}


/**
 * setWindowVisibility(): Sets window visibility.
 * @param visibility true to show; false to hide.
 */
void GensUI::setWindowVisibility(const bool visibility)
{
	STUB;
#if 0
	if (visibility)
		gtk_widget_show(gens_window);
	else
		gtk_widget_hide(gens_window);
#endif
}


/**
 * UI_Hide_Embedded_Window(): Hides the embedded SDL window.
 */
void UI_Hide_Embedded_Window(void)
{
	STUB;
#if 0
	gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
#endif
}


/**
 * UI_Show_Embedded_Window(): Shows the embedded SDL window.
 * @param w Width of the embedded SDL window.
 * @param h Height of the embedded SDL window.
 */
void UI_Show_Embedded_Window(const int w, const int h)
{
	STUB;
#if 0
	GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
	gtk_widget_set_size_request(sdlsock, w, h);
	gtk_widget_realize(sdlsock);
	gtk_widget_show(sdlsock);
#endif
}


/**
 * UI_Get_Embedded_WindowID(): Gets the window ID of the embedded SDL window.
 * @return Window ID of the embedded SDL window.
 */
int UI_Get_Embedded_WindowID(void)
{
	STUB;
#if 0
	GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
	return GDK_WINDOW_XWINDOW(sdlsock->window);
#endif
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
		owner = static_cast<void*>(Gens_hWnd);
	
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
		owner = Gens_hWnd;
	
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
		default:
			ofn.lpstrFilter = UI_Win32_FileFilter_AllFiles;
			break;
	}
	
	ofn.nFilterIndex = 0;
	ofn.lpstrInitialDir = initFile.c_str();
	
	ofn.Flags = OFN_HIDEREADONLY;
	BOOL ret;
	
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
	// (Why do GetOpenFIleName() and GetSaveFileName change it?)
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
		owner = static_cast<void*>(Gens_hWnd);
	
	bi.hwndOwner = static_cast<HWND>(owner);
	bi.pidlRoot = NULL;
	bi.pszDisplayName = displayName;
	bi.lpszTitle = title.c_str();
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lParam = NULL;
	bi.iImage = 0;
	bi.lpfn = selectDir_SetSelProc;
	bi.lParam = (LPARAM)(initDir.c_str());
	
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
	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}
