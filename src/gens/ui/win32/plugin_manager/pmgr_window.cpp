/***************************************************************************
 * Gens: (Win32) Plugin Manager Window.                                    *
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

#include "pmgr_window.hpp"
#include "ui/common/pmgr_window_common.hpp"
#include "gens/gens_window.h"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"
#include "ui/win32/charset.hpp"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// C++ includes.
#include <string>
#include <sstream>
#include <list>
#include <vector>
using std::string;
using std::stringstream;
using std::list;
using std::vector;

// Win32 line ending.
// For some reason, MinGW's std::endl always outputs "\n", even on Windows.
// This works on Wine, but not on Windows.
#define WIN32_ENDL "\r\n"


// Window.
HWND pmgr_window = NULL;

// Window class.
static WNDCLASS pmgr_wndclass;

// Window size.
#define PMGR_WINDOW_WIDTH  (320+8+8)
#define PMGR_WINDOW_HEIGHT (8+196+8+248+8+24+8)

#define PMGR_FRAME_PLUGIN_LIST_WIDTH  (PMGR_WINDOW_WIDTH-8-8)
#define PMGR_FRAME_PLUGIN_LIST_HEIGHT 196

#define PMGR_FRAME_PLUGIN_INFO_WIDTH  (PMGR_WINDOW_WIDTH-8-8)
#define PMGR_FRAME_PLUGIN_INFO_HEIGHT 248

// Window procedure.
static LRESULT CALLBACK pmgr_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	lstPluginList;
static HWND	lblPluginMainInfo;
static HWND	lblPluginSecInfo;
static HWND	lblPluginDesc;

// Widget creation functions.
static void	pmgr_window_create_child_windows(HWND hWnd);
static void	pmgr_window_create_plugin_list_frame(HWND container);
static void	pmgr_window_create_plugin_info_frame(HWND container);
static void	pmgr_window_populate_plugin_list(void);

// Callbacks.
static void	pmgr_window_callback_lstPluginList_cursor_changed(void);

// Plugin icon functions and variables.
#ifdef GENS_PNG
static HWND	imgPluginIcon;
static HIMAGELIST	imglPluginIcons = NULL;
static vector<HBITMAP>	vectPluginIcons;

static void	pmgr_window_create_plugin_icon_widget(HWND container);
static HBITMAP	pmgr_window_create_bitmap_from_png(const uint8_t *icon, const unsigned int iconLength);
static inline unsigned int pmgr_window_get_bg_color(void);
#endif


/**
 * pmgr_window_show(): Show the Plugin Manager window.
 */
void pmgr_window_show(void)
{
	if (pmgr_window)
	{
		// Plugin Manager window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(pmgr_window, SW_SHOW);
		return;
	}
	
	if (pmgr_wndclass.lpfnWndProc != pmgr_window_wndproc)
	{
		// Create the window class.
		pmgr_wndclass.style = 0;
		pmgr_wndclass.lpfnWndProc = pmgr_window_wndproc;
		pmgr_wndclass.cbClsExtra = 0;
		pmgr_wndclass.cbWndExtra = 0;
		pmgr_wndclass.hInstance = ghInstance;
		pmgr_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC));
		pmgr_wndclass.hCursor = NULL;
		pmgr_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		pmgr_wndclass.lpszMenuName = NULL;
		pmgr_wndclass.lpszClassName = TEXT("pmgr_window");
		
		RegisterClass(&pmgr_wndclass);
	}
	
	// Create the window.
	pmgr_window = CreateWindow(TEXT("pmgr_window"), TEXT("Plugin Manager"),
				   WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				   CW_USEDEFAULT, CW_USEDEFAULT,
				   PMGR_WINDOW_WIDTH, PMGR_WINDOW_HEIGHT,
				   gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(pmgr_window, PMGR_WINDOW_WIDTH, PMGR_WINDOW_HEIGHT);
	
	// Center the window on the Gens window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	Win32_centerOnGensWindow(pmgr_window);
	
	UpdateWindow(pmgr_window);
	ShowWindow(pmgr_window, 1);
}


