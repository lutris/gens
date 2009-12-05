/***************************************************************************
 * Gens: (GTK+) Common UI functions.                                       *
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

// C includes.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

// GTK+ includes.
#include <gtk/gtk.h>
#include "gtk-misc.h"
#include "gtk-compat.h"

#include "emulator/g_main.hpp"
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// Message logging.
#include "macros/log_msg.h"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

#ifdef GENS_OS_MACOSX
// SDL is needed in order to set the SDL window title.
#include <SDL/SDL.h>
#endif

// Video and Input handlers.
#include "video/vdraw.h"
#include "input/input.h"


// File Chooser function
static string UI_GTK_FileChooser(const string& title, const string& initFile,
				 const FileFilterType filterType,
				 GtkWidget* owner,
				 const GtkFileChooserAction action);

// Filename filters.
static void UI_GTK_AddFilter_ROMFile(GtkWidget* dialog);
static void UI_GTK_AddFilter_SavestateFile(GtkWidget* dialog);
static void UI_GTK_AddFilter_CDImage(GtkWidget* dialog);
static void UI_GTK_AddFilter_ConfigFile(GtkWidget* dialog);
static void UI_GTK_AddFilter_GYMFile(GtkWidget* dialog);

// Sleep handler
static bool sleeping;
static gboolean GensUI_GLib_SleepCallback(gpointer data);

// Wait list.
static list<pid_t> waitList;


/**
 * init(): Initialize the GTK+ UI.
 * @param argc main()'s argc.
 * @param argv main()'s argv.
 */
void GensUI::init(int *argc, char **argv[])
{
	// Add the pixmap directories.
	add_pixmap_directory(GENS_DATADIR);
	add_pixmap_directory("images");
	
	// Initialize GTK+.
	if (!gtk_init_check(argc, argv))
	{
		// Could not initialize GTK+.
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Could not initialize GTK+.");
		exit(EXIT_FAILURE);
	}
	
	// Create the Gens window.
	gens_window_create();
	
	// Set the window title to Idle.
	setWindowTitle_Idle();
}


/**
 * update(): Update the UI.
 */
void GensUI::update(void)
{
	while (gtk_events_pending())
		gtk_main_iteration_do(FALSE);
	
	if (waitList.size() == 0)
		return;
	
	// Check for waiting processes.
	list<pid_t>::iterator iter = waitList.begin();
	while (iter != waitList.end())
	{
		pid_t pID = *iter;
		if (waitpid(pID, NULL, WNOHANG) == pID)
		{
			// Process is finished.
			list<pid_t>::iterator iter_new = iter;
			iter_new++;
			waitList.erase(iter);
			iter = iter_new;
		}
		else
		{
			// Process is not finished. Check the next process.
			iter++;
		}
	}
}


/**
 * UI_GLib_SleepCallback(): GLib callback for the sleep function.
 * @param data Pointer to data, specified in initial g_timeout_add() call.
 * @return FALSE to disable the timer.
 */
static gboolean GensUI_GLib_SleepCallback(gpointer data)
{
	GSFT_UNUSED_PARAMETER(data);
	
	sleeping = false;
	return false;
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
		// Don't process GTK+ events.
		usleep(ms * 1000);
		return;
	}
	
	// Sleep 10 ms, then check for GTK+ events.
	sleeping = true;
	g_timeout_add(ms, GensUI_GLib_SleepCallback, NULL);
	while (sleeping)
	{
		usleep(10000);
		update();
		
		// SDL_PumpEvents() is needed to update video.
		// SDL events are handled in the Input Handler.
		input_update();
	}
}


/**
 * wakeup(): Wakeup from sleep.
 */
void GensUI::wakeup(void)
{
	sleeping = false;
}


/**
 * setWindowTitle(): Sets the window title.
 * @param title New window title.
 */
void GensUI::setWindowTitle(const string& title)
{
#ifndef GENS_OS_MACOSX
	// Set the title of the GTK+ window, since the SDL window
	// is embedded in the GTK+ window.
	gtk_window_set_title(GTK_WINDOW(gens_window), title.c_str());
#else
	// Set the window title of the SDL window.
	// The GTK+ window will hopefully be removed later,
	// since MacOS X uses a system-wide menu bar.
	gtk_window_set_title(GTK_WINDOW(gens_window), title.c_str());
	SDL_WM_SetCaption(title.c_str(), title.c_str());
#endif
	
	update();
}


