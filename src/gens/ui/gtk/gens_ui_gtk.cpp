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

#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "gens/gens_window.h"

#include "ui/gens_ui.hpp"
#include "gtk-misc.h"


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
bool sleeping;
gboolean GensUI_GLib_SleepCallback(gpointer data);


/**
 * init(): Initialize the GTK+ UI.
 * @param argc main()'s argc.
 * @param argv main()'s argv.
 */
void GensUI::init(int argc, char *argv[])
{
	// Add the pixmap directories.
	add_pixmap_directory(GENS_DATADIR);
	add_pixmap_directory("images");
	
	// Initialize GTK+.
	gtk_init(&argc, &argv);
	
	// Create and show the Gens window.
	create_gens_window();
	gtk_widget_show(gens_window);
	
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
}


/**
 * UI_GLib_SleepCallback(): GLib callback for the sleep function.
 * @param data Pointer to data, specified in initial g_timeout_add() call.
 * @return FALSE to disable the timer.
 */
gboolean GensUI_GLib_SleepCallback(gpointer data)
{
	sleeping = false;
	return false;
}


/**
 * sleep(): Sleep, but keep the UI active.
 * @param ms Interval to sleep, in milliseconds.
 */
void GensUI::sleep(const int ms)
{
	sleeping = true;
	g_timeout_add(ms, GensUI_GLib_SleepCallback, NULL);
	while (sleeping)
	{
		usleep(1000);
		update();
	}
}


/**
 * setWindowTitle(): Sets the window title.
 * @param title New window title.
 */
void GensUI::setWindowTitle(const string& title)
{
	gtk_window_set_title(GTK_WINDOW(gens_window), title.c_str());
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
}


/**
 * UI_Hide_Embedded_Window(): Hides the embedded SDL window.
 */
void UI_Hide_Embedded_Window(void)
{
	gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
}


/**
 * UI_Show_Embedded_Window(): Shows the embedded SDL window.
 * @param w Width of the embedded SDL window.
 * @param h Height of the embedded SDL window.
 */
void UI_Show_Embedded_Window(const int w, const int h)
{
	GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
	gtk_widget_set_size_request(sdlsock, w, h);
	gtk_widget_realize(sdlsock);
	gtk_widget_show(sdlsock);
}


/**
 * UI_Get_Embedded_WindowID(): Gets the window ID of the embedded SDL window.
 * @return Window ID of the embedded SDL window.
 */
int UI_Get_Embedded_WindowID(void)
{
	GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
	return GDK_WINDOW_XWINDOW(sdlsock->window);
}


/**
 * msgBox(): Show a message box.
 * @param msg Message.
 * @param title Title.
 * @param icon Icon.
 * @param owner Window that owns this dialog.
 */
void GensUI::msgBox(const string& msg, const string& title, const MSGBOX_ICON icon, void* owner)
{
	// TODO: Extend this function.
	// This function is currently merely a copy of the Glade auto-generated open_msgbox() function.
	// (Well, with an added "title" parameter.)
	
	GtkMessageType gtkMsgIcon;
	switch (icon)
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
			gtkMsgIcon = GTK_MESSAGE_OTHER;
			break;
	}
	
	// If no owner was specified, use the Gens window.
	if (!owner)
		owner = gens_window;
	
	GtkWidget *dialog = gtk_message_dialog_new(
			GTK_WINDOW(owner), GTK_DIALOG_MODAL,
			gtkMsgIcon, GTK_BUTTONS_OK, msg.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
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