/**
 * cc_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void pmgr_window_create_child_windows(HWND hWnd)
{
	// Create the plugin list frame.
	pmgr_window_create_plugin_list_frame(hWnd);
	
	// Create the plugin information frame.
	pmgr_window_create_plugin_info_frame(hWnd);
	
	// Create the "OK" button.
	HWND btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  PMGR_WINDOW_WIDTH-8-75, PMGR_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Populate the plugin list.
	pmgr_window_populate_plugin_list();
	
	// Initialize the plugin description frame.
	pmgr_window_callback_lstPluginList_cursor_changed();
}


/**
 * pmgr_window_create_plugin_list_frame(): Create the plugin list frame.
 * @param container Container for the frame.
 */
static void pmgr_window_create_plugin_list_frame(HWND container)
{
	// Create the plugin list frame.
	HWND fraPluginList = CreateWindow(WC_BUTTON, TEXT("Internal Plugins"),
					  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					  8, 8,
					  PMGR_FRAME_PLUGIN_LIST_WIDTH,
					  PMGR_FRAME_PLUGIN_LIST_HEIGHT,
					  container, NULL, ghInstance, NULL);
	SetWindowFont(fraPluginList, fntMain, true);
	
#ifdef GENS_PNG
	// Create the ImageList.
	imglPluginIcons = ImageList_Create(32, 32, ILC_MASK | ILC_COLOR32,
					   PluginMgr::lstMDP.size(),
					   PluginMgr::lstMDP.size());
	vectPluginIcons.clear();
	vectPluginIcons.reserve(PluginMgr::lstMDP.size());
#endif
	
	// Create the plugin ListView.
	lstPluginList = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
				       WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | WS_VSCROLL |
				       LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
				       8+8, 8+16,
				       PMGR_FRAME_PLUGIN_LIST_WIDTH-16,
				       PMGR_FRAME_PLUGIN_LIST_HEIGHT-24,
				       container, (HMENU)IDC_PMGR_WINDOW_LSTPLUGINLIST, ghInstance, NULL);
	SetWindowFont(lstPluginList, fntMain, true);
	ListView_SetExtendedListViewStyle(lstPluginList, LVS_EX_FULLROWSELECT);
	
#ifdef GENS_PNG
	// Set the ListView's ImageList.
	// "Small" is set in addition to "Normal", since LVS_REPORT uses "Small" icons.
	ListView_SetImageList(lstPluginList, imglPluginIcons, LVSIL_NORMAL);
	ListView_SetImageList(lstPluginList, imglPluginIcons, LVSIL_SMALL);
#endif
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// Icon.
	lvCol.pszText = TEXT("Icon");
#ifdef GENS_PNG
	lvCol.cx = 32+8;
#else
	lvcol.cx = 0;
#endif
	ListView_InsertColumn(lstPluginList, 0, &lvCol);
	
	// Plugin name.
	lvCol.pszText = TEXT("Plugin Name");
	lvCol.cx = PMGR_FRAME_PLUGIN_LIST_WIDTH-16-32-8-24;
	ListView_InsertColumn(lstPluginList, 1, &lvCol);
}


/**
 * pmgr_window_create_plugin_info_frame(): Create the plugin information frame.
 * @param container Container for the frame.
 */
