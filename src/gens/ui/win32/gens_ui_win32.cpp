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
				    const bool openOrSave);


/**
 * init(): Initialize the Win32 UI.
 * @param argc main()'s argc. (unused)
 * @param argv main()'s argv. (unused)
 */
void GensUI::init(int argc, char *argv[])
{
	GENS_UNUSED_PARAMETER(argc);
	GENS_UNUSED_PARAMETER(argv);
	
	// Create the fonts used by the rest of the program.
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
	// TODO: Use nCmdShow from WinMain for ShowWindow()'s second parameter.
	create_gens_window();
	ShowWindow(Gens_hWnd, 1);
	
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
 * @param icon Icon.
 */
void GensUI::msgBox(const string& msg, const string& title, const MSGBOX_ICON icon)
{
	// TODO: Extend this function.
	// This function is currently merely a copy of the Glade auto-generated open_msgbox() function.
	// (Well, with an added "title" parameter.)
	
	unsigned int win32MsgIcon;
	switch (icon)
	{
		case MSGBOX_ICON_INFO:
			win32MsgIcon = MB_ICONINFORMATION;
			break;
		case MSGBOX_ICON_QUESTION:
			win32MsgIcon = MB_ICONQUESTION;
			break;
		case MSGBOX_ICON_WARNING:
			win32MsgIcon = MB_ICONWARNING;
			break;
		case MSGBOX_ICON_ERROR:
			win32MsgIcon = MB_ICONSTOP;
			break;
		case MSGBOX_ICON_NONE:
		default:
			win32MsgIcon = 0;
			break;
	}
	
	MessageBox(Gens_hWnd, msg.c_str(), title.c_str(), win32MsgIcon | MB_OK);
}


/**
 * openFile(): Show the File Open dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename filter to use.
 * @return Filename if successful; otherwise, an empty string.
 */
string GensUI::openFile(const string& title, const string& initFile, const FileFilterType filterType)
{
	return UI_Win32_OpenFile_int(title, initFile, filterType, false);
}


/**
 * saveFile(): Show the File Save dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType of filename filter to use.
 * @param retSelectedFile Pointer to string buffer to store the filename in.
 * @return Filename if successful; otherwise, an empty string.
 */
string GensUI::saveFile(const string& title, const string& initFile, const FileFilterType filterType)
{
	return UI_Win32_OpenFile_int(title, initFile, filterType, true);
}


/**
 * UI_Win32_OpenFile_int(): Show the File Open/Save dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename filter to use.
 * @param openOrSave false for Open; true for Save.
 * @return Filename if successful; otherwise, an empty string.
 */
static string UI_Win32_OpenFile_int(const string& title,
				    const string& initFile,
				    const FileFilterType filterType,
				    const bool openOrSave)
{
	char filename[GENS_PATH_MAX];
	OPENFILENAME ofn;
	
	SetCurrentDirectory(PathNames.Gens_Path);
	
	memset(filename, 0, sizeof(filename));
	memset(&ofn, 0, sizeof(OPENFILENAME));
	
	// Open Filename dialog settings
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = Gens_hWnd;
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
	if (!openOrSave)
	{
		// Open Dialog
		ofn.Flags |= OFN_FILEMUSTEXIST;
		if (!GetOpenFileName(&ofn))
			return "";
	}
	else
	{
		// Save Dialog
		ofn.Flags |= OFN_OVERWRITEPROMPT;
		if (!GetSaveFileName(&ofn))
			return "";
	}
	
	return ofn.lpstrFile;
}


/**
 * selectDir(): Show the Select Directory dialog.
 * @param title Window title.
 * @param initDir Initial directory.
 * @return Directory name if successful; otherwise, an empty string.
 */
string GensUI::selectDir(const string& title, const string& initDir)
{
	STUB;
#if 0
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initDir, AnyFile, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
#endif
	return "";
}


#if 0
/**
 * UI_GTK_FileChooser(): Show the File Chooser dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename fitler to use.
 * @param action Type of file chooser dialog.
 * @return Filename if successful; otherwise, an empty string.
 */
static string UI_GTK_FileChooser(const string& title, const string& initFile,
				 const FileFilterType filterType,
				 const GtkFileChooserAction action)
{
	gint res;
	gchar *filename;
	gchar *acceptButton;
	GtkWidget *dialog;
	GtkFileFilter *all_files_filter;
	string retFilename;
	
	if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
		acceptButton = GTK_STOCK_SAVE;
	else //if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
		acceptButton = GTK_STOCK_OPEN;
	
	dialog = gtk_file_chooser_dialog_new(title.c_str(), GTK_WINDOW(gens_window), action,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     acceptButton, GTK_RESPONSE_ACCEPT, NULL);
	
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), initFile.c_str());
	
	// Add filters.
	switch (filterType)
	{
		case ROMFile:
			UI_GTK_AddFilter_ROMFile(dialog);
			break;
		case SavestateFile:
			UI_GTK_AddFilter_SavestateFile(dialog);
			break;
		case CDImage:
			UI_GTK_AddFilter_CDImage(dialog);
			break;
		case ConfigFile:
			UI_GTK_AddFilter_ConfigFile(dialog);
			break;
		case GYMFile:
			UI_GTK_AddFilter_GYMFile(dialog);
			break;
		default:
			break;
	}
	// All Files filter
	all_files_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(all_files_filter, "All Files");
	gtk_file_filter_add_pattern(all_files_filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_files_filter);
	
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT)
	{
		// File selected.
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		retFilename = filename;
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
	
	// Return the filename.
	return retFilename;
}


