/***************************************************************************
 * Gens: (Win32) General Options Window - Callback Functions.              *
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

#include "general_options_window.hpp"
#include "general_options_window_callbacks.h"
#include "general_options_window_misc.hpp"

#include "emulator/gens.hpp"
#include "gens_ui.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Contains useful macros.
#include <windowsx.h>


static void drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem);
static void selectRadioButton(int identifier);


LRESULT CALLBACK General_Options_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned short i, j;
	
	switch (message)
	{
		case WM_CREATE:
			General_Options_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(general_options_window);
			return 0;
		
		case WM_MENUSELECT:
		case WM_ENTERSIZEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			// Prevent audio stuttering when one of the following events occurs:
			// - Menu is opened.
			// - Window is resized.
			// - Left/Right mouse button down on title bar.
			Win32_ClearSoundBuffer();
			break;
		
		case WM_DRAWITEM:
			drawMsgRadioButton(LOWORD(wParam), (LPDRAWITEMSTRUCT)lParam);
			break;
		
		case WM_COMMAND:
			// Button press
			switch (LOWORD(wParam))
			{
				case IDOK: // Standard dialog button ID
				case IDC_BTN_OK:
				case IDC_BTN_SAVE:
					General_Options_Save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					General_Options_Save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
				
				default:
					if ((LOWORD(wParam) & 0xFF00) == 0xA000)
					{
						// Radio button selected.
						selectRadioButton(LOWORD(wParam));
					}
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != general_options_window)
				break;
			
			// Delete message color brushes and pens.
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 2; j++)
				{
					DeleteBrush(go_MsgColors_brushes[i][j]);
					DeletePen(go_MsgColors_pens[i][j]);
				}
			}
			
			// Delete intro effect color brushes and pens.
			for (i = 0; i < 8; i++)
			{
				for (j = 0; j < 2; j++)
				{
					DeleteBrush(go_IntroEffectColors_brushes[i][j]);
					DeletePen(go_IntroEffectColors_pens[i][j]);
				}
			}
			
			general_options_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


static void drawMsgRadioButton(int identifier, LPDRAWITEMSTRUCT lpDrawItem)
{
	unsigned short index;
	unsigned short button;
	
	if ((identifier & 0xF0) == 0x00)
	{
		index = (identifier & 0x0F) / 4;
		button = (identifier & 0x0F) % 4;
	}
	else if ((identifier & 0xF0) == 0x10)
	{
		index = 2;
		button = (identifier & 0x0F);
	}
	else
	{
		// Unknown button set.
		return;
	}
	
	HDC hDC = lpDrawItem->hDC;
	RECT itemRect = lpDrawItem->rcItem;
	
	if (index < 2)
		FillRect(hDC, &itemRect, go_MsgColors_brushes[button][0]);
	else //if (index == 2)
		FillRect(hDC, &itemRect, go_IntroEffectColors_brushes[button][0]);
	
	// Check if the radio button is checked.
	if (go_stcColor_State[index] == button)
	{
		// Checked. Draw a circle in the middle of the box.
		
		if (index < 2)
		{
			SelectObject(hDC, go_MsgColors_brushes[button][1]);
			SelectObject(hDC, go_MsgColors_pens[button][1]);
		}
		else //if (index == 2)
		{
			SelectObject(hDC, go_IntroEffectColors_brushes[button][1]);
			SelectObject(hDC, go_IntroEffectColors_pens[button][1]);
		}
		
		Ellipse(hDC, itemRect.left + 4, itemRect.top + 4,
			     itemRect.right - 4, itemRect.bottom - 4);
	}
}


static void selectRadioButton(int identifier)
{
	unsigned short index;
	unsigned short button;
	
	if ((identifier & 0xF0) == 0x00)
	{
		index = (identifier & 0x0F) / 4;
		button = (identifier & 0x0F) % 4;
	}
	else if ((identifier & 0xF0) == 0x10)
	{
		index = 2;
		button = (identifier & 0x0F);
	}
	else
	{
		// Unknown button set.
		return;
	}
	
	// Make sure the index and button IDs are valid.
	if (index < 2 && button >= 4)
		return;
	else if (index == 2 && button >= 8)
		return;
	else if (index > 2)
		return;
	
	if (go_stcColor_State[index] == button)
	{
		// State hasn't changed. Don't do anything.
		return;
	}
	
	// State has changed.
	unsigned short oldButton = go_stcColor_State[index];
	go_stcColor_State[index] = button;
	if (index < 2)
	{
		InvalidateRect(go_stcMsgColor[index][oldButton], NULL, FALSE);
		InvalidateRect(go_stcMsgColor[index][button], NULL, FALSE);
	}
	else //if (index == 2)
	{
		InvalidateRect(go_stcIntroEffectColor[oldButton], NULL, FALSE);
		InvalidateRect(go_stcIntroEffectColor[button], NULL, FALSE);
	}
}
