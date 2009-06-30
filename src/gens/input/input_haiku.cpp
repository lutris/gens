/***************************************************************************
 * Gens: Input Handler - Haiku Backend.                                    *
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

#include "input_haiku.hpp"

// Message logging.
#include "macros/log_msg.h"

#include <unistd.h>

#include "emulator/g_main.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_mcd.hpp"
#include "emulator/options.hpp"
#include "util/file/save.hpp"
#include "util/sound/gym.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/gfx/imageutil.hpp"

#include "ui/gens_ui.hpp"
#include "gens/gens_window.h"

//#ifdef GENS_DEBUGGER
//#include "debugger/debugger.hpp"
//#endif

//#ifdef GENS_CDROM
//#include "segacd/cd_aspi.hpp"
//#endif

#include "gens/gens_window_sync.hpp"
#include "gens_ui.hpp"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "audio/audio.h"
#include "input/input.h"

// C++ includes.
#include <list>
using std::list;

// Haiku Key Constants
#include <InterfaceKit.h>

// Needed to handle controller input configuration.
#include "controller_config/cc_window.h"

extern "C" {

// Haiku-specific handler function definitions
static int input_haiku_init(void)
{
	return 0;
}

static int input_haiku_end(void)
{
	return 0;
}

static int input_haiku_update(void)
{
	return 0;
}

static BOOL input_haiku_check_key_pressed(uint16_t key)
{
	return true;
}

static unsigned int input_haiku_get_key(void)
{
	return (unsigned int)0;
}

static BOOL input_haiku_joy_exists(int joy_num)
{
	// We don't have any joystick support in Haiku just yet AFAIK (2009-06-29)
	return false;
}

// Input Backend struct of pointers to the
// above Haiku-specific callback functions
const input_backend_t input_backend_haiku =
{
	.init = input_haiku_init,
	.end = input_haiku_end,
	
	.keymap_default = &input_haiku_keymap_default[0],
	
	.update = input_haiku_update,
	.check_key_pressed = input_haiku_check_key_pressed,
	.get_key = input_haiku_get_key,
	.joy_exists = input_haiku_joy_exists,
	.get_key_name = input_haiku_get_key_name
};

// Haiku key event handler implementation
void input_haiku_event_key_down(int key)
{
}

void input_haiku_event_key_up(int key)
{
}

} // extern "C"

