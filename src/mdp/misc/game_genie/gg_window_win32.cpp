/***************************************************************************
 * MDP: Game Genie. (Window Code) (Win32)                                  *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gg_window.hpp"
#include "gg_plugin.h"
#include "gg.hpp"
#include "gg_code.h"

// C includes.
#include <string.h>
#include <stdlib.h>

// C++ includes.
#include <list>
using std::list;

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_win32.h"

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// Window.
static HWND gg_window = NULL;
static WNDCLASS gg_window_wndclass;
static bool gg_window_child_windows_created = false;

// Window size.
#define GG_WINDOW_WIDTH  454
#define GG_WINDOW_HEIGHT 316

// Window procedure.
static LRESULT CALLBACK gg_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	txtCode;
static HWND	txtName;
static HWND	lstCodes;

// Widget creation functions.
static void	gg_window_create_child_windows(HWND hWnd);
static void	gg_window_create_lstCodes(HWND container);

// Button IDs.
#define IDAPPLY			0x000F
#define IDC_BTNADDCODE		0x0010
#define IDC_BTNDELETE		0x0011
#define IDC_BTNDEACTIVATEALL	0x0012

// Win32 instance and font.
static HINSTANCE gg_hInstance;
static HFONT gg_hFont = NULL;
static HFONT gg_hFont_title = NULL;

// Code load/save functions.
static void	gg_window_init(void);
static void	gg_window_save(void);

// Callbacks.
#if 0
static gboolean	gg_window_callback_close(HWND widget, GdkEvent *event, gpointer user_data);
static void	gg_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void	gg_window_callback_btnAddCode_clicked(GtkButton *button, gpointer user_data);
static gboolean	gg_window_callback_txtkeypress(HWND widget, GdkEventKey *event, gpointer user_data);
static void	gg_window_callback_lstCodes_toggled(GtkCellRendererToggle *cell_renderer,
						    gchar *path, gpointer user_data);
#endif
static void	gg_window_callback_delete(void);
static void	gg_window_callback_deactivate_all(void);

// Miscellaneous.
#if 0
static int	gg_window_add_code_from_textboxes(void);
static int	gg_window_add_code(const gg_code_t *gg_code, const char* name);
#endif


/**
 * gg_window_show(): Show the Game Genie window.
 * @param parent Parent window.
 */
void gg_window_show(void *parent)
{
	if (gg_window)
	{
		// Controller Configuration window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(gg_window, SW_SHOW);
		return;
	}
	
	gg_window_child_windows_created = false;
	
	// Create the window class.
	if (gg_window_wndclass.lpfnWndProc != gg_window_wndproc)
	{
		gg_window_wndclass.style = 0;
		gg_window_wndclass.lpfnWndProc = gg_window_wndproc;
		gg_window_wndclass.cbClsExtra = 0;
		gg_window_wndclass.cbWndExtra = 0;
		gg_window_wndclass.hInstance = GetModuleHandle(NULL);
		gg_window_wndclass.hIcon = NULL;
		gg_window_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		gg_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		gg_window_wndclass.lpszMenuName = NULL;
		gg_window_wndclass.lpszClassName = "gg_window_wndclass";
		
		RegisterClass(&gg_window_wndclass);
	}
	
	// Get the HINSTANCE and fonts.
	gg_hInstance = GetModuleHandle(NULL);
	gg_hFont = mdp_win32_get_message_font();
	gg_hFont_title = mdp_win32_get_title_font();
	
	// Create the window.
	gg_window = CreateWindowEx(0, "gg_window_wndclass", "Game Genie",
				   WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				   CW_USEDEFAULT, CW_USEDEFAULT,
				   GG_WINDOW_WIDTH, GG_WINDOW_HEIGHT,
				   (HWND)parent, NULL, gg_hInstance, NULL);
	
	// Window adjustment.
	mdp_win32_set_actual_window_size(gg_window, GG_WINDOW_WIDTH, GG_WINDOW_HEIGHT);
	mdp_win32_center_on_window(gg_window, (HWND)parent);
	
	UpdateWindow(gg_window);
	ShowWindow(gg_window, true);
	
	// Register the window with MDP Host Services.
	gg_host_srv->window_register(&mdp, gg_window);
}