/**
 * setWindowVisibility(): Sets window visibility.
 * @param visibility true to show; false to hide.
 */
void GensUI::setWindowVisibility(const bool visibility)
{
	if (visibility)
		gtk_widget_show(gens_window);
	else
		gtk_widget_hide(gens_window);
	
	// Rebuild the menu bar.
	gens_window_create_menubar();
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
	
	// Determine the GTK+ message icon.
	GtkMessageType gtkMsgIcon;
	switch (style & MSGBOX_ICON_MASK)
	{
		case MSGBOX_ICON_INFO:
			gtkMsgIcon = GTK_MESSAGE_INFO;
			break;
		case MSGBOX_ICON_QUESTION:
			gtkMsgIcon = GTK_MESSAGE_QUESTION;
			break;
		case MSGBOX_ICON_WARNING:
			gtkMsgIcon = GTK_MESSAGE_WARNING;
			break;
		case MSGBOX_ICON_ERROR:
			gtkMsgIcon = GTK_MESSAGE_ERROR;
			break;
		case MSGBOX_ICON_NONE:
		default:
			// GTK_MESSAGE_OTHER was added in GTK+ 2.10
			// for custom message box icons.
#ifdef GTK_MESSAGE_OTHER
			gtkMsgIcon = GTK_MESSAGE_OTHER;
#else  /* GTK_MESSAGE_OTHER */
			gtkMsgIcon = GTK_MESSAGE_INFO;
#endif /* GTK_MESSAGE_OTHER */
			break;
	}
	gtkMsgIcon = gtkMsgIcon;
	
	// Determine the GTK+ message buttons.
	GtkButtonsType gtkButtons;
	switch (style & MSGBOX_BUTTONS_MASK)
	{
		case MSGBOX_BUTTONS_OK_CANCEL:
			gtkButtons = GTK_BUTTONS_OK_CANCEL;
			break;
		case MSGBOX_BUTTONS_YES_NO:
			gtkButtons = GTK_BUTTONS_YES_NO;
			break;
		case MSGBOX_BUTTONS_OK:
		default:
			gtkButtons = GTK_BUTTONS_OK;
			break;
	}
	
	// If no owner was specified, use the Gens window.
	if (!owner)
		owner = gens_window;
	
	gint response;
	GtkWidget *dialog = gtk_message_dialog_new(
				GTK_WINDOW(owner),
				GTK_DIALOG_MODAL,
				gtkMsgIcon,
				gtkButtons,
				"%s", msg.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	
	// Check the response.
	switch (response)
	{
		case GTK_RESPONSE_YES:
			return MSGBOX_RESPONSE_YES;
		case GTK_RESPONSE_NO:
			return MSGBOX_RESPONSE_NO;
		case GTK_RESPONSE_CANCEL:
			return MSGBOX_RESPONSE_CANCEL;
		case GTK_RESPONSE_OK:
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
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initFile, filterType, static_cast<GtkWidget*>(owner), GTK_FILE_CHOOSER_ACTION_OPEN);
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
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initFile, filterType, static_cast<GtkWidget*>(owner), GTK_FILE_CHOOSER_ACTION_SAVE);
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
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initDir, AnyFile, static_cast<GtkWidget*>(owner), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
}


/**
 * UI_GTK_FileChooser(): Show the File Chooser dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename fitler to use.
 * @param owner Window that owns this dialog.
 * @param action Type of file chooser dialog.
 * @return Filename if successful; otherwise, an empty string.
 */
