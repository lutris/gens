/**
 * GENS: Common UI functions. (GTK+)
 */

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "gens/gens_window.h"

#include "g_main.h"
#include "ui-common.h"
#include "gtk-misc.h"


// File Chooser function
static int UI_GTK_FileChooser(const char* title, const char* initFile, FileFilterType filterType,
			      char* retSelectedFile, GtkFileChooserAction action);


// Filename filters.
static void UI_GTK_AddFilter_ROMFile(GtkWidget* dialog);
static void UI_GTK_AddFilter_SavestateFile(GtkWidget* dialog);
static void UI_GTK_AddFilter_CDImage(GtkWidget* dialog);
static void UI_GTK_AddFilter_ConfigFile(GtkWidget* dialog);
static void UI_GTK_AddFilter_GYMFile(GtkWidget* dialog);

/**
 * UI_Set_Window_Title(): Sets the window title.
 * @param title New window title.
 */
void UI_Set_Window_Title(const char *title)
{
	gtk_window_set_title(GTK_WINDOW(gens_window), title);
}


/**
 * UI_Set_Window_Visibility(): Sets window visibility.
 * @param visibility 0 to hide; anything else to show.
 */
void UI_Set_Window_Visibility (int visibility)
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
void UI_Show_Embedded_Window(int w, int h)
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
 * UI_MsgBox(): Show a message box.
 * @param msg Message.
 * @param title Title.
 */
void UI_MsgBox(const char* msg, const char* title)
{
	// TODO: Extend this function.
	// This function is currently merely a copy of the Glade auto-generated open_msgbox() function.
	// (Well, with an added "title" parameter.)
	
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gens_window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, msg);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


/**
 * UI_OpenFile(): Show the File Open dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename filter to use.
 * @param retSelectedFile Pointer to string buffer to store the filename in.
 * @return 0 if successful.
 */
int UI_OpenFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile)
{
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initFile, filterType, retSelectedFile, GTK_FILE_CHOOSER_ACTION_OPEN);
}


/**
 * UI_SaveFile(): Show the File Save dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType of filename filter to use.
 * @param retSelectedFile Pointer to string buffer to store the filename in.
 * @return 0 if successful.
 */
int UI_SaveFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile)
{
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initFile, filterType, retSelectedFile, GTK_FILE_CHOOSER_ACTION_SAVE);
}


/**
 * UI_SelectDir(): Show the Select Directory dialog.
 * @param title Window title.
 * @param initDir Initial directory.
 * @param retSelectedDir Pointer to string buffer to store the directory in.
 * @return 0 if successful.
 */
int UI_SelectDir(const char* title, const char* initDir, char* retSelectedDir)
{
	// TODO: Extend this function.
	// Perhaps set the path to the last path for the function calling this...
	return UI_GTK_FileChooser(title, initDir, AnyFile, retSelectedDir, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
}


/**
 * UI_GTK_FileChooser(): Show the File Chooser dialog.
 * @param title Window title.
 * @param initFileName Initial filename.
 * @param filterType Type of filename fitler to use.
 * @param retSelectedFile Pointer to string buffer to store the filename in.
 * @param action Type of file chooser dialog.
 * @return 0 if a file was selected.
 */
static int UI_GTK_FileChooser(const char* title, const char* initFile, FileFilterType filterType,
			      char* retSelectedFile, GtkFileChooserAction action)
{
	gint res;
	gchar *filename;
	gchar *acceptButton;
	GtkWidget *dialog;
	GtkFileFilter *all_files_filter;
	
	if (retSelectedFile == NULL)
		return 1;
	
	if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
		acceptButton = GTK_STOCK_SAVE;
	else //if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
		acceptButton = GTK_STOCK_OPEN;
	
	dialog = gtk_file_chooser_dialog_new(title, GTK_WINDOW(gens_window), action,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     acceptButton, GTK_RESPONSE_ACCEPT, NULL);
	
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), initFile);
	
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
		strncpy(retSelectedFile, filename, GENS_PATH_MAX);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
	
	// Return 0 if a file was selected.
	if (res == GTK_RESPONSE_ACCEPT)
		return 0;
	
	// No filename selected.
	return 2;
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
	const char* zip  = "*.[zZ][iI][pP]";
	const char* gz   = "*.[gG][zZ]";
	const char* zsg  = "*.[zZ][sS][gG]";
	
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
	gtk_file_filter_add_pattern(filter, zip);
	gtk_file_filter_add_pattern(filter, gz);
	gtk_file_filter_add_pattern(filter, zsg);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	// Genesis ROM files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Genesis ROMs");
	gtk_file_filter_add_pattern(filter, bin);
	gtk_file_filter_add_pattern(filter, smd);
	gtk_file_filter_add_pattern(filter, gen);
	gtk_file_filter_add_pattern(filter, zip);
	gtk_file_filter_add_pattern(filter, gz);
	gtk_file_filter_add_pattern(filter, zsg);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);	
	
	// 32X ROM files only
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "32X ROMs");
	gtk_file_filter_add_pattern(filter, zip);
	gtk_file_filter_add_pattern(filter, gz);
	gtk_file_filter_add_pattern(filter, _32x);
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