/**
 * UI_GTK_AddFilter_ROMFile(): Adds filename filters for "ROMFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_ROMFile(GtkWidget* dialog)
{
	GtkFileFilter *filter;
	const char* bin  = "*.[bB][iI][nN]";
	const char* smd  = "*.[sS][mM][dD]";
	const char* gen  = "*.[gG][eE][nN]";
	const char* _32x = "*.32[xX]";
	const char* iso  = "*.[iI][sS][oO]";
	const char* chd  = "*.[cC][hH][dD]";
	const char* raw  = "*.[rR][aA][wW]";
#ifdef GENS_ZLIB
	const char* zip  = "*.[zZ][iI][pP]";
	const char* gz   = "*.[gG][zZ]";
	const char* zsg  = "*.[zZ][sS][gG]";
#endif /* GENS_ZLIB */
	const char* _7z  = "*.7[zZ]";
	
	filter = gtk_file_filter_new();
	
	// All ROM files
	gtk_file_filter_set_name(filter, "Sega CD / 32X / Genesis ROMs");
	gtk_file_filter_add_pattern(filter, bin);
	gtk_file_filter_add_pattern(filter, smd);
	gtk_file_filter_add_pattern(filter, gen);
	gtk_file_filter_add_pattern(filter, _32x);
	gtk_file_filter_add_pattern(filter, iso);
	gtk_file_filter_add_pattern(filter, chd);
	gtk_file_filter_add_pattern(filter, raw);
#ifdef GENS_ZLIB
	gtk_file_filter_add_pattern(filter, zip);
	gtk_file_filter_add_pattern(filter, gz);
	gtk_file_filter_add_pattern(filter, zsg);
#endif /* GENS_ZLIB */
	gtk_file_filter_add_pattern(filter, _7z);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	// Genesis ROM files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Genesis ROMs");
	gtk_file_filter_add_pattern(filter, bin);
	gtk_file_filter_add_pattern(filter, smd);
	gtk_file_filter_add_pattern(filter, gen);
#ifdef GENS_ZLIB
	gtk_file_filter_add_pattern(filter, zip);
	gtk_file_filter_add_pattern(filter, gz);
	gtk_file_filter_add_pattern(filter, zsg);
#endif /* GENS_ZLIB */
	gtk_file_filter_add_pattern(filter, _7z);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);	
	
	// 32X ROM files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "32X ROMs");
#ifdef GENS_ZLIB
	gtk_file_filter_add_pattern(filter, zip);
	gtk_file_filter_add_pattern(filter, gz);
#endif /* GENS_ZLIB */
	gtk_file_filter_add_pattern(filter, _32x);
	gtk_file_filter_add_pattern(filter, _7z);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	// SegaCD disc image files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "SegaCD Disc Image");
	gtk_file_filter_add_pattern(filter, iso);
	gtk_file_filter_add_pattern(filter, chd);
	gtk_file_filter_add_pattern(filter, raw);
	gtk_file_filter_add_pattern(filter, bin);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_SavestateFile(): Adds filename filters for "SavestateFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_SavestateFile(GtkWidget* dialog)
{
	GtkFileFilter *filter;
	const char* gs = "*.[gG][sS]?";
	
	// Savestate Files
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Savestate Files");
	gtk_file_filter_add_pattern(filter, gs);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_CDImage(): Adds filename filters for "CDImage".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_CDImage(GtkWidget* dialog)
{
	GtkFileFilter *filter;
	const char* bin = "*.[bb][iI][nN]";
	const char* iso = "*.[iI][sS][oO]";
	const char* chd = "*.[cC][hH][dD]";
	const char* raw = "*.[rR][aA][wW]";
	
	// SegaCD disc images
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "SegaCD Disc Image");
	gtk_file_filter_add_pattern(filter, bin);
	gtk_file_filter_add_pattern(filter, iso);
	gtk_file_filter_add_pattern(filter, chd);
	gtk_file_filter_add_pattern(filter, raw);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_ConfigFile(): Adds filename filters for "ConfigFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_ConfigFile(GtkWidget* dialog)
{
	GtkFileFilter *filter;
	const char* cfg = "*.[cC][fF][gG]";
	
	// Config files
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Gens Config File");
	gtk_file_filter_add_pattern(filter, cfg);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_GYMFile(): Adds filename filters for "GYMFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_GYMFile(GtkWidget* dialog)
{
	GtkFileFilter *filter;
	const char* cfg = "*.[gG][yY][mM]";
	
	// Config files
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "GYM File");
	gtk_file_filter_add_pattern(filter, cfg);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}
#endif
