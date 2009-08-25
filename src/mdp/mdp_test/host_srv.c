/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * host_srv.c: MDP Host Services struct.                                   *
 *                                                                         *
 * Copyright (c) 2008-2009 by David Korth.                                 *
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

#include "host_srv.h"
#include "main.h"

#include "render.hpp"

// MDP includes.
#include "mdp/mdp_error.h"

#define MDP_HOST_FN(name) mdp_host_##name
#define MDP_HOST_STUB_FN(name) mdp_host_stub_##name
#define MDP_HOST_STUB_DECL(name) \
static int mdp_host_stub_##name(void) \
{ \
	/* Stub function. */ \
	TEST_WARN("host_srv->" #name "() is not implemented."); \
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED; \
}


MDP_HOST_STUB_DECL(val_set);
MDP_HOST_STUB_DECL(val_get);
	
MDP_HOST_STUB_DECL(osd_printf);
	
MDP_HOST_STUB_DECL(mem_read_8);
MDP_HOST_STUB_DECL(mem_read_16);
MDP_HOST_STUB_DECL(mem_read_32);
	
MDP_HOST_STUB_DECL(mem_write_8);
MDP_HOST_STUB_DECL(mem_write_16);
MDP_HOST_STUB_DECL(mem_write_32);
	
MDP_HOST_STUB_DECL(mem_read_block_8);
MDP_HOST_STUB_DECL(mem_read_block_16);
MDP_HOST_STUB_DECL(mem_read_block_32);
	
MDP_HOST_STUB_DECL(mem_write_block_8);
MDP_HOST_STUB_DECL(mem_write_block_16);
MDP_HOST_STUB_DECL(mem_write_block_32);
	
MDP_HOST_STUB_DECL(mem_size_get);
MDP_HOST_STUB_DECL(mem_size_set);
	
MDP_HOST_STUB_DECL(reg_get);
MDP_HOST_STUB_DECL(reg_set);
MDP_HOST_STUB_DECL(reg_get_all);
MDP_HOST_STUB_DECL(reg_set_all);
	
MDP_HOST_STUB_DECL(menu_item_add);
MDP_HOST_STUB_DECL(menu_item_remove);
	
MDP_HOST_STUB_DECL(menu_item_set_text);
MDP_HOST_STUB_DECL(menu_item_get_text);
	
MDP_HOST_STUB_DECL(menu_item_set_checked);
MDP_HOST_STUB_DECL(menu_item_get_checked);
	
MDP_HOST_STUB_DECL(event_register);
MDP_HOST_STUB_DECL(event_unregister);
	
MDP_HOST_STUB_DECL(window_register);
MDP_HOST_STUB_DECL(window_unregister);
MDP_HOST_STUB_DECL(window_get_main);
	
MDP_HOST_STUB_DECL(emulator_control);
	
MDP_HOST_STUB_DECL(config_set);
MDP_HOST_STUB_DECL(config_get);
	
MDP_HOST_STUB_DECL(dir_get_default_save_path);
MDP_HOST_STUB_DECL(dir_register);
MDP_HOST_STUB_DECL(dir_unregister);
	
MDP_HOST_STUB_DECL(crc32);
MDP_HOST_STUB_DECL(z_open);
MDP_HOST_STUB_DECL(z_get_file);
MDP_HOST_STUB_DECL(z_close)


mdp_host_t host_srv =
{
	.interfaceVersion	= TEST_MDP_INTERFACE_VERSION,
	
	.val_set		= MDP_HOST_STUB_FN(val_set),
	.val_get		= MDP_HOST_STUB_FN(val_get),
	
	.osd_printf		= MDP_HOST_STUB_FN(osd_printf),
	
	.renderer_register	= MDP_HOST_FN(renderer_register),
	.renderer_unregister	= MDP_HOST_FN(renderer_unregister),
	
	.mem_read_8		= MDP_HOST_STUB_FN(mem_read_8),
	.mem_read_16		= MDP_HOST_STUB_FN(mem_read_16),
	.mem_read_32		= MDP_HOST_STUB_FN(mem_read_32),
	
	.mem_write_8		= MDP_HOST_STUB_FN(mem_write_8),
	.mem_write_16		= MDP_HOST_STUB_FN(mem_write_16),
	.mem_write_32		= MDP_HOST_STUB_FN(mem_write_32),
	
	.mem_read_block_8	= MDP_HOST_STUB_FN(mem_read_block_8),
	.mem_read_block_16	= MDP_HOST_STUB_FN(mem_read_block_16),
	.mem_read_block_32	= MDP_HOST_STUB_FN(mem_read_block_32),
	
	.mem_write_block_8	= MDP_HOST_STUB_FN(mem_write_block_8),
	.mem_write_block_16	= MDP_HOST_STUB_FN(mem_write_block_16),
	.mem_write_block_32	= MDP_HOST_STUB_FN(mem_write_block_32),
	
	.mem_size_get		= MDP_HOST_STUB_FN(mem_size_get),
	.mem_size_set		= MDP_HOST_STUB_FN(mem_size_set),
	
	.reg_get		= MDP_HOST_STUB_FN(reg_get),
	.reg_set		= MDP_HOST_STUB_FN(reg_set),
	.reg_get_all		= MDP_HOST_STUB_FN(reg_get_all),
	.reg_set_all		= MDP_HOST_STUB_FN(reg_set_all),
	
	.menu_item_add		= MDP_HOST_STUB_FN(menu_item_add),
	.menu_item_remove	= MDP_HOST_STUB_FN(menu_item_remove),
	
	.menu_item_set_text	= MDP_HOST_STUB_FN(menu_item_set_text),
	.menu_item_get_text	= MDP_HOST_STUB_FN(menu_item_get_text),
	
	.menu_item_set_checked	= MDP_HOST_STUB_FN(menu_item_set_checked),
	.menu_item_get_checked	= MDP_HOST_STUB_FN(menu_item_get_checked),
	
	.event_register		= MDP_HOST_STUB_FN(event_register),
	.event_unregister	= MDP_HOST_STUB_FN(event_unregister),
	
	.window_register	= MDP_HOST_STUB_FN(window_register),
	.window_unregister	= MDP_HOST_STUB_FN(window_unregister),
	.window_get_main	= MDP_HOST_STUB_FN(window_get_main),
	
	.emulator_control	= MDP_HOST_STUB_FN(emulator_control),
	
	.config_set		= MDP_HOST_STUB_FN(config_set),
	.config_get		= MDP_HOST_STUB_FN(config_get),
	
	.dir_get_default_save_path = MDP_HOST_STUB_FN(dir_get_default_save_path),
	.dir_register		= MDP_HOST_STUB_FN(dir_register),
	.dir_unregister		= MDP_HOST_STUB_FN(dir_unregister),
	
	.crc32			= MDP_HOST_STUB_FN(crc32),
	.z_open			= MDP_HOST_STUB_FN(z_open),
	.z_get_file		= MDP_HOST_STUB_FN(z_get_file),
	.z_close		= MDP_HOST_STUB_FN(z_close)
};