/**
 * gg_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void gg_window_create_child_windows(HWND hWnd)
{
	if (gg_window_child_windows_created)
		return;
	
	// Strings.
	static const char* const strInfoTitle = "Information about Game Genie / Patch codes";
	static const char* const strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA:DDDD (address:data)";
	
	// Header label.
	HWND lblInfoTitle = CreateWindow(WC_STATIC, strInfoTitle,
					 WS_CHILD | WS_VISIBLE | SS_LEFT,
					 8, 8,
					 GG_WINDOW_WIDTH-16, 16,
					 hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblInfoTitle, gg_hFont_title, true);
	
	// Description label.
	HWND lblInfo = CreateWindow(WC_STATIC, strInfo,
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8+16,
				    GG_WINDOW_WIDTH-16, 68,
				    hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblInfo, gg_hFont, true);
	
	// "Code" label.
	HWND lblCode = CreateWindow(WC_STATIC, "Code",
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8+16+68+8, 32, 16,
				    hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblCode, gg_hFont, true);
	
	// "Code" textbox.
	txtCode = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				 8+32+8, 8+16+68+8,
				 GG_WINDOW_WIDTH-(8+32+8+64+8+8+16), 20,
				 hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(txtCode, gg_hFont, true);
	SendMessage(txtCode, EM_LIMITTEXT, 17, 0);
	// TODO: Subclass the "Code" textbox to handle the "Enter" key.
	
	// "Name" label.
	HWND lblName = CreateWindow(WC_STATIC, "Name",
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8+16+68+8+24, 32, 16,
				    hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(lblName, gg_hFont, true);
	
	// "Name" textbox.
	txtName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				 WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				 8+32+8, 8+16+68+8+24,
				 GG_WINDOW_WIDTH-(8+32+8+64+8+8+16), 20,
				 hWnd, NULL, gg_hInstance, NULL);
	SetWindowFont(txtName, gg_hFont, true);
	// TODO: Subclass the "Name" textbox to handle the "Enter" key.
	
	// "Add Code" button.
	HWND btnAddCode = CreateWindow(WC_BUTTON, "Add C&ode",
				       WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				       GG_WINDOW_WIDTH-(64+8+16), 8+16+68+8,
				       63+16, 20,
				       hWnd, (HMENU)IDC_BTNADDCODE, gg_hInstance, NULL);
	SetWindowFont(btnAddCode, gg_hFont, true);
	
	// Create the ListView for the Game Genie codes.
	gg_window_create_lstCodes(hWnd);
	
	// Create the dialog buttons.
	static const int btnTop = GG_WINDOW_HEIGHT-24-8;
	
	// "OK" button.
	HWND btnOK = CreateWindow(WC_BUTTON, "&OK",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  GG_WINDOW_WIDTH-(8+81+8+81+8+81+8+81+8+81), btnTop,
				  81, 24,
				  hWnd, (HMENU)IDOK, gg_hInstance, NULL);
	SetWindowFont(btnOK, gg_hFont, true);
	
	// "Apply" button.
	HWND btnApply = CreateWindow(WC_BUTTON, "&Apply",
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     GG_WINDOW_WIDTH-(8+81+8+81+8+81+8+81), btnTop,
				     81, 24,
				     hWnd, (HMENU)IDAPPLY, gg_hInstance, NULL);
	SetWindowFont(btnApply, gg_hFont, true);
	
	// "Cancel" button.
	HWND btnCancel = CreateWindow(WC_BUTTON, "&Cancel",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      GG_WINDOW_WIDTH-(8+81+8+81+8+81), btnTop,
				      81, 24,
				      hWnd, (HMENU)IDCANCEL, gg_hInstance, NULL);
	SetWindowFont(btnCancel, gg_hFont, true);
	
	// "Delete" button.
	HWND btnDelete = CreateWindow(WC_BUTTON, "&Delete",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      GG_WINDOW_WIDTH-(8+81+8+81), btnTop,
				      81, 24,
				      hWnd, (HMENU)IDC_BTNDELETE, gg_hInstance, NULL);
	SetWindowFont(btnDelete, gg_hFont, true);
	
	// "Deactivate All" button.
	HWND btnDeactivateAll = CreateWindow(WC_BUTTON, "Deac&tivate All",
					     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					     GG_WINDOW_WIDTH-(8+81), btnTop,
					     81, 24,
					     hWnd, (HMENU)IDC_BTNDEACTIVATEALL, gg_hInstance, NULL);
	SetWindowFont(btnDeactivateAll, gg_hFont, true);
	
	// Initialize the Game Genie code listview.
	gg_window_init();
}


/**
 * gg_window_create_lstCodes(): Create the treeview for the Game Genie codes.
 * @param container Container for the listview.
 */