static void pmgr_window_create_plugin_info_frame(HWND container)
{
	const int top = 8+PMGR_FRAME_PLUGIN_LIST_HEIGHT+8;
	
	HWND fraPluginInfo = CreateWindow(WC_BUTTON, TEXT("Plugin Information"),
					  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					  8, top,
					  PMGR_FRAME_PLUGIN_INFO_WIDTH, PMGR_FRAME_PLUGIN_INFO_HEIGHT,
					  container, NULL, ghInstance, NULL);
	SetWindowFont(fraPluginInfo, fntMain, true);
	
#ifdef GENS_PNG
	// Create the plugin icon widget.
	pmgr_window_create_plugin_icon_widget(container);
#endif /* GENS_PNG */
	
	// Label for the main plugin info.
#ifdef GENS_PNG
	const int lblPluginMainInfo_Left = 8+8+32+8;
#else /* !GENS_PNG */
	const int lblPluginMainInfo_Left = 8+8;
#endif /* GENS_PNG */
	const int lblPluginMainInfo_Height = 96;
	
	lblPluginMainInfo = CreateWindow(WC_EDIT, NULL,
					 WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
					 lblPluginMainInfo_Left, top+16,
					 PMGR_FRAME_PLUGIN_INFO_WIDTH - lblPluginMainInfo_Left,
					 lblPluginMainInfo_Height,
					 container, NULL, ghInstance, NULL);
	SetWindowFont(lblPluginMainInfo, fntMain, true);
	Edit_SetReadOnly(lblPluginMainInfo, true);
	
	// Label for secondary plugin info.
	const int lblPluginSecInfo_Height = 40;
	lblPluginSecInfo = CreateWindow(WC_EDIT, NULL,
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
					8+8, top+16+lblPluginMainInfo_Height+8,
					PMGR_FRAME_PLUGIN_INFO_WIDTH-8-8,
					lblPluginSecInfo_Height,
					container, NULL, ghInstance, NULL);
	SetWindowFont(lblPluginSecInfo, fntMain, true);
	Edit_SetReadOnly(lblPluginSecInfo, true);
	
	// Label for the plugin description.
	const int lblPluginDesc_Height = 72;
	lblPluginDesc = CreateWindow(WC_EDIT, NULL,
				     WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
				     8+8, top+16+lblPluginMainInfo_Height+8+lblPluginSecInfo_Height+8,
				     PMGR_FRAME_PLUGIN_INFO_WIDTH-8-8,
				     lblPluginDesc_Height,
				     container, NULL, ghInstance, NULL);
	SetWindowFont(lblPluginDesc, fntMain, true);
	Edit_SetReadOnly(lblPluginDesc, true);
}


/**
 * pmgr_window_populate_plugin_list(): Populate the plugin list.
 */
static void pmgr_window_populate_plugin_list(void)
{
	if (!lstPluginList)
		return;
	
	// Clear the plugin list.
	ListView_DeleteAllItems(lstPluginList);
	
	// Add all plugins to the listbox.
	char tmp[64];
	list<mdp_t*>::iterator curPlugin;
	for (curPlugin = PluginMgr::lstMDP.begin();
	     curPlugin != PluginMgr::lstMDP.end(); curPlugin++)
	{
		mdp_t *plugin = (*curPlugin);
		const char *pluginName;
		if (plugin->desc && plugin->desc->name)
		{
			pluginName = plugin->desc->name;
		}
		else
		{
			// No description or name.
			// TODO: For external plugins, indicate the external file.
			sprintf(tmp, "[No name: 0x%08X]", (unsigned int)plugin);
			pluginName = tmp;
		}
		
		LVITEM lviPlugin;
		memset(&lviPlugin, 0x00, sizeof(lviPlugin));
		
#ifdef GENS_PNG
		// Add the plugin's icon to the ImageList.
		lviPlugin.iImage = -1;
		HBITMAP hbmpIcon = NULL;
		if (plugin->desc)
		{
			hbmpIcon = pmgr_window_create_bitmap_from_png(plugin->desc->icon, plugin->desc->iconLength);
		}
		
		if (hbmpIcon)
		{
			lviPlugin.iImage = ImageList_Add(imglPluginIcons, hbmpIcon, NULL);
			vectPluginIcons.push_back(hbmpIcon);
		}
#endif
		
#ifdef GENS_PNG
		lviPlugin.mask = LVIF_IMAGE | LVIF_PARAM;
#else
		lviPlugin.mask = LVIF_PARAM;
#endif
		lviPlugin.cchTextMax = 256;
		lviPlugin.iItem = ListView_GetItemCount(lstPluginList);
		lviPlugin.lParam = (LPARAM)plugin;
		
		// First column: Icon.
		lviPlugin.iSubItem = 0;
		lviPlugin.pszText = NULL;
		ListView_InsertItem(lstPluginList, &lviPlugin);
		
		// lParam doesn't need to be set for the subitems.
		lviPlugin.mask = LVIF_TEXT;
		lviPlugin.lParam = NULL;
		
		// Second column: Plugin name.
		lviPlugin.iSubItem = 1;
		lviPlugin.pszText = const_cast<char*>(pluginName);
		ListView_SetItem(lstPluginList, &lviPlugin);
	}
}