static string UI_GTK_FileChooser(const string& title, const string& initFile,
				 const FileFilterType filterType, GtkWidget* owner,
				 const GtkFileChooserAction action)
{
	gint res;
	gchar *filename;
	gchar *acceptButton;
	GtkWidget *dialog;
	GtkFileFilter *all_files_filter;
	string retFilename;
	
	// If no owner was specified, use the Gens window.
	if (!owner)
		owner = gens_window;
	
	if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
		acceptButton = GTK_STOCK_SAVE;
	else //if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
		acceptButton = GTK_STOCK_OPEN;
	
	dialog = gtk_file_chooser_dialog_new(title.c_str(), GTK_WINDOW(owner), action,
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
		case AnyFile:
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
	static const char* filter_bin  = "*.[bB][iI][nN]";
	static const char* filter_smd  = "*.[sS][mM][dD]";
	static const char* filter_gen  = "*.[gG][eE][nN]";
	static const char* filter_32x = "*.32[xX]";
	static const char* filter_iso  = "*.[iI][sS][oO]";
	static const char* filter_raw  = "*.[rR][aA][wW]";
#ifdef GENS_ZLIB
	static const char* filter_zip  = "*.[zZ][iI][pP]";
	static const char* filter_gz   = "*.[gG][zZ]";
	static const char* filter_zsg  = "*.[zZ][sS][gG]";
#endif
#ifdef GENS_LZMA
	static const char* filter_7z  = "*.7[zZ]";
#endif
	static const char* filter_rar  = "*.[rR][aA][rR]";
	
	GtkFileFilter *filter = gtk_file_filter_new();
	
	// All ROM files
	gtk_file_filter_set_name(filter, "Sega CD / 32X / Genesis ROMs");
	gtk_file_filter_add_pattern(filter, filter_bin);
	gtk_file_filter_add_pattern(filter, filter_smd);
	gtk_file_filter_add_pattern(filter, filter_gen);
	gtk_file_filter_add_pattern(filter, filter_32x);
	gtk_file_filter_add_pattern(filter, filter_iso);
	gtk_file_filter_add_pattern(filter, filter_raw);
#ifdef GENS_ZLIB
	gtk_file_filter_add_pattern(filter, filter_zip);
	gtk_file_filter_add_pattern(filter, filter_gz);
	gtk_file_filter_add_pattern(filter, filter_zsg);
#endif
#ifdef GENS_LZMA
	gtk_file_filter_add_pattern(filter, filter_7z);
#endif
	gtk_file_filter_add_pattern(filter, filter_rar);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	// Genesis ROM files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Genesis ROMs");
	gtk_file_filter_add_pattern(filter, filter_bin);
	gtk_file_filter_add_pattern(filter, filter_smd);
	gtk_file_filter_add_pattern(filter, filter_gen);
#ifdef GENS_ZLIB
	gtk_file_filter_add_pattern(filter, filter_zip);
	gtk_file_filter_add_pattern(filter, filter_gz);
	gtk_file_filter_add_pattern(filter, filter_zsg);
#endif
#ifdef GENS_LZMA
	gtk_file_filter_add_pattern(filter, filter_7z);
#endif
	gtk_file_filter_add_pattern(filter, filter_rar);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);	
	
	// 32X ROM files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "32X ROMs");
	gtk_file_filter_add_pattern(filter, filter_32x);
#ifdef GENS_ZLIB
	gtk_file_filter_add_pattern(filter, filter_zip);
	gtk_file_filter_add_pattern(filter, filter_gz);
	gtk_file_filter_add_pattern(filter, filter_zsg);
#endif
#ifdef GENS_LZMA
	gtk_file_filter_add_pattern(filter, filter_7z);
#endif
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	// SegaCD disc image files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "SegaCD Disc Image");
	gtk_file_filter_add_pattern(filter, filter_iso);
	gtk_file_filter_add_pattern(filter, filter_raw);
	gtk_file_filter_add_pattern(filter, filter_bin);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_SavestateFile(): Adds filename filters for "SavestateFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_SavestateFile(GtkWidget* dialog)
{
	static const char* filter_gs = "*.[gG][sS]?";
	
	// Savestate Files
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Savestate Files");
	gtk_file_filter_add_pattern(filter, filter_gs);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_CDImage(): Adds filename filters for "CDImage".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_CDImage(GtkWidget* dialog)
{
	static const char* filter_bin = "*.[bb][iI][nN]";
	static const char* filter_iso = "*.[iI][sS][oO]";
	static const char* filter_raw = "*.[rR][aA][wW]";
	
	// SegaCD disc images
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "SegaCD Disc Image");
	gtk_file_filter_add_pattern(filter, filter_bin);
	gtk_file_filter_add_pattern(filter, filter_iso);
	gtk_file_filter_add_pattern(filter, filter_raw);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_ConfigFile(): Adds filename filters for "ConfigFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_ConfigFile(GtkWidget* dialog)
{
	static const char* filter_cfg = "*.[cC][fF][gG]";
	
	// Config files
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Gens Config File");
	gtk_file_filter_add_pattern(filter, filter_cfg);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * UI_GTK_AddFilter_GYMFile(): Adds filename filters for "GYMFile".
 * @param dialog File chooser dialog.
 */
static void UI_GTK_AddFilter_GYMFile(GtkWidget* dialog)
{
	static const char* filter_gym = "*.[gG][yY][mM]";
	
	// Config files
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "GYM File");
	gtk_file_filter_add_pattern(filter, filter_gym);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/**
 * setMousePointer(): Set the mouse pointer. 
 * @param busy True if the pointer should be busy; False if the pointer should be normal.
 */
void GensUI::setMousePointer(bool busy)
{
	if (vdraw_get_fullscreen())
		return;
	
	GdkCursor *cursor;
	
	if (busy)
		cursor = gdk_cursor_new(GDK_WATCH);
	else
		cursor = gdk_cursor_new(GDK_LEFT_PTR);
	
	gdk_window_set_cursor(gtk_widget_get_window(gens_window), cursor);
	gdk_window_set_cursor(gtk_widget_get_window(gens_window_sdlsock), cursor);
	gdk_cursor_destroy(cursor);
	
	update();
}


/**
 * launchBrowser(): Launch a web browser with the given URL.
 * @param url URL to load.
 */
void GensUI::LaunchBrowser(const string& url)
{
	// TODO: Mac OS X version.
	// TODO: Non-XDG version.
	
#ifndef GENS_OS_MACOSX
	// Use xdg-open.
	static const char xdg_open[] = "/usr/bin/xdg-open";
	if (access(xdg_open, X_OK) != 0)
	{
		if (access(xdg_open, F_OK) != 0)
		{
			LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
				"%s not found.", xdg_open);
		}
		else
		{
			LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
				"Cannot execute %s .", xdg_open);
		}
		
		return;
	}
	
	// Set the mouse cursor to busy.
	setMousePointer(true);
	update();
	
	pid_t pID = vfork();
	if (pID < 0)
	{
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"vfork() failed: %d (%s)", errno, strerror(errno));
		return;
	}
	else if (pID == 0)
	{
		// Child process. Run xdg-open.
		execl(xdg_open, xdg_open, url.c_str(), NULL);
	}
	else
	{
		// Add the PID to the wait list.
		waitList.push_back(pID);
		
		// Set the mouse cursor to normal.
		setMousePointer(false);
	}
#endif
}