static void gg_window_create_lstCodes(HWND container)
{
	// ListView containing the Game Genie codes.
	lstCodes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT,
				  8, 8+16+68+8+24+24,
				  GG_WINDOW_WIDTH-(8+8), 128,
				  container, NULL, gg_hInstance, NULL);
	SetWindowFont(lstCodes, gg_hFont, true);
	SendMessage(lstCodes, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// CPU
	lvCol.pszText = "CPU";
	lvCol.cx = 48;
	SendMessage(lstCodes, LVM_INSERTCOLUMN, 0, (LPARAM)&lvCol);
	
	// Code (Hex)
	lvCol.pszText = "Code (Hex)";
	lvCol.cx = 96;
	SendMessage(lstCodes, LVM_INSERTCOLUMN, 1, (LPARAM)&lvCol);
	
	// Code (GG)
	lvCol.pszText = "Code (GG)";
	lvCol.cx = 96;
	SendMessage(lstCodes, LVM_INSERTCOLUMN, 2, (LPARAM)&lvCol);
	
	// Name
	lvCol.pszText = "Name";
	lvCol.cx = 192;
	SendMessage(lstCodes, LVM_INSERTCOLUMN, 3, (LPARAM)&lvCol);
}


/**
 * gg_window_close(): Close the Game Genie window.
 */
void gg_window_close(void)
{
	if (!gg_window)
		return;
	
	// Unregister the window from MDP Host Services.
	gg_host_srv->window_unregister(&mdp, gg_window);
	
#if 0
	// Delete any gg_code pointers that may be stored in the list store.
	if (lmCodes)
	{
		GtkTreeIter iter;
		gg_code_t *stored_code;
		
		gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
		for (int i = 0; valid == true; i++)
		{
			gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 5, &stored_code, -1);
			
			// Delete the code.
			free(stored_code);
			
			// Get the next list element.
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
		}
		
		// Clear the list store.
		gtk_list_store_clear(GTK_LIST_STORE(lmCodes));
	}
#endif
	
	// Destroy the window.
	HWND tmp = gg_window;
	gg_window = NULL;
	DestroyWindow(tmp);
	
	// Delete the fonts.
	DeleteFont(gg_hFont);
	DeleteFont(gg_hFont_title);
	gg_hFont = NULL;
	gg_hFont_title = NULL;
}


/**
 * gg_window_init(): Initialize the Game Genie code listview.
 */
static void gg_window_init(void)
{
#if 0
	// Add all loaded codes to the listview.
	for (list<gg_code_t>::iterator iter = gg_code_list.begin();
	     iter != gg_code_list.end(); iter++)
	{
		gg_window_add_code(&(*iter), NULL);
	}
#endif
}


/**
 * gg_window_save(): Save the Game Genie codes.
 */
static void gg_window_save(void)
{
	// TODO: Undo any ROM-modifying codes.
	
#if 0
	// Clear the code list.
	gg_code_list.clear();
	
	gg_code_t gg_code;
	
	// Values retrieved from the list model.
	gg_code_t *stored_code;
	gboolean enabled;
	
	// Add all codes in the listview to the code list.
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
	for (int i = 0; valid == true; i++)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 0, &enabled, 5, &stored_code, -1);
		
		// Copy the code.
		memcpy(&gg_code, stored_code, sizeof(gg_code));
		
		// Copy the "enabled" value.
		gg_code.enabled = enabled;
		
		// Add the code to the list of codes.
		gg_code_list.push_back(gg_code);
		
		// Get the next list element.
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
	}
#endif
	
	// TODO: Apply codes if a game is running.
}


/**
 * gg_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK gg_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			gg_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					gg_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					gg_window_save();
					break;
				case IDC_BTNDELETE:
					gg_window_callback_delete();
					break;
				case IDC_BTNDEACTIVATEALL:
					gg_window_callback_deactivate_all();
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_DESTROY:
			gg_window_close();
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


#if 0
/**
 * gg_window_callback_btnAddCode_clicked(): "Add Code" button was clicked.
 * @param button Button widget.
 * @param user_data User data.
 */
static void gg_window_callback_btnAddCode_clicked(GtkButton *button, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(button);
	MDP_UNUSED_PARAMETER(user_data);
	
	gg_window_add_code_from_textboxes();
}