/**
 * pmgr_window_close(): Close the Plugin Manager window.
 */
void pmgr_window_close(void)
{
	if (!pmgr_window)
		return;
	
	// Destroy the window.
	DestroyWindow(pmgr_window);
	pmgr_window = NULL;
	
#ifdef GENS_PNG
	// Destroy the image list.
	if (imglPluginIcons)
	{
		ImageList_Destroy(imglPluginIcons);
		imglPluginIcons = NULL;
	}
	
	// Destroy the vector of HBITMAPs.
	for (int i = vectPluginIcons.size() - 1; i >= 0; i--)
	{
		DeleteBitmap(vectPluginIcons[i]);
	}
	vectPluginIcons.clear();
#endif
}


/**
 * pmgr_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK pmgr_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			pmgr_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					pmgr_window_close();
					break;
				
				default:
					// Unknown command identifier.
					break;
			}			
			break;
		
		case WM_NOTIFY:
		{
			LPNMLISTVIEW nmListView = (LPNMLISTVIEW)lParam;
			
			switch (nmListView->hdr.code)
			{
				case LVN_ITEMCHANGED:
					if (nmListView->hdr.idFrom == IDC_PMGR_WINDOW_LSTPLUGINLIST &&
					    (nmListView->uChanged & LVIF_STATE) &&
					    (!(nmListView->uOldState & LVIS_SELECTED)) &&
					    (nmListView->uNewState & LVIS_SELECTED))
					{
						// Item was newly selected.
						pmgr_window_callback_lstPluginList_cursor_changed();
					}
					break;
			}
			break;
		}
		
		case WM_DESTROY:
			if (hWnd != pmgr_window)
				break;
			
			pmgr_window = NULL;
			
#ifdef GENS_PNG
			// Destroy the image list.
			if (imglPluginIcons)
			{
				ImageList_Destroy(imglPluginIcons);
				imglPluginIcons = NULL;
			}
			
			// Destroy the vector of HBITMAPs.
			for (int i = vectPluginIcons.size() - 1; i >= 0; i--)
			{
				DeleteBitmap(vectPluginIcons[i]);
			}
			vectPluginIcons.clear();
#endif
			
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * pmgr_window_callback_lstPluginList_cursor_changed(): Cursor position has changed.
 */
