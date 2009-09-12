/***************************************************************************
 * Gens: (Haiku) Main Window.                                              *
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

#include "gens_window.h"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include "emulator/g_main.hpp"

// Menus.
#include "gens_menu.hpp"
#include "ui/common/gens/gens_menu.h"

// Message logging.
#include "macros/log_msg.h"

/**
 * gens_window_create(): Create the Gens window.
 */
void gens_window_create(void)
{
}

#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <View.h>

GensWindow::GensWindow() : BWindow(BRect(10, 10, 400, 300), "Gens", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE) {
	BMenuBar *menuBar = new BMenuBar(Bounds(), "menu");
	AddChild(menuBar);

	BMenuItem* item;

	// File Menu
	BMenu *menu = new BMenu("File");
	menu->AddItem(item = new BMenuItem("Open ROM", NULL));
	item->SetShortcut('O', B_COMMAND_KEY);
	menu->AddItem(item = new BMenuItem("Boot CD", NULL));
	item->SetShortcut('B', B_COMMAND_KEY);
	menu->AddItem(new BMenuItem("Netplay", NULL));
	BMenu *subMenu = new BMenu("ROM History");
	subMenu->AddItem(new BMenuItem("DUMMY HISTORY LINE", NULL));
	menu->AddItem(subMenu);
	menu->AddItem(new BMenuItem("Close ROM", NULL));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Game Genie", NULL));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Load State...", NULL));
	menu->AddItem(new BMenuItem("Save State as...", NULL));
	menu->AddItem(new BMenuItem("Quick Load", NULL));
	menu->AddItem(new BMenuItem("Quick Save", NULL));
	subMenu = new BMenu("Change State");
	subMenu->AddItem(new BMenuItem("DUMMY SAVESTATE LINE", NULL));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED)));
	item->SetShortcut('Q', B_COMMAND_KEY);
	menu->SetTargetForItems(this);
	menuBar->AddItem(menu);

	// Graphics Menu
	menu = new BMenu("Graphics");
	menu->AddItem(new BMenuItem("Full Screen", NULL));
	menu->AddItem(item = new BMenuItem("VSync", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddItem(item = new BMenuItem("Stretch", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("OpenGL", NULL));
	if (true)
		item->SetMarked(true);
	subMenu = new BMenu("OpenGL Resolution");
	subMenu->AddItem(new BMenuItem("320x240", NULL));
	subMenu->AddItem(new BMenuItem("640x480", NULL));
	subMenu->AddItem(new BMenuItem("800x600", NULL));
	subMenu->AddItem(new BMenuItem("1024x768", NULL));
	subMenu->AddItem(new BMenuItem("Custom", NULL));
	menu->AddItem(subMenu);
	subMenu = new BMenu("Bits per pixel");
	subMenu->AddItem(new BMenuItem("16", NULL));
	subMenu->AddItem(new BMenuItem("24", NULL));
	subMenu->AddItem(new BMenuItem("32", NULL));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Color Adjust", NULL));
	subMenu = new BMenu("Render");
	subMenu->AddItem(new BMenuItem("Normal", NULL));
	subMenu->AddItem(new BMenuItem("Double", NULL));
	subMenu->AddItem(new BMenuItem("Interpolated", NULL));
	subMenu->AddItem(new BMenuItem("Scanline", NULL));
	subMenu->AddItem(new BMenuItem("50% Scanline", NULL));
	subMenu->AddItem(new BMenuItem("25% Scanline", NULL));
	subMenu->AddItem(new BMenuItem("Interpolated Scanline", NULL));
	subMenu->AddItem(new BMenuItem("Interpolated 50% Scanline", NULL));
	subMenu->AddItem(new BMenuItem("Interpolated 25% Scanline", NULL));
	subMenu->AddItem(new BMenuItem("2xSAI (Kreed)", NULL));
	subMenu->AddItem(new BMenuItem("Scale2x", NULL));
	subMenu->AddItem(new BMenuItem("Hq2x", NULL));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Sprite Limit", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	subMenu = new BMenu("Frame Skip");
	subMenu->AddItem(new BMenuItem("Auto", NULL));
	subMenu->AddItem(new BMenuItem("0", NULL));
	subMenu->AddItem(new BMenuItem("1", NULL));
	subMenu->AddItem(new BMenuItem("2", NULL));
	subMenu->AddItem(new BMenuItem("3", NULL));
	subMenu->AddItem(new BMenuItem("4", NULL));
	subMenu->AddItem(new BMenuItem("5", NULL));
	subMenu->AddItem(new BMenuItem("6", NULL));
	subMenu->AddItem(new BMenuItem("7", NULL));
	subMenu->AddItem(new BMenuItem("8", NULL));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Screen Shot", NULL));
	menuBar->AddItem(menu);
	menu = new BMenu("CPU");
	subMenu = new BMenu("Debug");
	subMenu->AddItem(item = new BMenuItem("Genesis - 68000", NULL));
	if (true)
		item->SetMarked(true);
	subMenu->AddItem(item = new BMenuItem("Genesis - Z80", NULL));
	if (true)
		item->SetMarked(true);
	subMenu->AddItem(item = new BMenuItem("Genesis - VDP", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	subMenu = new BMenu("Country");
	subMenu->AddItem(new BMenuItem("Autodetect", NULL));
	subMenu->AddItem(new BMenuItem("Japan (NTSC)", NULL));
	subMenu->AddItem(new BMenuItem("USA (NTSC)", NULL));
	subMenu->AddItem(new BMenuItem("Europe (PAL)", NULL));
	subMenu->AddItem(new BMenuItem("Japan (PAL)", NULL));
	subMenu->AddSeparatorItem();
	BMenu *subMenu2 = new BMenu("Auto detection order");
	subMenu->AddItem(subMenu2);
	subMenu2->AddItem(new BMenuItem("USA (NTSC)", NULL));
	subMenu2->AddItem(new BMenuItem("Japan (NTSC)", NULL));
	subMenu2->AddItem(new BMenuItem("Europe (PAL)", NULL));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Hard Reset", NULL));
	menu->AddItem(new BMenuItem("Reset 68000", NULL));
	menu->AddItem(new BMenuItem("Reset Z80", NULL));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Perfect Synchro (SLOW)", NULL));
	if (true)
		item->SetMarked(true);
	menuBar->AddItem(menu);
	menu = new BMenu("Sound");
	menu->AddItem(item = new BMenuItem("Enabled", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	subMenu = new BMenu("Rate");
	subMenu->AddItem(new BMenuItem("11025", NULL));
	subMenu->AddItem(new BMenuItem("16000", NULL));
	subMenu->AddItem(new BMenuItem("22050", NULL));
	subMenu->AddItem(new BMenuItem("32000", NULL));
	subMenu->AddItem(new BMenuItem("44100", NULL));
	subMenu->AddItem(new BMenuItem("48000", NULL));
	menu->AddItem(subMenu);
	menu->AddItem(item = new BMenuItem("Stereo", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Z80", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("YM2612", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddItem(item = new BMenuItem("YM2612 Improved", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("DAC", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddItem(item = new BMenuItem("DAC Improved", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("PSG", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddItem(item = new BMenuItem("PSG Improved", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("PCM", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddItem(item = new BMenuItem("PWM", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("CDDA", NULL));
	if (true)
		item->SetMarked(true);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Start WAV Dump", NULL));
	menu->AddItem(new BMenuItem("Start GYM Dump", NULL));
	menuBar->AddItem(menu);
	menu = new BMenu("Options");
	menu->AddItem(new BMenuItem("General Options", NULL));
	menu->AddItem(new BMenuItem("Joypads...", NULL));
	menu->AddItem(new BMenuItem("Directories...", NULL));
	menu->AddItem(new BMenuItem("BIOS/Misc Files...", NULL));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("SDL Sound Test", NULL));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Current CD Drive", NULL));
	subMenu = new BMenu("Sega CD SRAM Size");
	subMenu->AddItem(new BMenuItem("None", NULL));
	subMenu->AddItem(new BMenuItem("8 kb", NULL));
	subMenu->AddItem(new BMenuItem("16 kb", NULL));
	subMenu->AddItem(new BMenuItem("32 kb", NULL));
	subMenu->AddItem(new BMenuItem("64 kb", NULL));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Load config", NULL));
	menu->AddItem(new BMenuItem("Save config as", NULL));
	menuBar->AddItem(menu);
	menu = new BMenu("Help");
	menu->AddItem(new BMenuItem("About", NULL));
	menuBar->AddItem(menu);
}

GensWindow::~GensWindow() {}