/**
 * gg_window_callback_txtkeypress(): A key was pressed in a txtEntry textbox.
 * @param button Button widget.
 * @param user_data User data.
 */
static gboolean	gg_window_callback_txtkeypress(HWND widget, GdkEventKey *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(user_data);
	
	if (event->keyval == GDK_Return ||
	    event->keyval == GDK_KP_Enter)
	{
		// "Enter" was pressed. Add the code.
		gg_window_add_code_from_textboxes();
		
		// Stop processing the key.
		return TRUE;
	}
	
	// Continue processing the key.
	return FALSE;
}


/**
 * gg_window_add_code_from_textboxes(): Add a code to the treeview from the textboxes.
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code_from_textboxes(void)
{
	// Decode the code.
	// TODO: Add support for more CPUs, datasizes, etc.
	gg_code_t gg_code;
	gg_code.name[0] = 0x00;
	
	const gchar* code_txt = gtk_entry_get_text(GTK_ENTRY(txtCode));
	GG_CODE_ERR gcp_rval = gg_code_parse(code_txt, &gg_code, CPU_M68K);
	
	if (gcp_rval != GGCE_OK)
	{
		// Error parsing the code.
		char err_msg_full[1024];
		char err_msg[1024];
		
		switch (gcp_rval)
		{
			case GGCE_UNRECOGNIZED:
				strcpy(err_msg, "The code could not be parsed correctly.");
				break;
			case GGCE_ADDRESS_RANGE:
				// TODO: Show range depending on the selected CPU.
				strcpy(err_msg, "The address for this code is out of range for the specified CPU.\n"
						"Valid range for MC68000 CPUs: 0x000000 - 0xFFFFFF");
				break;
			case GGCE_ADDRESS_ALIGNMENT:
				// TODO: Show range and alignment info based on CPU and datasize.
				strcpy(err_msg, "The address is not aligned properly for the specified data.\n"
						"For MC68000, 16-bit data must be stored at even addresses.");
				break;
			case GGCE_DATA_TOO_LARGE:
				strcpy(err_msg, "The data value is too large. Usually, this means that you"
						"entered too many characters.");
				break;
			default:
				// Other error.
				sprintf(err_msg, "Unknown error code %d.", gcp_rval);
				break;
		}
		
		sprintf(err_msg_full, "The specified code, \"%s\", could not be added due to an error:\n\n%s",
				      code_txt, err_msg);
		
		// Show an error message.
		HWND msgbox = gtk_message_dialog_new(
					GTK_WINDOW(gg_window),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s", err_msg_full);
		gtk_window_set_title(GTK_WINDOW(msgbox), "Game Genie: Code Error");
		gtk_dialog_run(GTK_DIALOG(msgbox));
		gtk_widget_destroy(msgbox);
		
		return gcp_rval;
	}
	
	int ggw_ac_rval = gg_window_add_code(&gg_code, gtk_entry_get_text(GTK_ENTRY(txtName)));
	if (ggw_ac_rval == 0)
	{
		// Code added successfully.
		// Clear the textboxes and set focus to the "Code" textbox.
		gtk_entry_set_text(GTK_ENTRY(txtCode), "");
		gtk_entry_set_text(GTK_ENTRY(txtName), "");
		gtk_widget_grab_focus(txtCode);
	}
	
	return ggw_ac_rval;
}
	
/**
 * gg_window_add_code(): Add a code to the listview.
 * @param gg_code Pointer to gg_code_t containing the code to add.
 * @param name Name of the code. (If NULL, the name in gg_code is used.)
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code(const gg_code_t *gg_code, const char* name)
{
	if (!gg_code)
		return 1;
	
	// Create the hex version of the code.
	char s_code_hex[32];
	
	switch (gg_code->cpu)
	{
		case CPU_M68K:
		case CPU_S68K:
		{
			
			// 68000: 24-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					sprintf(s_code_hex, "%06X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					sprintf(s_code_hex, "%06X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					sprintf(s_code_hex, "%06X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					// TODO: Show an error message.
					return 1;
			}
			break;
		}
		case CPU_Z80:
		{
			// Z80: 16-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					sprintf(s_code_hex, "%04X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					sprintf(s_code_hex, "%04X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					sprintf(s_code_hex, "%04X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					// TODO: Show an error message.
					return 1;
			}
			break;
		}
		case CPU_MSH2:
		case CPU_SSH2:
		{
			
			// SH2: 32-bit address.
			switch (gg_code->datasize)
			{
				case DS_BYTE:
					sprintf(s_code_hex, "%08X:%02X", gg_code->address, gg_code->data);
					break;
				case DS_WORD:
					sprintf(s_code_hex, "%08X:%04X", gg_code->address, gg_code->data);
					break;
				case DS_DWORD:
					sprintf(s_code_hex, "%08X:%08X", gg_code->address, gg_code->data);
					break;
				case DS_INVALID:
				default:
					// Invalid code.
					// TODO: Show an error message.
					return 1;
			}
			break;
		}
		case CPU_INVALID:
		default:
			// Invalid code.
			// TODO: Show an error message.
			return 1;
	}
	
	// CPU string.
	static const char* const s_cpu_list[8] = {NULL, "M68K", "S68K", "Z80", "MSH2", "SSH2", NULL, NULL};
	const char* s_cpu = s_cpu_list[(int)(gg_code->cpu) & 0x07];
	if (!s_cpu)
		return 1;
	
	// Determine what should be used for the Game Genie code.
	const char* s_code_gg;
	if (!gg_code->game_genie[0])
	{
		// The code can't be converted to Game Genie.
		s_code_gg = "N/A";
	}
	else
	{
		// The code can be converted to Game Genie.
		s_code_gg = gg_code->game_genie;
	}
	
	// Store the gg_code_t in the list store.
	gg_code_t *lst_code = (gg_code_t*)(malloc(sizeof(gg_code_t)));
	memcpy(lst_code, gg_code, sizeof(gg_code_t));
	
	// If no name is given, use the name in the gg_code.
	if (!name)
		name = &gg_code->name[0];
	else
	{
		// Copy the specified name into lst_code.
		memcpy(lst_code->name, name, sizeof(lst_code->name));
		lst_code->name[sizeof(lst_code->name)-1] = 0x00;
	}
	
	// Code is decoded. Add it to the listview.
	GtkTreeIter iter;
	gtk_list_store_append(lmCodes, &iter);
	gtk_list_store_set(GTK_LIST_STORE(lmCodes), &iter,
			   0, FALSE,		// Disable the code by default.
			   1, s_cpu,		// CPU.
			   2, s_code_hex,	// Hex code.
			   3, s_code_gg,	// Game Genie code. (if applicable)
			   4, name,		// Code name.
			   5, lst_code, -1);	// gg_code_t
	
	// Code added successfully.
	return 0;
}


/**
 * gg_window_callback_lstCodes_toggled(): Code was toggled.
 * @param cell_renderer
 * @param path
 * @param user_data Pointer to the list model.
 */