static void pmgr_window_callback_lstPluginList_cursor_changed(void)
{
	// Check which plugin is clicked.
	int index = ListView_GetNextItem(lstPluginList, -1, LVNI_SELECTED);
	
	if (index == -1)
	{
		// No plugin selected.
		Edit_SetText(lblPluginMainInfo, TEXT("No plugin selected."));
		Edit_SetText(lblPluginSecInfo, NULL);
		Edit_SetText(lblPluginDesc, NULL);
#ifdef GENS_PNG
		SendMessage(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
#endif
		return;
	}
	
	// Found a selected plugin.
	
	LVITEM lvItem;
	lvItem.iItem = index;
	lvItem.iSubItem = 0;
#ifdef GENS_PNG
	lvItem.mask = LVIF_IMAGE | LVIF_PARAM;
#else
	lvItem.mask = LVIF_PARAM;
#endif
	int rval = ListView_GetItem(lstPluginList, &lvItem);
	
	mdp_t *plugin = reinterpret_cast<mdp_t*>(lvItem.lParam);
	
	// Get the plugin information.
	if (!rval || !plugin)
	{
		// Invalid plugin.
		Edit_SetText(lblPluginMainInfo, TEXT("Invalid plugin selected."));
		Edit_SetText(lblPluginSecInfo, NULL);
		Edit_SetText(lblPluginDesc, NULL);
#ifdef GENS_PNG
		SendMessage(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
#endif
		return;
	}
	
	if (!plugin->desc)
	{
		Edit_SetText(lblPluginMainInfo, TEXT("This plugin does not have a valid description field."));
		Edit_SetText(lblPluginSecInfo, NULL);
		Edit_SetText(lblPluginDesc, NULL);
#ifdef GENS_PNG
		SendMessage(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
#endif
		return;
	}
	
	// Fill in the descriptions.
	mdp_desc_t *desc = plugin->desc;
	stringstream ssMainInfo;
	
	// Plugin name.
	ssMainInfo << "Name: " << (desc->name ? charset_utf8_to_cp1252(desc->name) : "(none)") << WIN32_ENDL;
	
	// Plugin version.
	ssMainInfo << "Version: " << MDP_VERSION_MAJOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_MINOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_REVISION(plugin->pluginVersion) << WIN32_ENDL;
	
	// Plugin author.
	ssMainInfo << "MDP Author: " + (desc->author_mdp ? charset_utf8_to_cp1252(desc->author_mdp) : "(none)") << WIN32_ENDL;
	
	// Original code author.
	if (desc->author_orig)
	{
		ssMainInfo << "Original Author: " << charset_utf8_to_cp1252(desc->author_orig) << WIN32_ENDL;
	}
	
	// Website.
	if (desc->website)
	{
		ssMainInfo << "Website: " << charset_utf8_to_cp1252(desc->website) << WIN32_ENDL;
	}
	
	// License.
	ssMainInfo << "License: " + ((desc->license && desc->license[0]) ? charset_utf8_to_cp1252(desc->license) : "(none)");
	
	// Set the main plugin information.
	Edit_SetText(lblPluginMainInfo, ssMainInfo.str().c_str());
	
	// UUID.
	string sUUID = UUIDtoString(plugin->uuid);
	
	// Secondary plugin information.
	// Includes UUID and CPU flags.
	stringstream ssSecInfo;
	ssSecInfo << "UUID: " << sUUID << WIN32_ENDL
		  << GetCPUFlags_string(plugin->cpuFlagsRequired, plugin->cpuFlagsSupported, false);
	
	// Set the secondary information label.
	Edit_SetText(lblPluginSecInfo, ssSecInfo.str().c_str());
	
	// Plugin description.
	if (desc->description)
	{
		string pluginDesc = string("Description:") + WIN32_ENDL + charset_utf8_to_cp1252(desc->description);
		Edit_SetText(lblPluginDesc, pluginDesc.c_str());
	}
	else
	{
		Edit_SetText(lblPluginDesc, NULL);
	}
	
#ifdef GENS_PNG
	// Set the plugin icon.
	HBITMAP hbmpIcon = NULL;
	
	if (lvItem.iImage >= 0 && lvItem.iImage < vectPluginIcons.size())
		hbmpIcon = vectPluginIcons[lvItem.iImage];
	
	SendMessage(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpIcon);
#endif
}


#ifdef GENS_PNG
/**
 * pmgr_window_create_plugin_icon_widget(): Create the plugin icon widget and bitmap.
 * @param container Container for the plugin icon widget.
 */
static void pmgr_window_create_plugin_icon_widget(HWND container)
{
	// Plugin icon widget.
	const int top = 8+PMGR_FRAME_PLUGIN_LIST_HEIGHT+8;
	imgPluginIcon = CreateWindow(WC_STATIC, NULL,
				     WS_CHILD | WS_VISIBLE | SS_BITMAP,
				     8+8, top+16, 32, 32,
				     container, NULL, ghInstance, NULL);
	
	// Clear the icon.
	SendMessage(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
}


/**
 * pmgr_window_create_bitmap_from_png(): Create a bitmap from a PNG image.
 * @param icon Icon data. (PNG format)
 * @param iconLength Length of the icon data.
 * @return Bitmap containing the icon, or NULL on error.
 */
static HBITMAP pmgr_window_create_bitmap_from_png(const uint8_t *icon, const unsigned int iconLength)
{
	static const unsigned char pngMagicNumber[8] = {0x89, 'P', 'N', 'G',0x0D, 0x0A, 0x1A, 0x0A};
	
	if (!icon || iconLength < sizeof(pngMagicNumber))
		return NULL;
	
	// Check that the icon is in PNG format.
	if (memcmp(icon, pngMagicNumber, sizeof(pngMagicNumber)))
	{
		// Not in PNG format.
		return NULL;
	}

	// Initialize libpng.
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return NULL;
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// TODO: Is setjmp() really necessary?
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Set the custom read function.
	pmgr_window_png_dataptr = icon;
	pmgr_window_png_datalen = iconLength;
	pmgr_window_png_datapos = 0;
	
	void *read_io_ptr = png_get_io_ptr(png_ptr);
	png_set_read_fn(png_ptr, read_io_ptr, &pmgr_window_png_user_read_data);
	
	// Get the PNG information.
	png_read_info(png_ptr, info_ptr);
	
	// Get the PNG information.
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	bool has_alpha = false;
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_method);
	
	
	if (width != 32 || height != 32)
	{
		// Not 32x32.
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Make sure RGB color is used.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY)
		png_set_gray_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		has_alpha = true;
	
	// Win32 expects BGRA format.
	png_set_bgr(png_ptr);
	
	// Convert tRNS to alpha channel.
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		has_alpha = true;
	}
	
	// Convert 16-bit per channel PNG to 8-bit.
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	
	// Get the new PNG information.
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_method);
	
	// Check if the PNG image has an alpha channel.
	// TODO: Windows XP (and probably 2000 and Vista) support alpha transparency directly.
	// Older versions don't.
	if (!has_alpha)
	{
		// No alpha channel specified.
		// Use filler instead.
		png_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
	}
	
	// Update the PNG info.
	png_read_update_info(png_ptr, info_ptr);
	
	// Create the bitmap.
	// Plugin icon bitmap.
	HDC dc = CreateCompatibleDC(NULL);
	BITMAPINFO bmInfo;
	
	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	bmInfo.bmiHeader.biWidth = 32;
	bmInfo.bmiHeader.biHeight = -32;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = 32;
	bmInfo.bmiHeader.biCompression = 0;
	bmInfo.bmiHeader.biSizeImage = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biClrImportant = 0;
	
	// TODO: Destroy hbmpPluginIcon later (if the ImageList doesn't do that automatically).
	void *bmpPluginIconData;
	HBITMAP hbmpPluginIcon = CreateDIBSection(dc, &bmInfo, DIB_RGB_COLORS,
						  &bmpPluginIconData, NULL, 0);
	
	if (!hbmpPluginIcon)
	{
		// Could not allocate a bitmap.
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Create the row pointers.
	png_bytep row_pointers[32];
	unsigned char *pixels = static_cast<unsigned char*>(bmpPluginIconData);
	for (unsigned int i = 0; i < 32; i++)
	{
		row_pointers[i] = pixels;
		pixels += 32*4;
	}
	
	// Read the image data.
	png_read_image(png_ptr, row_pointers);
	
	// Close the PNG image.
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	
	// Return the bitmap.
	return hbmpPluginIcon;
}


/**
 * pmgr_window_get_bg_color() Get the background color.
 * @return Background color.
 */
static inline unsigned int pmgr_window_get_bg_color(void)
{
	// Get the background color.
	unsigned int bgColor = GetSysColor(COLOR_3DFACE);
	
	// Byteswap the lower 24 bits.
	bgColor = ((bgColor & 0xFF000000)) |
		  ((bgColor & 0x00FF0000) >> 16) |
		  ((bgColor & 0x0000FF00)) |
		  ((bgColor & 0x000000FF) << 16);
	
	return bgColor;
}
#endif /* GENS_PNG */