/**
 * fsMinimize(): Full Screen Minimize.
 * @param fst Type of FS Minimization.
 */
void GensUI::fsMinimize(fsMinimize_Type fst)
{
	// Check if fst is valid.
	if (fst < FSMINIMIZE_DIALOG || fst >= FSMINIMIZE_MAX)
		return;
	
	// Increment the fsMinimize counter for this type.
	fsMinimize_Counter[(int)fst]++;
	if (!vdraw_get_fullscreen())
		return;
	
	if (fsMinimize_Counter[(int)fst] == 1)
	{
		// First minimization.
		vdraw_set_fullscreen(false);
		Sync_Gens_Window_GraphicsMenu();
		fsMinimize_OldFS[(int)fst] = true;
		
		// Minimize the window for Alt-Tab only.
		if (fst == FSMINIMIZE_ALTTAB)
			gtk_window_iconify(GTK_WINDOW(gens_window));
	}
}


/**
 * fsRestore() Full Screen Restore.
 * @param fst Type of FS Minimization.
 */
void GensUI::fsRestore(fsMinimize_Type fst)
{
	// Check if fst is valid.
	if (fst < FSMINIMIZE_DIALOG || fst >= FSMINIMIZE_MAX)
		return;
	
	if (fsMinimize_Counter[(int)fst] == 0)
		return;
	
	// Decrement the fsMinimize counter for this type.
	fsMinimize_Counter[(int)fst]--;
	if (!fsMinimize_OldFS[(int)fst] || vdraw_get_fullscreen())
		return;
	
	if (fsMinimize_Counter[(int)fst] == 0)
	{
		// Last restoration.
		vdraw_set_fullscreen(true);
		Sync_Gens_Window_GraphicsMenu();
		fsMinimize_OldFS[(int)fst] = false;
	}
}