static void gg_window_callback_lstCodes_toggled(GtkCellRendererToggle *cell_renderer,
						gchar *path, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(cell_renderer);
	
	// Toggle the state of this item.
	GtkTreeIter iter;
	gboolean cur_state;
	
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(user_data), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(user_data), &iter, 0, &cur_state, -1);
	gtk_list_store_set(GTK_LIST_STORE(user_data), &iter, 0, !cur_state, -1);
}
#endif


/**
 * gg_window_callback_delete(): Delete the selected code(s).
 */
static void gg_window_callback_delete(void)
{
#if 0
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	gboolean need_check, row_erased, valid;
	
	// Get the current selection.
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(lstCodes));
	
	// Delete all selected codes.
	need_check = true;
	while (need_check)
	{
		row_erased = false;
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
		while (valid && !row_erased)
		{
			if (gtk_tree_selection_iter_is_selected(selection, &iter))
			{
				// Delete the gg_code_t first.
				gg_code_t *stored_code;
				gtk_tree_model_get(GTK_TREE_MODEL(lmCodes), &iter, 4, &stored_code, -1);
				free(stored_code);
				
				// Delete the row.
				gtk_list_store_remove(lmCodes, &iter);
				row_erased = true;
			}
			else
			{
				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
			}
		}
		if (!valid && !row_erased)
			need_check = false;
	}
#endif
}


/**
 * gg_window_callback_deactivate_all(): Deactivate all codes.
 */
static void gg_window_callback_deactivate_all(void)
{
#if 0
	GtkTreeIter iter;
	gboolean valid;
	
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lmCodes), &iter);
	while (valid)
	{
		gtk_list_store_set(GTK_LIST_STORE(lmCodes), &iter, 0, 0, -1);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lmCodes), &iter);
	}
#endif
}
