/***************************************************************************
 * Gens: Savestate handler.                                                *
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

// Turn this on to enable savestate debugging.
#define GENS_DEBUG_SAVESTATE
#include <assert.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.hpp"

#include "emulator/g_main.hpp"

// CPU
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "mdZ80/mdZ80.h"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_m68k_32x.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/io/io.h"
#include "util/file/rom.hpp"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// Audio
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// SegaCD
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"
#include "segacd/lc89510.h"
#include "segacd/cd_file.h"

// UI
#include "ui/gens_ui.hpp"

// Byteswapping macros.
#include "libgsft/gsft_byteswap.h"

// Video Drawing.
#include "video/vdraw.h"

// GSX savestate structs.
#include "gsx_struct.h"
#include "gsx_v6.h"
#include "gsx_v7.h"

// Needed for SetCurrentDirectory.
#ifdef GENS_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif /* GENS_OS_WIN32 */

#ifdef GENS_MP3
// Gens Rerecording
#include "segacd/cdda_mp3.h"
#endif /* GENS_MP3 */

extern char preloaded_tracks [100], played_tracks_linear [101]; // added for synchronous MP3 code

int Current_State = 0;
char State_Dir[GENS_PATH_MAX] = "";
char SRAM_Dir[GENS_PATH_MAX] = "";
char BRAM_Dir[GENS_PATH_MAX] = "";
unsigned char State_Buffer[MAX_STATE_FILE_LENGTH];

// C++ includes
using std::string;


/**
 * SelectFile(): Select a savestate to load or save.
 * @param save If false, load; if true, save.
 * @param dir ???
 * @return Filename if a savestate was selected, or "" if no file was selected.
 */
string Savestate::SelectFile(const bool save, const string& dir)
{
	string filename;
	
	if (save)
		filename = GensUI::saveFile("Save Savestate", "" /*Rom_Dir*/, SavestateFile);
	else
		filename = GensUI::openFile("Load Savestate", "" /*Rom_Dir*/, SavestateFile);
	
	return filename;
}


/**
 * SelectCDImage(): Select a CD image to load. (Used for multi-disc SegaCD games.)
 * @param dir ???
 * @return Filename if a CD image was selected, or "" if no file was selected.
 */
string Savestate::SelectCDImage(const string& dir)
{
	return GensUI::openFile("Load SegaCD Disc Image", "" /*Rom_Dir*/, CDImage);
}


/**
 * GetStateFile(): Open the current savestate file.
 * @return File handle for the current savestate file.
 */
FILE* Savestate::GetStateFile(void)
{
	return fopen(GetStateFilename().c_str(), "rb");
}


/**
 * GetStateFilename(): Get the filename of the current savestate.
 * @return Filename of the current savestate.
 */
string Savestate::GetStateFilename(void)
{
	return string(State_Dir) + string(ROM_Filename) + ".gs" + (char)('0' + Current_State);
}


/**
 * LoadState(): Load a savestate.
 * @param filename Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Savestate::LoadState(const string& filename)
{
	FILE *f;
	unsigned char *buf;
	int len;
	
	ice = 0;
	
	len = GENESIS_STATE_LENGTH;
	if (Genesis_Started);
	else if (SegaCD_Started)
		len += SEGACD_LENGTH_EX;
	else if (_32X_Started)
		len += G32X_LENGTH_EX;
	else
		return 0;
	
	buf = State_Buffer;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	if (!(f = fopen(filename.c_str(), "rb")))
		return 0;
	
	memset(buf, 0, len);
	if (fread(buf, 1, len, f))
	{
		// Verify that the savestate is in GSX format.
		static const uint8_t gsxHeader[5] = {'G', 'S', 'T', 0x40, 0xE0};
		if (memcmp(&buf[0], &gsxHeader[0], sizeof(gsxHeader)))
		{
			// Header does not match GSX.
			vdraw_text_printf(2000, "Error: State %d is not in GSX format.", Current_State);
			fclose(f);
			return 0;
		}
		
		//z80_Reset (&M_Z80); // Commented out in Gens Rerecording...
		/*
		main68k_reset();
		YM2612ResetChip(0);
		Reset_VDP();
		*/
		
		// Save functions updated from Gens Rerecording
		buf += GsxImportGenesis(buf);
		if (SegaCD_Started)
		{
			GsxImportSegaCD(buf);
			buf += SEGACD_LENGTH_EX;
		}
		if (_32X_Started)
		{
			GsxImport32X(buf);
			buf += G32X_LENGTH_EX;
		}
		
		// Make sure CRAM and VRAM are updated.
		Flag_Clr_Scr = 1;
		CRam_Flag = 1;
		VRam_Flag = 1;
		
		vdraw_text_printf(2000, "STATE %d LOADED", Current_State);
	}
	
	fclose(f);
	
	return 1;
}


/**
 * SaveState(): Save a savestate.
 * @param filename Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Savestate::SaveState(const string& filename)
{
	FILE *f;
	unsigned char *buf;
	int len;
	
	ice = 0;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	buf = State_Buffer;
	if ((f = fopen(filename.c_str(), "wb")) == NULL)
		return 0;
	
	len = GENESIS_STATE_LENGTH;
	if (Genesis_Started);
	else if (SegaCD_Started)
		len += SEGACD_LENGTH_EX;
	else if (_32X_Started)
		len += G32X_LENGTH_EX;
	else
		return 0;
	
	if (buf == NULL)
		return 0;
	memset(buf, 0, len);
	
	GsxExportGenesis(buf);
	buf += GENESIS_STATE_LENGTH;
	if (SegaCD_Started)
	{
		GsxExportSegaCD(buf);
		buf += SEGACD_LENGTH_EX;
	}
	if (_32X_Started)
	{
		GsxExport32X(buf);
		buf += G32X_LENGTH_EX;
	}
	
	fwrite(State_Buffer, 1, len, f);
	fclose(f);
	
	vdraw_text_printf(2000, "STATE %d SAVED", Current_State);
	
	return 1;
}


// See doc/genecyst_save_file_format.txt for information
// on the Genecyst save file format.


// Version field is initialized in GsxImportGenesis(),
// but is also used in GsxImportSegaCD() and GsxImport32X().
// TODO: Move this to the Savestate class.
static unsigned char m_Version;


// ImportData and ExportData functions from Gens Rerecording

//Modif N. - added ImportData and ExportData because the import/export code was getting a little hairy without these
// The main advantage to using these, besides less lines of code, is that
// you can replace ImportData with ExportData, without changing anything else in the arguments,
// to go from import code to export code.

inline void Savestate::ImportData(void* into, const void* data,
				  const unsigned int offset,
				  unsigned int numBytes)
{
	unsigned char* dst = static_cast<unsigned char*>(into);
	const unsigned char* src = static_cast<const unsigned char*>(data) + offset;
	
	while (numBytes--)
		*dst++ = *src++;
}


inline void Savestate::ExportData(const void* from, void* data,
				  const unsigned int offset,
				  unsigned int numBytes)
{
	const unsigned char* src = static_cast<const unsigned char*>(from);
	unsigned char* dst = static_cast<unsigned char*>(data) + offset;
	
	while (numBytes--)
	{
#ifdef GENS_DEBUG_SAVESTATE
		assert((*dst == 0 || *dst == 0xFF) && "error: saved over the same byte twice");
#endif /* GENS_DEBUG_SAVESTATE */
		*dst++ = *src++;
	}
}


// versions that auto-increment the offset
inline void Savestate::ImportDataAuto(void* into, const void* data,
				      unsigned int& offset,
				      const unsigned int numBytes)
{
	ImportData(into, data, offset, numBytes);
	offset += numBytes;
}


inline void Savestate::ExportDataAuto(const void* from, void* data,
				      unsigned int& offset,
				      const unsigned int numBytes)
{
	ExportData(from, data, offset, numBytes);
	offset += numBytes;
}


/**
 * GsxImportGenesis(): Load Genesis data from a GSX savestate.
 * (Portions ported from Gens Rerecording.)
 * @param Data Savestate data.
 * @return Number of bytes read.
 */
int Savestate::GsxImportGenesis(const unsigned char* data)
{
	// Savestate V6 and V7 code from Gens Rerecording.
	
#ifdef GENS_DEBUG_SAVESTATE
	// Make sure the save structs are the correct size.
	assert(sizeof(gsx_struct_md_t) == GENESIS_LENGTH_EX1);
	assert(sizeof(gsx_struct_md_v6_t) == GENESIS_V6_LENGTH_EX2);
	assert(sizeof(gsx_struct_md_v7_t) == GENESIS_LENGTH_EX2);
#endif
	
	/*
	// Commented out in Gens Rerecording.
	VDP_Int = 0;
	DMAT_Length = 0;
	*/
	
	// Length of the savestate.
	int len = GENESIS_STATE_LENGTH;
	
	// Copy the first part of the data into a gsx_struct_md_t struct.
	gsx_struct_md_t md_save;
	memcpy(&md_save, &data[0], sizeof(md_save));
	
	// Get the savestate version.
	m_Version = md_save.version.version;
	
	// Savestates earlier than Version 6 are shitty.
	if (m_Version < 6)
		len -= 0x10000;
	
	// Copy the CRAM, VSRAM, and Z80 RAM.
	// [TODO: Is CRAM supposed to be 16-bit byteswapped?]
	memcpy(&CRam, &md_save.cram, sizeof(CRam));
	memcpy(&VSRam, &md_save.vsram, sizeof(VSRam));
	memcpy(&Ram_Z80, &md_save.z80_ram, sizeof(Ram_Z80));
	
	// 68000 RAM.
	memcpy(&Ram_68k, &md_save.mc68000_ram, sizeof(Ram_68k));
	be16_to_cpu_array(&Ram_68k, sizeof(Ram_68k));
	
	// VRAM.
	memcpy(&VRam, &md_save.vram, sizeof(VRam));
	be16_to_cpu_array(&VRam, sizeof(VRam));
	
	// YM2612 registers.
	YM2612_Restore(&md_save.ym2612[0]);
	
	// Special data based on the savestate version.
	uint32_t psg_state[8];
	if ((m_Version >= 2) && (m_Version < 4))
	{
		// TODO: Create a struct fr this.
		ImportData(&Ctrl, data, 0x30, 7 * 4);
		
		// 0x440: Z80 busreq
		// 0x444: Z80 reset
		Z80_State &= ~(Z80_STATE_BUSREQ | Z80_STATE_RESET);
		if (data[0x440] & 1)
			Z80_State |= Z80_STATE_BUSREQ;
		if (data[0x444] & 1)
			Z80_State |= Z80_STATE_RESET;
		
		// 0x448: Z80 bank.
		ImportData(&Bank_Z80, data, 0x448, 4);
		
		// 0x22488: PSG registers.
		// TODO: Import this using correct endianness.
		ImportData(&psg_state, data, 0x224B8, 8 * 4);
		PSG_Restore_State(psg_state);
	}
	else if ((m_Version >= 4) || (m_Version == 0))
	{
		// New savestate version compatible with Kega.
		Z80_State &= ~(Z80_STATE_BUSREQ | Z80_STATE_RESET);
		
		if (m_Version == 4)
		{
			// Version 4 stores IFF and State differently.
			M_Z80.IM = data[0x437];
			M_Z80.IFF.b.IFF1 = (data[0x438] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (data[0x438] & 1) << 2;
			
			if (data[0x439] & 1)
				Z80_State |= Z80_STATE_BUSREQ;
		}
		else
		{
			// Other versions use the Gens v5 format.
			M_Z80.IM = 1;
			M_Z80.IFF.b.IFF1 = (md_save.z80_reg.IFF1 & 1) << 2;
			M_Z80.IFF.b.IFF2 = M_Z80.IFF.b.IFF1;
			
			if (!md_save.z80_reg.state_busreq)
				Z80_State |= Z80_STATE_BUSREQ;
			if (!md_save.z80_reg.state_reset)
				Z80_State |= Z80_STATE_RESET;
		}
		
		// Clear VDP control.
		memset(&Ctrl, 0x00, sizeof(Ctrl));
		
		// Load VDP control settings.
		uint32_t lastCtrlData = le32_to_cpu(md_save.vdp_ctrl.ctrl_data);
		Write_VDP_Ctrl(lastCtrlData >> 16);
		Write_VDP_Ctrl(lastCtrlData & 0xFFFF);
		
		Ctrl.Flag = md_save.vdp_ctrl.write_flag_2;
		Ctrl.DMA = (md_save.vdp_ctrl.dma_fill_flag & 1) << 2;
		Ctrl.Access = le16_to_cpu(md_save.vdp_ctrl.ctrl_access); //Nitsuja added this
		Ctrl.Address = le32_to_cpu(md_save.vdp_ctrl.write_address) & 0xFFFF;
		
		// Load the Z80 bank register.
		Bank_Z80 = le32_to_cpu(md_save.z80_reg.bank);
		
		if (m_Version >= 4)
		{
			// Load the PSG registers.
			for (int i = 0; i < 8; i++)
				psg_state[i] = md_save.psg[i];
			
			PSG_Restore_State(psg_state);
		}
	}
	
	// Z80 registers.
	mdZ80_set_AF(&M_Z80, le16_to_cpu(md_save.z80_reg.AF));
	M_Z80.AF.b.FXY = md_save.z80_reg.FXY; //Modif N [Gens Rerecording]
	M_Z80.BC.w.BC = le16_to_cpu(md_save.z80_reg.BC);
	M_Z80.DE.w.DE = le16_to_cpu(md_save.z80_reg.DE);
	M_Z80.HL.w.HL = le16_to_cpu(md_save.z80_reg.HL);
	M_Z80.IX.w.IX = le16_to_cpu(md_save.z80_reg.IX);
	M_Z80.IY.w.IY = le16_to_cpu(md_save.z80_reg.IY);
	mdZ80_set_PC(&M_Z80, le16_to_cpu(md_save.z80_reg.PC));
	M_Z80.SP.w.SP = le16_to_cpu(md_save.z80_reg.SP);
	mdZ80_set_AF2(&M_Z80, le16_to_cpu(md_save.z80_reg.AF2));
	M_Z80.BC2.w.BC2 = le16_to_cpu(md_save.z80_reg.BC2);
	M_Z80.DE2.w.DE2 = le16_to_cpu(md_save.z80_reg.DE2);
	M_Z80.HL2.w.HL2 = le16_to_cpu(md_save.z80_reg.HL2);
	M_Z80.I = md_save.z80_reg.I;
	
	// Gens Rerecording: This seems to only be used for movies (e.g. *.giz), so ignore it for now.
	//FrameCount = data[0x22478] + (data[0x22479] << 8) + (data[0x2247A] << 16) + (data[0x2247B] << 24);
	
	main68k_GetContext(&Context_68K);
	
	// VDP registers.
	for (int i = 0; i < 24; i++)
		Set_VDP_Reg(i, md_save.vdp_reg[i]);
	
	// MC68000 registers.
	for (int i = 0; i < 8; i++)
	{
		Context_68K.dreg[i] = le32_to_cpu(md_save.mc68000_reg.dreg[i]);
		Context_68K.areg[i] = le32_to_cpu(md_save.mc68000_reg.areg[i]);
	}
	Context_68K.pc = le32_to_cpu(md_save.mc68000_reg.pc);
	Context_68K.sr = (uint32_t)(le16_to_cpu(md_save.mc68000_reg.sr));
	
	// Stack pointer.
	if ((m_Version >= 3) || (m_Version == 0))
	{
		if (Context_68K.sr & 0x2000)
		{
			// Supervisor.
			Context_68K.asp = le32_to_cpu(md_save.mc68000_reg.usp);
		}
		else
		{
			// User.
			Context_68K.asp = le32_to_cpu(md_save.mc68000_reg.ssp);
		}
	}
	
	// NEW AND IMPROVED! Gens v6 and v7 savestate formats are here!
	// Ported from Gens Rerecording.
	unsigned int offset = GENESIS_LENGTH_EX1;
	if (m_Version == 6)
	{
		// Gens v6 savestate extensions.
		gsx_struct_md_v6_t md_save_v6;
		memcpy(&md_save_v6, &data[GENESIS_LENGTH_EX1], sizeof(md_save_v6));
		
		//Modif N. - saving more stuff (although a couple of these are saved above in a weird way that I don't trust)
		
		for (unsigned int i = 0; i < 8; i++)
		{
			Context_68K.dreg[i] = le32_to_cpu(md_save_v6.mc68000_reg.dreg[i]);
			Context_68K.areg[i] = le32_to_cpu(md_save_v6.mc68000_reg.areg[i]);
			Context_68K.interrupts[i] = md_save_v6.mc68000_reg.interrupts[i];
		}
		Context_68K.asp		= le32_to_cpu(md_save_v6.mc68000_reg.asp);
		Context_68K.pc		= le32_to_cpu(md_save_v6.mc68000_reg.pc);
		Context_68K.odometer	= le32_to_cpu(md_save_v6.mc68000_reg.odometer);
		Context_68K.sr		= le16_to_cpu(md_save_v6.mc68000_reg.sr);
		Context_68K.contextfiller00 = le16_to_cpu(md_save_v6.mc68000_reg.contextfiller00);
		
		VDP_Reg.H_Int		= le32_to_cpu(md_save_v6.vdp_reg.DUPE_H_Int);
		VDP_Reg.Set1		= le32_to_cpu(md_save_v6.vdp_reg.Set1);
		VDP_Reg.Set2		= le32_to_cpu(md_save_v6.vdp_reg.Set2);
		VDP_Reg.Pat_ScrA_Adr	= le32_to_cpu(md_save_v6.vdp_reg.DUPE_Pat_ScrA_Adr);
		VDP_Reg.Pat_ScrA_Adr	= le32_to_cpu(md_save_v6.vdp_reg.Pat_ScrA_Adr);
		VDP_Reg.Pat_Win_Adr	= le32_to_cpu(md_save_v6.vdp_reg.Pat_Win_Adr);
		VDP_Reg.Pat_ScrB_Adr	= le32_to_cpu(md_save_v6.vdp_reg.Pat_ScrB_Adr);
		VDP_Reg.Spr_Att_Adr	= le32_to_cpu(md_save_v6.vdp_reg.Spr_Att_Adr);
		VDP_Reg.Reg6		= le32_to_cpu(md_save_v6.vdp_reg.Reg6);
		VDP_Reg.BG_Color	= le32_to_cpu(md_save_v6.vdp_reg.BG_Color);
		VDP_Reg.Reg8		= le32_to_cpu(md_save_v6.vdp_reg.Reg8);
		VDP_Reg.Reg9		= le32_to_cpu(md_save_v6.vdp_reg.Reg9);
		VDP_Reg.H_Int		= le32_to_cpu(md_save_v6.vdp_reg.H_Int);
		VDP_Reg.Set3		= le32_to_cpu(md_save_v6.vdp_reg.Set3);
		VDP_Reg.Set4		= le32_to_cpu(md_save_v6.vdp_reg.Set4);
		VDP_Reg.H_Scr_Adr	= le32_to_cpu(md_save_v6.vdp_reg.H_Scr_Adr);
		VDP_Reg.Reg14		= le32_to_cpu(md_save_v6.vdp_reg.Reg14);
		VDP_Reg.Auto_Inc	= le32_to_cpu(md_save_v6.vdp_reg.Auto_Inc);
		VDP_Reg.Scr_Size	= le32_to_cpu(md_save_v6.vdp_reg.Scr_Size);
		VDP_Reg.Win_H_Pos	= le32_to_cpu(md_save_v6.vdp_reg.Win_H_Pos);
		VDP_Reg.Win_V_Pos	= le32_to_cpu(md_save_v6.vdp_reg.Win_V_Pos);
		VDP_Reg.DMA_Length_L	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Length_L);
		VDP_Reg.DMA_Length_H	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Length_H);
		VDP_Reg.DMA_Src_Adr_L	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Src_Adr_L);
		VDP_Reg.DMA_Src_Adr_M	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Src_Adr_M);
		VDP_Reg.DMA_Src_Adr_H	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Src_Adr_H);
		VDP_Reg.DMA_Length	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Length);
		VDP_Reg.DMA_Address	= le32_to_cpu(md_save_v6.vdp_reg.DMA_Address);
		
		// Control Port 1. (Port Information)
		Controller_1_Counter	= le32_to_cpu(md_save_v6.control_port1.counter);
		Controller_1_Delay	= le32_to_cpu(md_save_v6.control_port1.delay);
		Controller_1_State	= le32_to_cpu(md_save_v6.control_port1.state);
		Controller_1_COM	= le32_to_cpu(md_save_v6.control_port1.COM);
		
		// Control Port 2. (Port Information)
		Controller_2_State	= le32_to_cpu(md_save_v6.control_port2.state);
		Controller_2_COM	= le32_to_cpu(md_save_v6.control_port2.COM);
		Controller_2_Counter	= le32_to_cpu(md_save_v6.control_port2.counter);
		Controller_2_Delay	= le32_to_cpu(md_save_v6.control_port2.delay);
		
		Memory_Control_Status	= le32_to_cpu(md_save_v6.memory_control_status);
		// TODO: Cell_Conv_Tab can't be saved like this...
		//ImportDataAuto(&Cell_Conv_Tab, data, offset, 4);
		//Cell_Conv_Tab		= le32_to_cpu(md_save_v6.cell_conv_tab);
		
		// TODO: Add a configuration option to enable/disable loading controller types.
		#define GSX_V6_LOAD_CONTROLLER_STATUS(gsx_save, player)			\
		{									\
			/* Controller_ ## player ## _Type = le32_to_cpu(gsx_save.type); */		\
			Controller_ ## player ## _Buttons =						\
				((le32_to_cpu(gsx_save.up) & 0x01)    ? CONTROLLER_UP : 0)	|	\
				((le32_to_cpu(gsx_save.down) & 0x01)  ? CONTROLLER_DOWN : 0)	|	\
				((le32_to_cpu(gsx_save.left) & 0x01)  ? CONTROLLER_LEFT : 0)	|	\
				((le32_to_cpu(gsx_save.right) & 0x01) ? CONTROLLER_RIGHT : 0)	|	\
				((le32_to_cpu(gsx_save.start) & 0x01) ? CONTROLLER_START : 0)	|	\
				((le32_to_cpu(gsx_save.mode) & 0x01)  ? CONTROLLER_MODE : 0)	|	\
				((le32_to_cpu(gsx_save.A) & 0x01)     ? CONTROLLER_A : 0)	|	\
				((le32_to_cpu(gsx_save.B) & 0x01)     ? CONTROLLER_B : 0)	|	\
				((le32_to_cpu(gsx_save.C) & 0x01)     ? CONTROLLER_C : 0)	|	\
				((le32_to_cpu(gsx_save.X) & 0x01)     ? CONTROLLER_X : 0)	|	\
				((le32_to_cpu(gsx_save.Y) & 0x01)     ? CONTROLLER_Y : 0)	|	\
				((le32_to_cpu(gsx_save.Z) & 0x01)     ? CONTROLLER_Z : 0);		\
		}
		
		// Button/Type information for control ports 1 and 2.
		GSX_V6_LOAD_CONTROLLER_STATUS(md_save_v6.player1, 1);
		GSX_V6_LOAD_CONTROLLER_STATUS(md_save_v6.player2, 2);
		
		// MIscellaneous.
		DMAT_Length		= le32_to_cpu(md_save_v6.dmat_length);
		DMAT_Type		= le32_to_cpu(md_save_v6.dmat_type);
		DMAT_Tmp		= le32_to_cpu(md_save_v6.dmat_tmp);
		VDP_Current_Line	= le32_to_cpu(md_save_v6.vdp_current_line);
		VDP_Num_Vis_Lines	= le32_to_cpu(md_save_v6.DUPE_vdp_num_vis_lines);
		VDP_Num_Vis_Lines	= le32_to_cpu(md_save_v6.vdp_num_vis_lines);
		Bank_M68K		= le32_to_cpu(md_save_v6.bank_m68k);
		S68K_State		= le32_to_cpu(md_save_v6.s68k_state);
		Z80_State		= le32_to_cpu(md_save_v6.z80_state);
		Last_BUS_REQ_Cnt	= le32_to_cpu(md_save_v6.last_bus_req_cnt);
		Last_BUS_REQ_St		= le32_to_cpu(md_save_v6.last_bus_req_st);
		Fake_Fetch		= le32_to_cpu(md_save_v6.fake_fetch);
		Game_Mode		= le32_to_cpu(md_save_v6.game_mode);
		CPU_Mode		= le32_to_cpu(md_save_v6.cpu_mode);
		CPL_M68K		= le32_to_cpu(md_save_v6.cpl_m68k);
		CPL_S68K		= le32_to_cpu(md_save_v6.cpl_s68k);
		CPL_Z80			= le32_to_cpu(md_save_v6.cpl_z80);
		Cycles_S68K		= le32_to_cpu(md_save_v6.cycles_s68k);
		Cycles_M68K		= le32_to_cpu(md_save_v6.cycles_m68k);
		Cycles_Z80		= le32_to_cpu(md_save_v6.cycles_z80);
		VDP_Status		= le32_to_cpu(md_save_v6.vdp_status);
		VDP_Int			= le32_to_cpu(md_save_v6.vdp_int);
		Ctrl.Write		= le32_to_cpu(md_save_v6.vdp_ctrl_write);
		Ctrl.DMA_Mode		= le32_to_cpu(md_save_v6.vdp_ctrl_dma_mode);
		Ctrl.DMA		= le32_to_cpu(md_save_v6.vdp_ctrl_dma);
		//ImportDataAuto(&CRam_Flag, data, offset, 4); //Causes screen to blank
		//offset+=4;
		
		// TODO: LagCount from Gens Rerecording.
		//LadCount		= le32_to_cpu(md_save_v6.lag_count);
		
		VRam_Flag		= le32_to_cpu(md_save_v6.vram_flag);
		VDP_Reg.DMA_Length	= le32_to_cpu(md_save_v6.DUPE1_vdp_reg_dma_length);
		VDP_Reg.Auto_Inc	= le32_to_cpu(md_save_v6.vdp_reg_auto_inc);
		VDP_Reg.DMA_Length	= le32_to_cpu(md_save_v6.DUPE2_vdp_reg_dma_length);
		//ImportDataAuto(VRam, data, offset, sizeof(VRam));
		memcpy(&CRam, &md_save_v6.cram, sizeof(CRam));
		//ImportDataAuto(VSRam, data, offset, 64);
		memcpy(&H_Counter_Table, &md_save_v6.h_counter_table, sizeof(H_Counter_Table));
		//ImportDataAuto(Spr_Link, data, offset, 4*256);
		//extern int DMAT_Tmp, VSRam_Over;
		//ImportDataAuto(&DMAT_Tmp, data, offset, 4);
		//ImportDataAuto(&VSRam_Over, data, offset, 4);
		VDP_Reg.DMA_Length_L	= le32_to_cpu(md_save_v6.vdp_reg_dma_length_l);
		VDP_Reg.DMA_Length_H	= le32_to_cpu(md_save_v6.vdp_reg_dma_length_h);
		VDP_Reg.DMA_Src_Adr_L	= le32_to_cpu(md_save_v6.vdp_reg_dma_src_adr_l);
		VDP_Reg.DMA_Src_Adr_M	= le32_to_cpu(md_save_v6.vdp_reg_dma_src_adr_m);
		VDP_Reg.DMA_Src_Adr_H	= le32_to_cpu(md_save_v6.vdp_reg_dma_src_adr_h);
		VDP_Reg.DMA_Length	= le32_to_cpu(md_save_v6.vdp_reg_dma_length);
		VDP_Reg.DMA_Address	= le32_to_cpu(md_save_v6.vdp_reg_dma_address);
	}
	else if (m_Version >= 7)
	{
		// Gens v7 savestate extensions.
		gsx_struct_md_v7_t md_save_v7;
		memcpy(&md_save_v7, &data[GENESIS_LENGTH_EX1], sizeof(md_save_v7));
		
		// Load all YM2612 registers.
		// some Important parts of this weren't saved above
		YM2612_Restore_Full(&md_save_v7.ym2612);
		
		// Load all PSG registers.
		// some Important parts of this weren't saved above
		PSG_Restore_State_GSX_v7(&md_save_v7.psg);
		
		// Load all Z80 registers.
		M_Z80.AF.w.AF		= le16_to_cpu(md_save_v7.z80_reg.FA);
		M_Z80.AF.b.x		= md_save_v7.z80_reg.reserved_AF;
		M_Z80.AF.b.FXY		= md_save_v7.z80_reg.FXY;
		M_Z80.BC.w.BC		= le16_to_cpu(md_save_v7.z80_reg.BC);
		M_Z80.BC.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_BC);
		M_Z80.DE.w.DE		= le16_to_cpu(md_save_v7.z80_reg.DE);
		M_Z80.DE.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_DE);
		M_Z80.HL.w.HL		= le16_to_cpu(md_save_v7.z80_reg.HL);
		M_Z80.HL.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_HL);
		M_Z80.IX.w.IX		= le16_to_cpu(md_save_v7.z80_reg.IX);
		M_Z80.IX.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_IX);
		M_Z80.IY.w.IY		= le16_to_cpu(md_save_v7.z80_reg.IY);
		M_Z80.IY.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_IY);
		M_Z80.SP.w.SP		= le16_to_cpu(md_save_v7.z80_reg.SP);
		M_Z80.SP.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_SP);
		
		M_Z80.AF2.w.AF2		= le16_to_cpu(md_save_v7.z80_reg.FA2);
		M_Z80.AF2.b.x		= md_save_v7.z80_reg.reserved_AF2;
		M_Z80.AF2.b.FXY2	= md_save_v7.z80_reg.FXY2;
		M_Z80.BC2.w.BC2		= le16_to_cpu(md_save_v7.z80_reg.BC2);
		M_Z80.BC2.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_BC2);
		M_Z80.DE2.w.DE2		= le16_to_cpu(md_save_v7.z80_reg.DE2);
		M_Z80.DE2.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_DE2);
		M_Z80.HL2.w.HL2		= le16_to_cpu(md_save_v7.z80_reg.HL2);
		M_Z80.HL2.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_HL2);
		
		M_Z80.IFF.b.IFF1	= md_save_v7.z80_reg.IFF1;
		M_Z80.IFF.b.IFF2	= md_save_v7.z80_reg.IFF2;
		M_Z80.IFF.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_IFF);
		M_Z80.R.w.R		= le16_to_cpu(md_save_v7.z80_reg.R);
		M_Z80.R.w.x		= le16_to_cpu(md_save_v7.z80_reg.reserved_R);
		
		M_Z80.I			= md_save_v7.z80_reg.I;
		M_Z80.IM		= md_save_v7.z80_reg.IM;
		M_Z80.IntVect		= md_save_v7.z80_reg.IntVect;
		M_Z80.IntLine		= md_save_v7.z80_reg.IntLine;
		M_Z80.Status		= le32_to_cpu(md_save_v7.z80_reg.status);
		
		M_Z80.TmpSav0		= le32_to_cpu(md_save_v7.z80_reg.TmpSav0);
		M_Z80.TmpSav1		= le32_to_cpu(md_save_v7.z80_reg.TmpSav1);
		
		M_Z80.CycleCnt		= le32_to_cpu(md_save_v7.z80_reg.CycleCnt);
		M_Z80.CycleTD		= le32_to_cpu(md_save_v7.z80_reg.CycleTD);
		M_Z80.CycleIO		= le32_to_cpu(md_save_v7.z80_reg.CycleIO);
		M_Z80.CycleSup		= le32_to_cpu(md_save_v7.z80_reg.CycleSup);
		
		// [Gens Rerecording] not sure about the last two variables, might as well save them too
		M_Z80.RetIC		= le32_to_cpu(md_save_v7.z80_reg.RetIC);
		M_Z80.IntAckC		= le32_to_cpu(md_save_v7.z80_reg.IntAckC);
		
		// All MC68000 registers.
		// some Important parts of this weren't saved above
		for (unsigned int i = 0; i < 8; i++)
		{
			Context_68K.dreg[i] = le32_to_cpu(md_save_v7.mc68000_reg.dreg[i]);
			Context_68K.areg[i] = le32_to_cpu(md_save_v7.mc68000_reg.areg[i]);
			Context_68K.interrupts[i] = md_save_v7.mc68000_reg.interrupts[i];
		}
		Context_68K.asp		= cpu_to_le32(md_save_v7.mc68000_reg.asp);
		Context_68K.pc		= cpu_to_le32(md_save_v7.mc68000_reg.pc);
		Context_68K.odometer	= cpu_to_le32(md_save_v7.mc68000_reg.odometer);
		Context_68K.sr		= cpu_to_le32(md_save_v7.mc68000_reg.sr);
		
		// TODO: Add a configuration option to enable/disable loading controller types.
		// Controller status. (apparently necessary)
		#define GSX_V7_LOAD_CONTROLLER_STATUS(gsx_save, player)			\
		{									\
			/* Controller_ ## player ## _Type = le32_to_cpu(gsx_save.type); */		\
			Controller_ ## player ## _Buttons =						\
				((le32_to_cpu(gsx_save.up) & 0x01)    ? CONTROLLER_UP : 0)	|	\
				((le32_to_cpu(gsx_save.down) & 0x01)  ? CONTROLLER_DOWN : 0)	|	\
				((le32_to_cpu(gsx_save.left) & 0x01)  ? CONTROLLER_LEFT : 0)	|	\
				((le32_to_cpu(gsx_save.right) & 0x01) ? CONTROLLER_RIGHT : 0)	|	\
				((le32_to_cpu(gsx_save.start) & 0x01) ? CONTROLLER_START : 0)	|	\
				((le32_to_cpu(gsx_save.mode) & 0x01)  ? CONTROLLER_MODE : 0)	|	\
				((le32_to_cpu(gsx_save.A) & 0x01)     ? CONTROLLER_A : 0)	|	\
				((le32_to_cpu(gsx_save.B) & 0x01)     ? CONTROLLER_B : 0)	|	\
				((le32_to_cpu(gsx_save.C) & 0x01)     ? CONTROLLER_C : 0)	|	\
				((le32_to_cpu(gsx_save.X) & 0x01)     ? CONTROLLER_X : 0)	|	\
				((le32_to_cpu(gsx_save.Y) & 0x01)     ? CONTROLLER_Y : 0)	|	\
				((le32_to_cpu(gsx_save.Z) & 0x01)     ? CONTROLLER_Z : 0);		\
		}
		
		// Control Port 1.
		Controller_1_State	= le32_to_cpu(md_save_v7.controllers.port1.state);
		Controller_1_COM	= le32_to_cpu(md_save_v7.controllers.port1.COM);
		Controller_1_Counter	= le32_to_cpu(md_save_v7.controllers.port1.counter);
		Controller_1_Delay	= le32_to_cpu(md_save_v7.controllers.port1.delay);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player1, 1);
		
		// Control Port 2.
		Controller_2_State	= le32_to_cpu(md_save_v7.controllers.port2.state);
		Controller_2_COM	= le32_to_cpu(md_save_v7.controllers.port2.COM);
		Controller_2_Counter	= le32_to_cpu(md_save_v7.controllers.port2.counter);
		Controller_2_Delay	= le32_to_cpu(md_save_v7.controllers.port2.delay);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player2, 2);
		
		// Teamplayer controllers.
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player1B, 1B);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player1C, 1C);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player1D, 1D);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player2B, 2B);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player2C, 2C);
		GSX_V7_LOAD_CONTROLLER_STATUS(md_save_v7.controllers.player2D, 2D);
		
		// Miscellaneous. (apparently necessary)
		VDP_Status		= le32_to_cpu(md_save_v7.vdp_status);
		VDP_Int			= le32_to_cpu(md_save_v7.vdp_int);
		VDP_Current_Line	= le32_to_cpu(md_save_v7.vdp_current_line);
		VDP_Num_Lines		= le32_to_cpu(md_save_v7.vdp_num_lines);
		VDP_Num_Vis_Lines	= le32_to_cpu(md_save_v7.vdp_num_vis_lines);
		DMAT_Length		= le32_to_cpu(md_save_v7.dmat_length);
		DMAT_Type		= le32_to_cpu(md_save_v7.dmat_type);
		//ImportDataAuto(&CRam_Flag. data. &offset, 4); //emulator flag which causes Gens not to update its draw palette, but doesn't affect sync state
		
		// TODO: LagCount for Gens Rerecording.
		//Lag_Count		= le32_to_cpu(md_save_v7.lag_count);
		VRam_Flag		= le32_to_cpu(md_save_v7.vram_flag);
		
		// Color RAM. [TODO: Is this supposed to be 16-bit byteswapped?]
		memcpy(&CRam, &md_save_v7.cram, sizeof(CRam));
		
		// Save RAM. [TODO: Is this supposed to be 16-bit byteswapped?]
		// it's probably safer sync-wise to keep SRAM stuff in the savestate
		memcpy(&SRAM, &md_save_v7.sram.sram, sizeof(SRAM));
		SRAM_Start		= le32_to_cpu(md_save_v7.sram.sram_start);
		SRAM_End		= le32_to_cpu(md_save_v7.sram.sram_end);
		SRAM_ON			= le32_to_cpu(md_save_v7.sram.sram_on);
		SRAM_Write		= le32_to_cpu(md_save_v7.sram.sram_write);
		SRAM_Custom		= le32_to_cpu(md_save_v7.sram.sram_custom);
		
		// More miscellaneous.
		// (Gens Rerecording) this group I'm not sure about,
		// they don't seem to be necessary but I'm keeping them around just in case
		Bank_M68K		= le32_to_cpu(md_save_v7.bank_m68k);
		S68K_State		= le32_to_cpu(md_save_v7.s68k_state);
		Z80_State		= le32_to_cpu(md_save_v7.z80_state);
		Last_BUS_REQ_Cnt	= le32_to_cpu(md_save_v7.last_bus_req_cnt);
		Last_BUS_REQ_St		= le32_to_cpu(md_save_v7.last_bus_req_st);
		Fake_Fetch		= le32_to_cpu(md_save_v7.fake_fetch);
		Game_Mode		= le32_to_cpu(md_save_v7.game_mode);
		CPU_Mode		= le32_to_cpu(md_save_v7.cpu_mode);
		CPL_M68K		= le32_to_cpu(md_save_v7.cpl_m68k);
		CPL_S68K		= le32_to_cpu(md_save_v7.cpl_s68k);
		CPL_Z80			= le32_to_cpu(md_save_v7.cpl_z80);
		Cycles_S68K		= le32_to_cpu(md_save_v7.cycles_s68k);
		Cycles_M68K		= le32_to_cpu(md_save_v7.cycles_m68k);
		Cycles_Z80		= le32_to_cpu(md_save_v7.cycles_z80);
		Gen_Mode		= le32_to_cpu(md_save_v7.gen_mode);
		Gen_Version		= le32_to_cpu(md_save_v7.gen_version);
		
		// TODO: Is this supposed to be 16-bit bytewapped?
		memcpy(&H_Counter_Table, &md_save_v7.h_counter_table, sizeof(H_Counter_Table));
		
		// VDP registers.
		uint32_t *reg = (uint32_t*)&VDP_Reg;
		for (unsigned int i = 0; i < 26; i++)
		{
			*reg = le32_to_cpu(md_save_v7.vdp_reg[i]);
			reg++;
		}
		
		// VDP control.
		reg = (uint32_t*)&Ctrl;
		for (unsigned int i = 0; i < 7; i++)
		{
			*reg = le32_to_cpu(md_save_v7.vdp_ctrl[i]);
			reg++;
		}
		
		// Extra Starscream MC68000 information.
		Context_68K.cycles_needed		= le32_to_cpu(md_save_v7.starscream_extra.cycles_needed);
		Context_68K.cycles_leftover		= le32_to_cpu(md_save_v7.starscream_extra.cycles_leftover);
		Context_68K.fetch_region_start		= le32_to_cpu(md_save_v7.starscream_extra.fetch_region_start);
		Context_68K.fetch_region_end		= le32_to_cpu(md_save_v7.starscream_extra.fetch_region_end);
		Context_68K.xflag			= md_save_v7.starscream_extra.xflag;
		Context_68K.execinfo			= md_save_v7.starscream_extra.execinfo;
		Context_68K.trace_trickybit		= md_save_v7.starscream_extra.trace_trickybit;
		Context_68K.filler			= md_save_v7.starscream_extra.filler;
		Context_68K.io_cycle_counter		= le32_to_cpu(md_save_v7.starscream_extra.io_cycle_counter);
		Context_68K.io_fetchbase		= le32_to_cpu(md_save_v7.starscream_extra.io_fetchbase);
		Context_68K.io_fetchbased_pc		= le32_to_cpu(md_save_v7.starscream_extra.io_fetchbased_pc);
		Context_68K.access_address		= le32_to_cpu(md_save_v7.starscream_extra.access_address);
		Context_68K.save_01			= le32_to_cpu(md_save_v7.starscream_extra.save_01);
		Context_68K.save_02			= le32_to_cpu(md_save_v7.starscream_extra.save_02);
	}
	
	main68k_SetContext(&Context_68K);
	return len;
}


/**
 * GsxExportGenesis(): Save Genesis data to a savestate.
 * (Portions ported from Gens Rerecording.)
 * @param data Savestate data buffer.
 */
void Savestate::GsxExportGenesis(unsigned char* data)
{
	// This savestate function uses the Gens v7 savestate format, ported from Gens Rerecording.
	// Note about Gens v7 savestate format:
	// - Plain MD savestates will work in older versions of GENS.
	// - Sega CD savestates won't work in older versions, but then again, older versions didn't properly support it.
	// - 32X savestates will *not* work in older versions of GENS. :(
	
#ifdef GENS_DEBUG_SAVESTATE
	// Make sure the save structs are the correct size.
	assert(sizeof(gsx_struct_md_t) == GENESIS_LENGTH_EX1);
	assert(sizeof(gsx_struct_md_v7_t) == GENESIS_LENGTH_EX2);
#endif
	
	// from Gens Rerecording
	struct S68000CONTEXT Context_68K; // Modif N.: apparently no longer necessary but I'm leaving it here just to be safe: purposely shadows the global Context_68K variable with this local copy to avoid tampering with it while saving
	
	// Be sure to finish DMA before save
	// [from Gens Rerecording] commented out; this may cause the saving to change the current state
	/*
	while (DMAT_Length)
		Update_DMA();
	*/
	
	gsx_struct_md_t md_save;
	memset(&md_save, 0x00, sizeof(md_save));
	
	// Genecyst savestate header.
	static const unsigned char gsx_magic[5] = {'G', 'S', 'T', 0x40, 0xE0};
	memcpy(&md_save.header.magic[0], &gsx_magic[0], sizeof(gsx_magic));
	
	// Genecyst savestate version information.
	md_save.version.version = 7;	// Version
	md_save.version.emulator = 0;	// Emulator ID (0 == Gens)
	
	// Save the PSG state.
	uint32_t psg_state[8];
	PSG_Save_State(psg_state);
	
	// Copy the PSG state into the savestate buffer.
	for (unsigned int i = 0; i < 8; i++)
	{
		md_save.psg[i] = cpu_to_le16(psg_state[i]);
	}
	
#ifdef GENS_DEBUG_SAVESTATE
	int contextsize1 = main68k_GetContextSize();
	int contextsize2 = sizeof(Context_68K);
	assert(contextsize1 == contextsize2);
#endif
	
	main68k_GetContext(&Context_68K);
	
	// MC68000 registers.
	for (unsigned int i = 0; i < 8; i++)
	{
		md_save.mc68000_reg.dreg[i] = cpu_to_le32(Context_68K.dreg[i]);
		md_save.mc68000_reg.areg[i] = cpu_to_le32(Context_68K.areg[i]);
	}
	md_save.mc68000_reg.pc = cpu_to_le32(Context_68K.pc);
	md_save.mc68000_reg.sr = cpu_to_le16((uint16_t)(Context_68K.sr));
	
	// Stack pointer.
	if (Context_68K.sr & 0x2000)
	{
		// Supervisor.
		md_save.mc68000_reg.usp = cpu_to_le32(Context_68K.asp);
		md_save.mc68000_reg.ssp = cpu_to_le32(Context_68K.areg[7]);
	}
	else
	{
		// User.
		md_save.mc68000_reg.usp = cpu_to_le32(Context_68K.areg[7]);
		md_save.mc68000_reg.ssp = cpu_to_le32(Context_68K.asp);
	}
	
	// VDP control data.
	md_save.vdp_ctrl.ctrl_data = cpu_to_le32(Ctrl.Data);
	
	md_save.vdp_ctrl.write_flag_2 = (uint8_t)(Ctrl.Flag);
	md_save.vdp_ctrl.dma_fill_flag = (uint8_t)((Ctrl.DMA >> 2) & 1);
	
	// Ctrl.Access added by Gens Rerecording.
	md_save.vdp_ctrl.ctrl_access = cpu_to_le16((uint16_t)(Ctrl.Access));
	
	md_save.vdp_ctrl.write_address = cpu_to_le32(Ctrl.Address & 0xFFFF);
	
	// VDP registers.
	VDP_Reg.DMA_Length_L = VDP_Reg.DMA_Length & 0xFF;
	VDP_Reg.DMA_Length_H = (VDP_Reg.DMA_Length >> 8) & 0xFF;
	
	VDP_Reg.DMA_Src_Adr_L = VDP_Reg.DMA_Address & 0xFF;
	VDP_Reg.DMA_Src_Adr_M = (VDP_Reg.DMA_Address >> 8) & 0xFF;
	VDP_Reg.DMA_Src_Adr_H = (VDP_Reg.DMA_Address >> 16) & 0xFF;
	
	VDP_Reg.DMA_Src_Adr_H |= Ctrl.DMA_Mode & 0xC0;
	
	// Registers are currently stored as 32-bit unsigned int,
	// but only the lower byte is used.
	uint32_t *vdp_src = (uint32_t*)&(VDP_Reg.Set1);
	for (unsigned int i = 0; i < 24; i++)
	{
		md_save.vdp_reg[i] = (uint8_t)(*vdp_src);
		vdp_src++;
	}
	
	// CRAM and VSRAM.
	// [TODO: Is CRAM supposed to be 16-bit byteswapped?]
	memcpy(&md_save.cram, CRam, sizeof(md_save.cram));
	memcpy(&md_save.vsram, VSRam, sizeof(md_save.vsram));
	
	// YM2612 registers.
	YM2612_Save(&md_save.ym2612[0]);
	
	// Z80 registers.
	md_save.z80_reg.AF  = cpu_to_le16((uint16_t)(mdZ80_get_AF(&M_Z80)));
	md_save.z80_reg.FXY = M_Z80.AF.b.FXY; // Modif N [Gens Rerecording]
	md_save.z80_reg.BC  = cpu_to_le16((uint16_t)(M_Z80.BC.w.BC));
	md_save.z80_reg.DE  = cpu_to_le16((uint16_t)(M_Z80.DE.w.DE));
	md_save.z80_reg.HL  = cpu_to_le16((uint16_t)(M_Z80.HL.w.HL));
	md_save.z80_reg.IX  = cpu_to_le16((uint16_t)(M_Z80.IX.w.IX));
	md_save.z80_reg.IY  = cpu_to_le16((uint16_t)(M_Z80.IY.w.IY));
	md_save.z80_reg.PC  = cpu_to_le16((uint16_t)(mdZ80_get_PC(&M_Z80)));
	md_save.z80_reg.SP  = cpu_to_le16((uint16_t)(M_Z80.SP.w.SP));
	md_save.z80_reg.AF2 = cpu_to_le16((uint16_t)(mdZ80_get_AF2(&M_Z80)));
	md_save.z80_reg.BC2 = cpu_to_le16((uint16_t)(M_Z80.BC2.w.BC2));
	md_save.z80_reg.DE2 = cpu_to_le16((uint16_t)(M_Z80.DE2.w.DE2));
	md_save.z80_reg.HL2 = cpu_to_le16((uint16_t)(M_Z80.HL2.w.HL2));
	md_save.z80_reg.I = M_Z80.I;
	md_save.z80_reg.IFF1 = (M_Z80.IFF.b.IFF1 >> 2);
	md_save.z80_reg.state_busreq = (((Z80_State & Z80_STATE_BUSREQ) ? 1 : 0) ^ 1);
	md_save.z80_reg.state_reset  = (((Z80_State & Z80_STATE_RESET) ? 1 : 0) ^ 1);
	md_save.z80_reg.bank = cpu_to_le32(Bank_Z80);
	
	// Z80 RAM.
	memcpy(&md_save.z80_ram, &Ram_Z80, sizeof(md_save.z80_ram));
	
	// 68000 RAM.
	memcpy(&md_save.mc68000_ram, &Ram_68k, sizeof(md_save.mc68000_ram));
	cpu_to_be16_array(&md_save.mc68000_ram, sizeof(md_save.mc68000_ram));
	
	// VRAM.
	memcpy(&md_save.vram, &VRam, sizeof(md_save.vram));
	cpu_to_be16_array(&md_save.vram, sizeof(md_save.vram));
	
	// TODO: This is from Gens Rerecording, and is probably not any use right now.
	//md_save.frame_count = cpu_to_le32(FrameCount);	//Modif
	
	// Copy md_save to the data variable.
	memcpy(&data[0], &md_save, sizeof(md_save));
	
	// Gens v7 Savestate Additions
	// version 7 additions (version 6 additions deleted)
	//Modif N. - saving more stuff (added everything after this)
	
	gsx_struct_md_v7_t md_save_v7;
	memset(&md_save_v7, 0x00, sizeof(md_save_v7));
	
	// Save all YM2612 registers.
	YM2612_Save_Full(&md_save_v7.ym2612);
	
	// Save all PSG registers.
	PSG_Save_State_GSX_v7(&md_save_v7.psg);
	
	// Save all Z80 registers.
	md_save_v7.z80_reg.FA		= cpu_to_le16(M_Z80.AF.w.AF);
	md_save_v7.z80_reg.reserved_AF	= M_Z80.AF.b.x;
	md_save_v7.z80_reg.FXY		= M_Z80.AF.b.FXY;
	md_save_v7.z80_reg.BC		= cpu_to_le16(M_Z80.BC.w.BC);
	md_save_v7.z80_reg.reserved_BC	= cpu_to_le16(M_Z80.BC.w.x);
	md_save_v7.z80_reg.DE		= cpu_to_le16(M_Z80.DE.w.DE);
	md_save_v7.z80_reg.reserved_DE	= cpu_to_le16(M_Z80.DE.w.x);
	md_save_v7.z80_reg.HL		= cpu_to_le16(M_Z80.HL.w.HL);
	md_save_v7.z80_reg.reserved_HL	= cpu_to_le16(M_Z80.HL.w.x);
	md_save_v7.z80_reg.IX		= cpu_to_le16(M_Z80.IX.w.IX);
	md_save_v7.z80_reg.reserved_IX	= cpu_to_le16(M_Z80.IX.w.x);
	md_save_v7.z80_reg.IY		= cpu_to_le16(M_Z80.IY.w.IY);
	md_save_v7.z80_reg.reserved_IY	= cpu_to_le16(M_Z80.IY.w.x);
	
	// This was originally "PC", but mdZ80 stores the program counter
	// as an x86 pointer, not as a Z80 pointer. So, it is written as
	// 0xDEADBEEF (big-endian) in the Gens v7 savestate extension,
	// and the original Gens v5 Z80 PC is used instead.
	md_save_v7.z80_reg.DEADBEEF_1	= cpu_to_be32(0xDEADBEEF);
	
	md_save_v7.z80_reg.SP		= cpu_to_le16(M_Z80.SP.w.SP);
	md_save_v7.z80_reg.reserved_SP	= cpu_to_le16(M_Z80.SP.w.x);
	
	md_save_v7.z80_reg.FA2		= cpu_to_le16(M_Z80.AF2.w.AF2);
	md_save_v7.z80_reg.reserved_AF2	= M_Z80.AF2.b.x;
	md_save_v7.z80_reg.FXY2		= M_Z80.AF2.b.FXY2;
	md_save_v7.z80_reg.BC2		= cpu_to_le16(M_Z80.BC2.w.BC2);
	md_save_v7.z80_reg.reserved_BC2	= cpu_to_le16(M_Z80.BC2.w.x);
	md_save_v7.z80_reg.DE2		= cpu_to_le16(M_Z80.DE2.w.DE2);
	md_save_v7.z80_reg.reserved_DE2	= cpu_to_le16(M_Z80.DE2.w.x);
	md_save_v7.z80_reg.HL2		= cpu_to_le16(M_Z80.HL2.w.HL2);
	md_save_v7.z80_reg.reserved_HL2	= cpu_to_le16(M_Z80.HL2.w.x);
	
	md_save_v7.z80_reg.IFF1		= M_Z80.IFF.b.IFF1;
	md_save_v7.z80_reg.IFF2		= M_Z80.IFF.b.IFF2;
	md_save_v7.z80_reg.reserved_IFF	= cpu_to_le16(M_Z80.IFF.w.x);
	md_save_v7.z80_reg.R		= cpu_to_le16(M_Z80.R.w.R);
	md_save_v7.z80_reg.reserved_R	= cpu_to_le16(M_Z80.R.w.x);
	
	md_save_v7.z80_reg.I		= M_Z80.I;
	md_save_v7.z80_reg.IM		= M_Z80.IM;
	md_save_v7.z80_reg.IntVect	= M_Z80.IntVect;
	md_save_v7.z80_reg.IntLine	= M_Z80.IntLine;
	md_save_v7.z80_reg.status	= cpu_to_le32(M_Z80.Status);
	
	// This was originally "BasePC", but mdZ80 stores the program counter
	// as an x86 pointer, not as a Z80 pointer. So, it is written as
	// 0xDEADBEEF (big-endian) in the Gens v7 savestate extension,
	// and the original Gens v5 Z80 PC is used instead.
	md_save_v7.z80_reg.DEADBEEF_2	= cpu_to_be32(0xDEADBEEF);
	
	md_save_v7.z80_reg.TmpSav0	= cpu_to_le32(M_Z80.TmpSav0);
	md_save_v7.z80_reg.TmpSav1	= cpu_to_le32(M_Z80.TmpSav1);
	
	md_save_v7.z80_reg.CycleCnt	= cpu_to_le32(M_Z80.CycleCnt);
	md_save_v7.z80_reg.CycleTD	= cpu_to_le32(M_Z80.CycleTD);
	md_save_v7.z80_reg.CycleIO	= cpu_to_le32(M_Z80.CycleIO);
	md_save_v7.z80_reg.CycleSup	= cpu_to_le32(M_Z80.CycleSup);
	
	// [Gens Rerecording] not sure about the last two variables, might as well save them too
	md_save_v7.z80_reg.RetIC	= cpu_to_le32(M_Z80.RetIC);
	md_save_v7.z80_reg.IntAckC	= cpu_to_le32(M_Z80.IntAckC);
	
	// All MC68000 registers.
	for (unsigned int i = 0; i < 8; i++)
	{
		md_save_v7.mc68000_reg.dreg[i] = cpu_to_le32(Context_68K.dreg[i]);
		md_save_v7.mc68000_reg.areg[i] = cpu_to_le32(Context_68K.areg[i]);
		md_save_v7.mc68000_reg.interrupts[i] = Context_68K.interrupts[i];
	}
	md_save_v7.mc68000_reg.asp	= cpu_to_le32(Context_68K.asp);
	md_save_v7.mc68000_reg.pc	= cpu_to_le32(Context_68K.pc);
	md_save_v7.mc68000_reg.odometer	= cpu_to_le32(Context_68K.odometer);
	md_save_v7.mc68000_reg.sr	= cpu_to_le32(Context_68K.sr);
	
	// Controller status.
	#define GSX_V7_SAVE_CONTROLLER_STATUS(gsx_save, player)			\
	{									\
		gsx_save.type	= cpu_to_le32(Controller_ ## player ## _Type);	\
		gsx_save.up	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_UP) ? 1 : 0); \
		gsx_save.down	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_DOWN) ? 1 : 0); \
		gsx_save.left	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_LEFT) ? 1 : 0); \
		gsx_save.right	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_RIGHT) ? 1 : 0); \
		gsx_save.start	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_START) ? 1 : 0); \
		gsx_save.mode	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_MODE) ? 1 : 0); \
		gsx_save.A	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_A) ? 1 : 0); \
		gsx_save.B	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_B) ? 1 : 0); \
		gsx_save.C	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_C) ? 1 : 0); \
		gsx_save.X	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_X) ? 1 : 0); \
		gsx_save.Y	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_Y) ? 1 : 0); \
		gsx_save.Z	= cpu_to_le32((Controller_ ## player ## _Buttons & CONTROLLER_Z) ? 1 : 0); \
	}
	
	md_save_v7.controllers.port1.state	= cpu_to_le32(Controller_1_State);
	md_save_v7.controllers.port1.COM	= cpu_to_le32(Controller_1_COM);
	md_save_v7.controllers.port1.counter	= cpu_to_le32(Controller_1_Counter);
	md_save_v7.controllers.port1.delay	= cpu_to_le32(Controller_1_Delay);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player1, 1);
	
	md_save_v7.controllers.port2.state	= cpu_to_le32(Controller_2_State);
	md_save_v7.controllers.port2.COM	= cpu_to_le32(Controller_2_COM);
	md_save_v7.controllers.port2.counter	= cpu_to_le32(Controller_2_Counter);
	md_save_v7.controllers.port2.delay	= cpu_to_le32(Controller_2_Delay);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player2, 2);
	
	// Teamplayer controllers.
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player1B, 1B);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player1C, 1C);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player1D, 1D);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player2B, 2B);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player2C, 2C);
	GSX_V7_SAVE_CONTROLLER_STATUS(md_save_v7.controllers.player2D, 2D);
	
	// Miscellaneous.
	md_save_v7.vdp_status		= cpu_to_le32(VDP_Status);
	md_save_v7.vdp_int		= cpu_to_le32(VDP_Int);
	md_save_v7.vdp_current_line	= cpu_to_le32(VDP_Current_Line);
	md_save_v7.vdp_num_lines	= cpu_to_le32(VDP_Num_Lines);
	md_save_v7.vdp_num_vis_lines	= cpu_to_le32(VDP_Num_Vis_Lines);
	md_save_v7.dmat_length		= cpu_to_le32(DMAT_Length);
	md_save_v7.dmat_type		= cpu_to_le32(DMAT_Type);
	//ExportDataAuto(&CRam_Flag, data, offset, 4); [CRam Flag was not used at all in Gens Rerecording, not even for offsets.]
	// TODO: LagCount for Gens Rerecording.
	//md_save_v7.lag_count		= cpu_to_le32(LagCount);
	md_save_v7.vram_flag		= cpu_to_le32(VRam_Flag);
	
	// Color RAM. [TODO: Is this supposed to be 16-bit byteswapped?]
	memcpy(&md_save_v7.cram, &CRam, sizeof(CRam));
	
	// Save RAM. [TODO: Is this supposed to be 16-bit byteswapped?]
	// it's probably safer sync-wise to keep SRAM stuff in the savestate
	memcpy(&md_save_v7.sram.sram, &SRAM, sizeof(md_save_v7.sram.sram));
	md_save_v7.sram.sram_start	= cpu_to_le32(SRAM_Start);
	md_save_v7.sram.sram_end	= cpu_to_le32(SRAM_End);
	md_save_v7.sram.sram_on		= cpu_to_le32(SRAM_ON);
	md_save_v7.sram.sram_write	= cpu_to_le32(SRAM_Write);
	md_save_v7.sram.sram_custom	= cpu_to_le32(SRAM_Custom);
	
	// More miscellaneous.
	// (Gens Rerecording) this group I'm not sure about,
	// they don't seem to be necessary but I'm keeping them around just in case
	md_save_v7.bank_m68k		= cpu_to_le32(Bank_M68K);
	md_save_v7.s68k_state		= cpu_to_le32(S68K_State);
	md_save_v7.z80_state		= cpu_to_le32(Z80_State);
	md_save_v7.last_bus_req_cnt	= cpu_to_le32(Last_BUS_REQ_Cnt);
	md_save_v7.last_bus_req_st	= cpu_to_le32(Last_BUS_REQ_St);
	md_save_v7.fake_fetch		= cpu_to_le32(Fake_Fetch);
	md_save_v7.game_mode		= cpu_to_le32(Game_Mode);
	md_save_v7.cpu_mode		= cpu_to_le32(CPU_Mode);
	md_save_v7.cpl_m68k		= cpu_to_le32(CPL_M68K);
	md_save_v7.cpl_s68k		= cpu_to_le32(CPL_S68K);
	md_save_v7.cpl_z80		= cpu_to_le32(CPL_Z80);
	md_save_v7.cycles_s68k		= cpu_to_le32(Cycles_S68K);
	md_save_v7.cycles_m68k		= cpu_to_le32(Cycles_M68K);
	md_save_v7.cycles_z80		= cpu_to_le32(Cycles_Z80);
	md_save_v7.gen_mode		= cpu_to_le32(Gen_Mode);
	md_save_v7.gen_version		= cpu_to_le32(Gen_Version);
	
	// TODO: Is this supposed to be 16-bit byteswapped?
	memcpy(&md_save_v7.h_counter_table, &H_Counter_Table, sizeof(H_Counter_Table));
	
	// VDP registers.
	uint32_t *reg = (uint32_t*)&VDP_Reg;
	for (unsigned int i = 0; i < 26; i++)
	{
		md_save_v7.vdp_reg[i] = cpu_to_le32(*reg);
		reg++;
	}
	
	// VDP control.
	reg = (uint32_t*)&Ctrl;
	for (unsigned int i = 0; i < 7; i++)
	{
		md_save_v7.vdp_ctrl[i] = cpu_to_le32(*reg);
		reg++;
	}
	
	// Extra Starscream MC68000 information.
	md_save_v7.starscream_extra.cycles_needed	= cpu_to_le32(Context_68K.cycles_needed);
	md_save_v7.starscream_extra.cycles_leftover	= cpu_to_le32(Context_68K.cycles_leftover);
	md_save_v7.starscream_extra.fetch_region_start	= cpu_to_le32(Context_68K.fetch_region_start);
	md_save_v7.starscream_extra.fetch_region_end	= cpu_to_le32(Context_68K.fetch_region_end);
	md_save_v7.starscream_extra.xflag		= Context_68K.xflag;
	md_save_v7.starscream_extra.execinfo		= Context_68K.execinfo;
	md_save_v7.starscream_extra.trace_trickybit	= Context_68K.trace_trickybit;
	md_save_v7.starscream_extra.filler		= Context_68K.filler;
	md_save_v7.starscream_extra.io_cycle_counter	= cpu_to_le32(Context_68K.io_cycle_counter);
	md_save_v7.starscream_extra.io_fetchbase	= cpu_to_le32(Context_68K.io_fetchbase);
	md_save_v7.starscream_extra.io_fetchbased_pc	= cpu_to_le32(Context_68K.io_fetchbased_pc);
	md_save_v7.starscream_extra.access_address	= cpu_to_le32(Context_68K.access_address);
	md_save_v7.starscream_extra.save_01		= cpu_to_le32(Context_68K.save_01);
	md_save_v7.starscream_extra.save_02		= cpu_to_le32(Context_68K.save_02);
	
	// Copy md_save to the data variable.
	memcpy(&data[GENESIS_LENGTH_EX1], &md_save_v7, sizeof(md_save_v7));
}


/**
 * GsxImportSegaCD(): Load Sega CD data from a savestate.
 * (Ported from Gens Rerecording.)
 * @param data Savestate data.
 */
void Savestate::GsxImportSegaCD(const unsigned char* data)
{
	struct S68000CONTEXT Context_sub68K;
	unsigned char *src;
	int i, j;
	
	sub68k_GetContext(&Context_sub68K);
	
	//sub68K bit goes here
	ImportData(&(Context_sub68K.dreg[0]), data, 0x0, 8 * 4);
	ImportData(&(Context_sub68K.areg[0]), data, 0x20, 8 * 4);
	ImportData(&(Context_sub68K.pc), data, 0x48, 4);
	ImportData(&(Context_sub68K.sr), data, 0x50, 2);
	
	if(data[0x51] & 0x20)
	{
		// Supervisor
		ImportData(&Context_68K.asp, data, 0x52, 4);
	}
	else
	{
		// User
		ImportData(&Context_68K.asp, data, 0x56, 4);
	}
	
	
	ImportData(&Context_sub68K.odometer, data, 0x5A, 4);
	ImportData(Context_sub68K.interrupts, data, 0x60, 8);
	ImportData(&Ram_Word_State, data, 0x6C, 4);
	
	//here ends sub68k bit
	
	//sub68k_SetContext(&Context_sub68K); // Modif N. -- moved to later
	
	//PCM Chip Load
	ImportData(&PCM_Chip.Rate, data, 0x100, 4);
	ImportData(&PCM_Chip.Enable, data, 0x104, 4);
	ImportData(&PCM_Chip.Cur_Chan, data, 0x108, 4);
	ImportData(&PCM_Chip.Bank, data, 0x10C, 4);
	
	for (j = 0; j < 8; j++)
	{
		ImportData(&PCM_Chip.Channel[j].ENV, data, 0x120 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].PAN, data, 0x124 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].MUL_L, data, 0x128 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].MUL_R, data, 0x12C + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].St_Addr, data, 0x130 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Loop_Addr, data, 0x134 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Addr, data, 0x138 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Step, data, 0x13C + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Step_B, data, 0x140 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Enable, data, 0x144 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Data, data, 0x148 + (j * 4 * 11), 4);
	}
	
	//End PCM Chip Load
	
	//Init_RS_GFX(); //purge old GFX data
	//GFX State Load
	ImportData(&(Rot_Comp.Stamp_Size), data, 0x300, 4);
	ImportData(&(Rot_Comp.Stamp_Map_Adr), data, 0x304, 4);
	ImportData(&(Rot_Comp.IB_V_Cell_Size), data, 0x308, 4);
	ImportData(&(Rot_Comp.IB_Adr), data, 0x30C, 4);
	ImportData(&(Rot_Comp.IB_Offset), data, 0x310, 4);
	ImportData(&(Rot_Comp.IB_H_Dot_Size), data, 0x314, 4);
	ImportData(&(Rot_Comp.IB_V_Dot_Size), data, 0x318, 4);
	ImportData(&(Rot_Comp.Vector_Adr), data, 0x31C, 4);
	ImportData(&(Rot_Comp.Rotation_Running), data, 0x320, 4);
	//End GFX State Load
	
	//gate array bit
	ImportData(&COMM.Flag, data, 0x0500, 4);
	src = (unsigned char *) &COMM.Command;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			*src++ = data[i + 0x0504 + j];
		}
	}
	src = (unsigned char *) &COMM.Status;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			*src++ = data[i + 0x0514 + j];
		}
	}
	ImportData(&Memory_Control_Status, data, 0x0524, 4);
	ImportData(&Init_Timer_INT3, data, 0x0528, 4);
	ImportData(&Timer_INT3, data, 0x052C, 4);
	ImportData(&Timer_Step, data, 0x0530, 4);
	ImportData(&Int_Mask_S68K, data, 0x0534, 4);
	ImportData(&Font_COLOR, data, 0x0538, 4);
	ImportData(&Font_BITS, data, 0x053C, 4);
	ImportData(&CD_Access_Timer, data, 0x0540, 4);
	ImportData(&SCD.Status_CDC, data, 0x0544, 4);
	ImportData(&SCD.Status_CDD, data, 0x0548, 4);
	ImportData(&SCD.Cur_LBA, data, 0x054C, 4);
	ImportData(&SCD.Cur_Track, data, 0x0550, 4);
	ImportData(&S68K_Mem_WP, data, 0x0554, 4);
	ImportData(&S68K_Mem_PM, data, 0x0558, 4);
	// More goes here when found
	//here ends gate array bit
	
	//Misc Status Flags
	ImportData(&Ram_Word_State, data, 0xF00, 4); //For determining 1M or 2M
	ImportData(&LED_Status, data, 0xF08, 4); //So the LED shows up properly
	//Word RAM state
	
	//Prg RAM
	ImportData(Ram_Prg, data, 0x1000, 0x80000);
	
	//Word RAM
	if (Ram_Word_State >= 2)
		ImportData(Ram_Word_1M, data, 0x81000, 0x40000); //1M mode
	else
		ImportData(Ram_Word_2M, data, 0x81000, 0x40000); //2M mode
		//ImportData(Ram_Word_2M, data, 0x81000, 0x40000); //2M mode
	//Word RAM end
	
	ImportData(Ram_PCM, data, 0xC1000, 0x10000); //PCM RAM
	
	//CDD & CDC Data
	//CDD
	unsigned int CDD_data[8]; //makes an array for reading CDD unsigned int Data into
	for (j = 0; j < 8; j++)
	{
		ImportData(&CDD_data[j], data, 0xD1000  + (4 * j), 4);
	}
	for(i = 0; i < 10; i++)
		CDD.Rcv_Status[i] = data[0xD1020 + i];
	for(i = 0; i < 10; i++)
		CDD.Trans_Comm[i] = data[0xD102A + i];
	CDD.Fader = CDD_data[0];
	CDD.Control = CDD_data[1];
	CDD.Cur_Comm = CDD_data[2];
	CDD.Status = CDD_data[3];
	CDD.Minute = CDD_data[4];
	CDD.Seconde = CDD_data[5];
	CDD.Frame = CDD_data[6];
	CDD.Ext = CDD_data[7];
	if (CDD.Status & PLAYING)
		if (IsAsyncAllowed()) // Modif N. -- disabled call to resume in synchronous mode (it's unnecessary there and can cause desyncs)
			FILE_Play_CD_LBA(0); // and replaced Resume_CDD_c7 with a call to preload the (new) current MP3 when a savestate is loaded (mainly for sound quality and camhack stability reasons), or do nothing if it's not an MP3
	//CDD end
	
	//CDC
	ImportData(&CDC.RS0, data, 0xD1034, 4);
	ImportData(&CDC.RS1, data, 0xD1038, 4);
	ImportData(&CDC.Host_Data, data, 0xD103C, 4);
	ImportData(&CDC.DMA_Adr, data, 0xD1040, 4);
	ImportData(&CDC.Stop_Watch, data, 0xD1044, 4);
	ImportData(&CDC.COMIN, data, 0xD1048, 4);
	ImportData(&CDC.IFSTAT, data, 0xD104C, 4);
	ImportData(&CDC.DBC.N, data, 0xD1050, 4);
	ImportData(&CDC.DAC.N, data, 0xD1054, 4);
	ImportData(&CDC.HEAD.N, data, 0xD1058, 4);
	ImportData(&CDC.PT.N, data, 0xD105C, 4);
	ImportData(&CDC.WA.N, data, 0xD1060, 4);
	ImportData(&CDC.STAT.N, data, 0xD1064, 4);
	ImportData(&CDC.SBOUT, data, 0xD1068, 4);
	ImportData(&CDC.IFCTRL, data, 0xD106C, 4);
	ImportData(&CDC.CTRL.N, data, 0xD1070, 4);
	ImportData(CDC.Buffer, data, 0xD1074, ((32 * 1024 * 2) + 2352)); //Modif N. - added the *2 because the buffer appears to be that large
	//CDC end
	//CDD & CDC Data end
	
	if (m_Version >= 7)
	{
		//Modif N. - extra stuff added to save/set for synchronization reasons
		// I'm not sure how much of this really needs to be saved, should check it sometime
		unsigned int offset = SEGACD_LENGTH_EX1;
		
		ImportDataAuto(&File_Add_Delay, data, offset, 4);
//		ImportDataAuto(CD_Audio_Buffer_L, data, offset, 4*8192); // removed, seems to be unnecessary
//		ImportDataAuto(CD_Audio_Buffer_R, data, offset, 4*8192); // removed, seems to be unnecessary
		ImportDataAuto(&CD_Audio_Buffer_Read_Pos, data, offset, 4);
		ImportDataAuto(&CD_Audio_Buffer_Write_Pos, data, offset, 4);
		ImportDataAuto(&CD_Audio_Starting, data, offset, 4);
		ImportDataAuto(&CD_Present, data, offset, 4);
		ImportDataAuto(&CD_Load_System, data, offset, 4);
		ImportDataAuto(&CD_Timer_Counter, data, offset, 4);
		ImportDataAuto(&CDD_Complete, data, offset, 4);
		ImportDataAuto(&track_number, data, offset, 4);
		ImportDataAuto(&CD_timer_st, data, offset, 4);
		ImportDataAuto(&CD_LBA_st, data, offset, 4);
		ImportDataAuto(&CDC_Decode_Reg_Read, data, offset, 4);
		
		ImportDataAuto(&SCD, data, offset, sizeof(SCD));
		//ImportDataAuto(&CDC, data, offset, sizeof(CDC)); // removed, seems unnecessary/redundant
		ImportDataAuto(&CDD, data, offset, sizeof(CDD));
		ImportDataAuto(&COMM, data, offset, sizeof(COMM));
		
		ImportDataAuto(Ram_Backup, data, offset, sizeof(Ram_Backup));
		ImportDataAuto(Ram_Backup_Ex, data, offset, sizeof(Ram_Backup_Ex));
		
		ImportDataAuto(&Rot_Comp, data, offset, sizeof(Rot_Comp));
		ImportDataAuto(&Stamp_Map_Adr, data, offset, 4);
		ImportDataAuto(&Buffer_Adr, data, offset, 4);
		ImportDataAuto(&Vector_Adr, data, offset, 4);
		ImportDataAuto(&Jmp_Adr, data, offset, 4);
		ImportDataAuto(&Float_Part, data, offset, 4);
		ImportDataAuto(&Draw_Speed, data, offset, 4);
		ImportDataAuto(&XS, data, offset, 4);
		ImportDataAuto(&YS, data, offset, 4);
		ImportDataAuto(&DXS, data, offset, 4);
		ImportDataAuto(&DYS, data, offset, 4);
		ImportDataAuto(&XD, data, offset, 4);
		ImportDataAuto(&YD, data, offset, 4);
		ImportDataAuto(&XD_Mul, data, offset, 4);
		ImportDataAuto(&H_Dot, data, offset, 4);
		
		ImportDataAuto(&Context_sub68K.cycles_needed, data, offset, 44);
		ImportDataAuto(&Rom_Data[0x72], data, offset, 2); 	//Sega CD games can overwrite the low two bytes of the Horizontal Interrupt vector
		
#ifdef GENS_MP3
		ImportDataAuto(&fatal_mp3_error, data, offset, 4);
		ImportDataAuto(&Current_OUT_Pos, data, offset, 4);
		ImportDataAuto(&Current_OUT_Size, data, offset, 4);
#endif /* GENS_MP3 */
		ImportDataAuto(&Track_Played, data, offset, 1);
		ImportDataAuto(played_tracks_linear, data, offset, 100);
		//ImportDataAuto(&Current_IN_Pos, data, offset, 4)? // don't save this; bad things happen
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == SEGACD_LENGTH_EX);
#endif
	}
	
	sub68k_SetContext(&Context_sub68K); // Modif N. -- moved here from earlier in the function
	
	M68K_Set_Prg_Ram();
	MS68K_Set_Word_Ram();	
}


/**
 * GsxExportSegaCD(): Save Sega CD data to a savestate.
 * (Uses Gens v7 format from Gens Rerecording.)
 * @param data Savestate data buffer.
 */
void Savestate::GsxExportSegaCD(unsigned char* data)
{
	struct S68000CONTEXT Context_sub68K;
	unsigned char *src;
	int i,j;
	
	sub68k_GetContext(&Context_sub68K);
	
	//sub68K bit goes here
	ExportData(&Context_sub68K.dreg[0], data, 0x0, 8 * 4);
	ExportData(&Context_sub68K.areg[0], data, 0x20, 8 * 4);
	ExportData(&Context_sub68K.pc, data, 0x48, 4);
	ExportData(&Context_sub68K.sr, data, 0x50, 2);
	
	if (Context_sub68K.sr & 0x2000)
	{
		ExportData(&Context_sub68K.asp, data, 0x52, 4);
		ExportData(&Context_sub68K.areg[7], data, 0x56, 4);
	}
	else
	{
		ExportData(&Context_sub68K.asp, data, 0x56, 4);
		ExportData(&Context_sub68K.areg[7], data, 0x52, 4);
	}
	
	ExportData(&Context_sub68K.odometer, data, 0x5A, 4);
	ExportData(Context_sub68K.interrupts, data, 0x60, 8);
	ExportData(&Ram_Word_State, data, 0x6C, 4);
	//here ends sub68k bit
	
	//PCM Chip dump
	ExportData(&PCM_Chip.Rate, data, 0x100, 4);
	ExportData(&PCM_Chip.Enable, data, 0x104, 4);
	ExportData(&PCM_Chip.Cur_Chan, data, 0x108, 4);
	ExportData(&PCM_Chip.Bank, data, 0x10C, 4);
	
	for (j = 0; j < 8; j++)
	{
		ExportData(&PCM_Chip.Channel[j].ENV, data, 0x120 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].PAN, data, 0x124 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].MUL_L, data, 0x128 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].MUL_R, data, 0x12C + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].St_Addr, data, 0x130 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Loop_Addr, data, 0x134 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Addr, data, 0x138 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Step, data, 0x13C + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Step_B, data, 0x140 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Enable, data, 0x144 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Data, data, 0x148 + (j * 4 * 11), 4);
	}
	//End PCM Chip Dump

	//GFX State Dump
	ExportData(&Rot_Comp.Stamp_Size, data, 0x300, 4);
	ExportData(&Rot_Comp.Stamp_Map_Adr, data, 0x304, 4);
	ExportData(&Rot_Comp.IB_V_Cell_Size, data, 0x308, 4);
	ExportData(&Rot_Comp.IB_Adr, data, 0x30C, 4);
	ExportData(&Rot_Comp.IB_Offset, data, 0x310, 4);
	ExportData(&Rot_Comp.IB_H_Dot_Size, data, 0x314, 4);
	ExportData(&Rot_Comp.IB_V_Dot_Size, data, 0x318, 4);
	ExportData(&Rot_Comp.Vector_Adr, data, 0x31C, 4);
	ExportData(&Rot_Comp.Rotation_Running, data, 0x320, 4);
	//End GFX State Dump

	//gate array bit
	ExportData(&COMM.Flag, data, 0x0500, 4);
	src = (unsigned char *) &COMM.Command;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			data[i + 0x0504 + j] = *src++;
		}
	}
	src = (unsigned char *) &COMM.Status;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			data[i + 0x0514 + j] = *src++;
		}
	}
	ExportData(&Memory_Control_Status, data, 0x0524, 4);
	ExportData(&Init_Timer_INT3, data, 0x0528, 4);
	ExportData(&Timer_INT3, data, 0x052C, 4);
	ExportData(&Timer_Step, data, 0x0530, 4);
	ExportData(&Int_Mask_S68K, data, 0x0534, 4);
	ExportData(&Font_COLOR, data, 0x0538, 4);
	ExportData(&Font_BITS, data, 0x053C, 4);
	ExportData(&CD_Access_Timer, data, 0x0540, 4);
	ExportData(&SCD.Status_CDC, data, 0x0544, 4);
	ExportData(&SCD.Status_CDD, data, 0x0548, 4);
	ExportData(&SCD.Cur_LBA, data, 0x054C, 4);
	ExportData(&SCD.Cur_Track, data, 0x0550, 4);
	ExportData(&S68K_Mem_WP, data, 0x0554, 4);
	ExportData(&S68K_Mem_PM, data, 0x0558, 4);
	// More goes here When found
	//here ends gate array bit
	
	//Misc Status Flags
	ExportData(&Ram_Word_State, data, 0xF00, 4); //For determining 1M or 2M
	ExportData(&LED_Status, data, 0xF08, 4); //So the LED shows up properly
	//Word RAM state
	
	//Prg RAM
	ExportData(Ram_Prg, data, 0x1000, 0x80000);
	
	//Word RAM
	if (Ram_Word_State >= 2)
		ExportData(Ram_Word_1M, data, 0x81000, 0x40000); //1M mode
	else
		ExportData(Ram_Word_2M, data, 0x81000, 0x40000); //2M mode
	//Word RAM end
	
	ExportData(Ram_PCM, data, 0xC1000, 0x10000); //PCM RAM
	
	//CDD & CDC Data
	//CDD
	unsigned int CDD_src[8] = {CDD.Fader, CDD.Control, CDD.Cur_Comm, CDD.Status,
				   CDD.Minute, CDD.Seconde, CDD.Frame, CDD.Ext}; // Makes an array for easier loop construction
	for (j = 0; j < 8; j++)
	{
		ExportData(&CDD_src[j], data, 0xD1000  + (4 * j), 4);
	}
	for(i = 0; i < 10; i++)
		data[0xD1020 + i] = CDD.Rcv_Status[i];
	for(i = 0; i < 10; i++)
		data[0xD102A + i] = CDD.Trans_Comm[i];
	//CDD end
	
	//CDC
	ExportData(&CDC.RS0, data, 0xD1034, 4);
	ExportData(&CDC.RS1, data, 0xD1038, 4);
	ExportData(&CDC.Host_Data, data, 0xD103C, 4);
	ExportData(&CDC.DMA_Adr, data, 0xD1040, 4);
	ExportData(&CDC.Stop_Watch, data, 0xD1044, 4);
	ExportData(&CDC.COMIN, data, 0xD1048, 4);
	ExportData(&CDC.IFSTAT, data, 0xD104C, 4);
	ExportData(&CDC.DBC.N, data, 0xD1050, 4);
	ExportData(&CDC.DAC.N, data, 0xD1054, 4);
	ExportData(&CDC.HEAD.N, data, 0xD1058, 4);
	ExportData(&CDC.PT.N, data, 0xD105C, 4);
	ExportData(&CDC.WA.N, data, 0xD1060, 4);
	ExportData(&CDC.STAT.N, data, 0xD1064, 4);
	ExportData(&CDC.SBOUT, data, 0xD1068, 4);
	ExportData(&CDC.IFCTRL, data, 0xD106C, 4);
	ExportData(&CDC.CTRL.N, data, 0xD1070, 4);
	ExportData(CDC.Buffer, data, 0xD1074, ((32 * 1024 * 2) + 2352)); //Modif N. - added the *2 because the buffer appears to be that large
	//CDC end
	//CDD & CDC Data end
	
	//Modif N. - extra stuff added to save/set for synchronization reasons
	// I'm not sure how much of this really needs to be saved, should check it sometime
	
	unsigned int offset = SEGACD_LENGTH_EX1;
	
	ExportDataAuto(&File_Add_Delay, data, offset, 4);
//	ExportDataAuto(CD_Audio_Buffer_L, data, offset, 4*8192); // removed, seems to be unnecessary
//	ExportDataAuto(CD_Audio_Buffer_R, data, offset, 4*8192); // removed, seems to be unnecessary
	ExportDataAuto(&CD_Audio_Buffer_Read_Pos, data, offset, 4);
	ExportDataAuto(&CD_Audio_Buffer_Write_Pos, data, offset, 4);
	ExportDataAuto(&CD_Audio_Starting, data, offset, 4);
	ExportDataAuto(&CD_Present, data, offset, 4);
	ExportDataAuto(&CD_Load_System, data, offset, 4);
	ExportDataAuto(&CD_Timer_Counter, data, offset, 4);
	ExportDataAuto(&CDD_Complete, data, offset, 4);
	ExportDataAuto(&track_number, data, offset, 4);
	ExportDataAuto(&CD_timer_st, data, offset, 4);
	ExportDataAuto(&CD_LBA_st, data, offset, 4);
	ExportDataAuto(&CDC_Decode_Reg_Read, data, offset, 4);
	
	ExportDataAuto(&SCD, data, offset, sizeof(SCD));
//	ExportDataAuto(&CDC, data, offset, sizeof(CDC)); // removed, seems unnecessary/redundant
	ExportDataAuto(&CDD, data, offset, sizeof(CDD));
	ExportDataAuto(&COMM, data, offset, sizeof(COMM));
	
	ExportDataAuto(Ram_Backup, data, offset, sizeof(Ram_Backup));
	ExportDataAuto(Ram_Backup_Ex, data, offset, sizeof(Ram_Backup_Ex));
	
	ExportDataAuto(&Rot_Comp, data, offset, sizeof(Rot_Comp));
	ExportDataAuto(&Stamp_Map_Adr, data, offset, 4);
	ExportDataAuto(&Buffer_Adr, data, offset, 4);
	ExportDataAuto(&Vector_Adr, data, offset, 4);
	ExportDataAuto(&Jmp_Adr, data, offset, 4);
	ExportDataAuto(&Float_Part, data, offset, 4);
	ExportDataAuto(&Draw_Speed, data, offset, 4);
	ExportDataAuto(&XS, data, offset, 4);
	ExportDataAuto(&YS, data, offset, 4);
	ExportDataAuto(&DXS, data, offset, 4);
	ExportDataAuto(&DYS, data, offset, 4);
	ExportDataAuto(&XD, data, offset, 4);
	ExportDataAuto(&YD, data, offset, 4);
	ExportDataAuto(&XD_Mul, data, offset, 4);
	ExportDataAuto(&H_Dot, data, offset, 4);
	
	ExportDataAuto(&Context_sub68K.cycles_needed, data, offset, 44);
	ExportDataAuto(&Rom_Data[0x72], data, offset, 2);	//Sega CD games can overwrite the low two bytes of the Horizontal Interrupt vector
	
#ifdef GENS_MP3
	ExportDataAuto(&fatal_mp3_error, data, offset, 4);
	ExportDataAuto(&Current_OUT_Pos, data, offset, 4);
	ExportDataAuto(&Current_OUT_Size, data, offset, 4);
#else
	int dummy = 0;
	ExportDataAuto(&dummy, data, offset, 4);
	ExportDataAuto(&dummy, data, offset, 4);
	ExportDataAuto(&dummy, data, offset, 4);
#endif /* GENS_MP3 */
	ExportDataAuto(&Track_Played, data, offset, 1);
	ExportDataAuto(played_tracks_linear, data, offset, 100);
	//ExportDataAuto(&Current_IN_Pos, data, offset, 4)? // don't save this; bad things happen
	
#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == SEGACD_LENGTH_EX);
#endif
}


/**
 * GsxImport32X(): Load 32X data from a GSX savestate.
 * (Ported from Gens Rerecording.)
 * @param data Savestate data.
 */
void Savestate::GsxImport32X(const unsigned char* data)
{
	// TODO: Reimplement v5 support.
	
	unsigned int offset = 0;
	int i, contextNum;
	
	for (contextNum = 0; contextNum < 2; contextNum++)
	{
		SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;

		ImportDataAuto(context->Cache, data, offset, sizeof(context->Cache));
		ImportDataAuto(context->R, data, offset, sizeof(context->R));
		ImportDataAuto(&context->SR, data, offset, sizeof(context->SR));
		ImportDataAuto(&context->INT, data, offset, sizeof(context->INT));
		ImportDataAuto(&context->GBR, data, offset, sizeof(context->GBR));
		ImportDataAuto(&context->VBR, data, offset, sizeof(context->VBR));
		ImportDataAuto(context->INT_QUEUE, data, offset, sizeof(context->INT_QUEUE));
		ImportDataAuto(&context->MACH, data, offset, sizeof(context->MACH));
		ImportDataAuto(&context->MACL, data, offset, sizeof(context->MACL));
		ImportDataAuto(&context->PR, data, offset, sizeof(context->PR));
		ImportDataAuto(&context->PC, data, offset, sizeof(context->PC));
		ImportDataAuto(&context->Status, data, offset, sizeof(context->Status));
		ImportDataAuto(&context->Base_PC, data, offset, sizeof(context->Base_PC));
		ImportDataAuto(&context->Fetch_Start, data, offset, sizeof(context->Fetch_Start));
		ImportDataAuto(&context->Fetch_End, data, offset, sizeof(context->Fetch_End));
		ImportDataAuto(&context->DS_Inst, data, offset, sizeof(context->DS_Inst));
		ImportDataAuto(&context->DS_PC, data, offset, sizeof(context->DS_PC));
		ImportDataAuto(&context->Odometer, data, offset, sizeof(context->Odometer));
		ImportDataAuto(&context->Cycle_TD, data, offset, sizeof(context->Cycle_TD));
		ImportDataAuto(&context->Cycle_IO, data, offset, sizeof(context->Cycle_IO));
		ImportDataAuto(&context->Cycle_Sup, data, offset, sizeof(context->Cycle_Sup));
		ImportDataAuto(context->IO_Reg, data, offset, sizeof(context->IO_Reg));
		ImportDataAuto(&context->DVCR, data, offset, sizeof(context->DVCR));
		ImportDataAuto(&context->DVSR, data, offset, sizeof(context->DVSR));
		ImportDataAuto(&context->DVDNTH, data, offset, sizeof(context->DVDNTH));
		ImportDataAuto(&context->DVDNTL, data, offset, sizeof(context->DVDNTL));
		ImportDataAuto(&context->DRCR0, data, offset, sizeof(context->DRCR0));
		ImportDataAuto(&context->DRCR1, data, offset, sizeof(context->DRCR1));
		ImportDataAuto(&context->DREQ0, data, offset, sizeof(context->DREQ0));
		ImportDataAuto(&context->DREQ1, data, offset, sizeof(context->DREQ1));
		ImportDataAuto(&context->DMAOR, data, offset, sizeof(context->DMAOR));
		ImportDataAuto(&context->SAR0, data, offset, sizeof(context->SAR0));
		ImportDataAuto(&context->DAR0, data, offset, sizeof(context->DAR0));
		ImportDataAuto(&context->TCR0, data, offset, sizeof(context->TCR0));
		ImportDataAuto(&context->CHCR0, data, offset, sizeof(context->CHCR0));
		ImportDataAuto(&context->SAR1, data, offset, sizeof(context->SAR1));
		ImportDataAuto(&context->DAR1, data, offset, sizeof(context->DAR1));
		ImportDataAuto(&context->TCR1, data, offset, sizeof(context->TCR1));
		ImportDataAuto(&context->CHCR1, data, offset, sizeof(context->CHCR1));
		ImportDataAuto(&context->VCRDIV, data, offset, sizeof(context->VCRDIV));
		ImportDataAuto(&context->VCRDMA0, data, offset, sizeof(context->VCRDMA0));
		ImportDataAuto(&context->VCRDMA1, data, offset, sizeof(context->VCRDMA1));
		ImportDataAuto(&context->VCRWDT, data, offset, sizeof(context->VCRWDT));
		ImportDataAuto(&context->IPDIV, data, offset, sizeof(context->IPDIV));
		ImportDataAuto(&context->IPDMA, data, offset, sizeof(context->IPDMA));
		ImportDataAuto(&context->IPWDT, data, offset, sizeof(context->IPWDT));
		ImportDataAuto(&context->IPBSC, data, offset, sizeof(context->IPBSC));
		ImportDataAuto(&context->BARA, data, offset, sizeof(context->BARA));
		ImportDataAuto(&context->BAMRA, data, offset, sizeof(context->BAMRA));
		ImportDataAuto(context->WDT_Tab, data, offset, sizeof(context->WDT_Tab));
		ImportDataAuto(&context->WDTCNT, data, offset, sizeof(context->WDTCNT));
		ImportDataAuto(&context->WDT_Sft, data, offset, sizeof(context->WDT_Sft));
		ImportDataAuto(&context->WDTSR, data, offset, sizeof(context->WDTSR));
		ImportDataAuto(&context->WDTRST, data, offset, sizeof(context->WDTRST));
		ImportDataAuto(context->FRT_Tab, data, offset, sizeof(context->FRT_Tab));
		ImportDataAuto(&context->FRTCNT, data, offset, sizeof(context->FRTCNT));
		ImportDataAuto(&context->FRTOCRA, data, offset, sizeof(context->FRTOCRA));
		ImportDataAuto(&context->FRTOCRB, data, offset, sizeof(context->FRTOCRB));
		ImportDataAuto(&context->FRTTIER, data, offset, sizeof(context->FRTTIER));
		ImportDataAuto(&context->FRTCSR, data, offset, sizeof(context->FRTCSR));
		ImportDataAuto(&context->FRTTCR, data, offset, sizeof(context->FRTTCR));
		ImportDataAuto(&context->FRTTOCR, data, offset, sizeof(context->FRTTOCR));
		ImportDataAuto(&context->FRTICR, data, offset, sizeof(context->FRTICR));
		ImportDataAuto(&context->FRT_Sft, data, offset, sizeof(context->FRT_Sft));
		ImportDataAuto(&context->BCR1, data, offset, sizeof(context->BCR1));
		ImportDataAuto(&context->FRTCSR, data, offset, sizeof(context->FRTCSR));
	}

	ImportDataAuto(_32X_Ram, data, offset, sizeof(_32X_Ram));
	ImportDataAuto(_MSH2_Reg, data, offset, sizeof(_MSH2_Reg));
	ImportDataAuto(_SSH2_Reg, data, offset, sizeof(_SSH2_Reg));
	ImportDataAuto(_SH2_VDP_Reg, data, offset, sizeof(_SH2_VDP_Reg));
	ImportDataAuto(_32X_Comm, data, offset, sizeof(_32X_Comm));
	ImportDataAuto(&_32X_ADEN, data, offset, sizeof(_32X_ADEN));
	ImportDataAuto(&_32X_RES, data, offset, sizeof(_32X_RES));
	ImportDataAuto(&_32X_FM, data, offset, sizeof(_32X_FM));
	ImportDataAuto(&_32X_RV, data, offset, sizeof(_32X_RV));
	ImportDataAuto(&_32X_DREQ_ST, data, offset, sizeof(_32X_DREQ_ST));
	ImportDataAuto(&_32X_DREQ_SRC, data, offset, sizeof(_32X_DREQ_SRC));
	ImportDataAuto(&_32X_DREQ_DST, data, offset, sizeof(_32X_DREQ_DST));
	ImportDataAuto(&_32X_DREQ_LEN, data, offset, sizeof(_32X_DREQ_LEN));
	ImportDataAuto(_32X_FIFO_A, data, offset, sizeof(_32X_FIFO_A));
	ImportDataAuto(_32X_FIFO_B, data, offset, sizeof(_32X_FIFO_B));
	ImportDataAuto(&_32X_FIFO_Block, data, offset, sizeof(_32X_FIFO_Block));
	ImportDataAuto(&_32X_FIFO_Read, data, offset, sizeof(_32X_FIFO_Read));
	ImportDataAuto(&_32X_FIFO_Write, data, offset, sizeof(_32X_FIFO_Write));
	ImportDataAuto(&_32X_MINT, data, offset, sizeof(_32X_MINT));
	ImportDataAuto(&_32X_SINT, data, offset, sizeof(_32X_SINT));
	ImportDataAuto(&_32X_HIC, data, offset, sizeof(_32X_HIC));
	ImportDataAuto(&CPL_SSH2, data, offset, sizeof(CPL_SSH2));
	ImportDataAuto(&CPL_MSH2, data, offset, sizeof(CPL_MSH2));
	ImportDataAuto(&Cycles_MSH2, data, offset, sizeof(Cycles_MSH2));
	ImportDataAuto(&Cycles_SSH2, data, offset, sizeof(Cycles_SSH2));

	ImportDataAuto(&_32X_VDP, data, offset, sizeof(_32X_VDP));
	ImportDataAuto(_32X_VDP_Ram, data, offset, sizeof(_32X_VDP_Ram));
	ImportDataAuto(_32X_VDP_CRam, data, offset, sizeof(_32X_VDP_CRam));

	ImportDataAuto(Set_SR_Table, data, offset, sizeof(Set_SR_Table));
	ImportDataAuto(&Bank_SH2, data, offset, sizeof(Bank_SH2));

	ImportDataAuto(PWM_FIFO_R, data, offset, sizeof(PWM_FIFO_R));
	ImportDataAuto(PWM_FIFO_L, data, offset, sizeof(PWM_FIFO_L));
	ImportDataAuto(&PWM_RP_R, data, offset, sizeof(PWM_RP_R));
	ImportDataAuto(&PWM_WP_R, data, offset, sizeof(PWM_WP_R));
	ImportDataAuto(&PWM_RP_L, data, offset, sizeof(PWM_RP_L));
	ImportDataAuto(&PWM_WP_L, data, offset, sizeof(PWM_WP_L));
	ImportDataAuto(&PWM_Cycles, data, offset, sizeof(PWM_Cycles));
	ImportDataAuto(&PWM_Cycle, data, offset, sizeof(PWM_Cycle));
	ImportDataAuto(&PWM_Cycle_Cnt, data, offset, sizeof(PWM_Cycle_Cnt));
	ImportDataAuto(&PWM_Int, data, offset, sizeof(PWM_Int));
	ImportDataAuto(&PWM_Int_Cnt, data, offset, sizeof(PWM_Int_Cnt));
	ImportDataAuto(&PWM_Mode, data, offset, sizeof(PWM_Mode));
	ImportDataAuto(&PWM_Out_R, data, offset, sizeof(PWM_Out_R));
	ImportDataAuto(&PWM_Out_L, data, offset, sizeof(PWM_Out_L));
	PWM_Recalc_Scale();
	
	ImportDataAuto(_32X_Rom, data, offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	ImportDataAuto(_32X_MSH2_Rom, data, offset, sizeof(_32X_MSH2_Rom));
	ImportDataAuto(_32X_SSH2_Rom, data, offset, sizeof(_32X_SSH2_Rom));

	M68K_32X_Mode();
	_32X_Set_FB();
	M68K_Set_32X_Rom_Bank();

	//Recalculate_Palettes();
	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Adjusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
		_32X_VDP_CRam_Adjusted32[i] = _32X_Palette_32B[_32X_VDP_CRam[i]];
	}

#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == G32X_LENGTH_EX);
#endif
}


/**
 * GsxExport32X(): Save 32X data to a GSX savestate.
 * (Ported from Gens Rerecording.)
 * @param data Savestate data buffer.
 */
void Savestate::GsxExport32X(unsigned char* data)
{
	// NOTE: Gens v7 Savestates with 32X data will *not* work on older versions of Gens!
	unsigned int offset = 0;
	int contextNum;
	
	for (contextNum = 0; contextNum < 2; contextNum++)
	{
		SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;
	
		ExportDataAuto(context->Cache, data, offset, sizeof(context->Cache));
		ExportDataAuto(context->R, data, offset, sizeof(context->R));
		ExportDataAuto(&context->SR, data, offset, sizeof(context->SR));
		ExportDataAuto(&context->INT, data, offset, sizeof(context->INT));
		ExportDataAuto(&context->GBR, data, offset, sizeof(context->GBR));
		ExportDataAuto(&context->VBR, data, offset, sizeof(context->VBR));
		ExportDataAuto(context->INT_QUEUE, data, offset, sizeof(context->INT_QUEUE));
		ExportDataAuto(&context->MACH, data, offset, sizeof(context->MACH));
		ExportDataAuto(&context->MACL, data, offset, sizeof(context->MACL));
		ExportDataAuto(&context->PR, data, offset, sizeof(context->PR));
		ExportDataAuto(&context->PC, data, offset, sizeof(context->PC));
		ExportDataAuto(&context->Status, data, offset, sizeof(context->Status));
		ExportDataAuto(&context->Base_PC, data, offset, sizeof(context->Base_PC));
		ExportDataAuto(&context->Fetch_Start, data, offset, sizeof(context->Fetch_Start));
		ExportDataAuto(&context->Fetch_End, data, offset, sizeof(context->Fetch_End));
		ExportDataAuto(&context->DS_Inst, data, offset, sizeof(context->DS_Inst));
		ExportDataAuto(&context->DS_PC, data, offset, sizeof(context->DS_PC));
		ExportDataAuto(&context->Odometer, data, offset, sizeof(context->Odometer));
		ExportDataAuto(&context->Cycle_TD, data, offset, sizeof(context->Cycle_TD));
		ExportDataAuto(&context->Cycle_IO, data, offset, sizeof(context->Cycle_IO));
		ExportDataAuto(&context->Cycle_Sup, data, offset, sizeof(context->Cycle_Sup));
		ExportDataAuto(context->IO_Reg, data, offset, sizeof(context->IO_Reg));
		ExportDataAuto(&context->DVCR, data, offset, sizeof(context->DVCR));
		ExportDataAuto(&context->DVSR, data, offset, sizeof(context->DVSR));
		ExportDataAuto(&context->DVDNTH, data, offset, sizeof(context->DVDNTH));
		ExportDataAuto(&context->DVDNTL, data, offset, sizeof(context->DVDNTL));
		ExportDataAuto(&context->DRCR0, data, offset, sizeof(context->DRCR0));
		ExportDataAuto(&context->DRCR1, data, offset, sizeof(context->DRCR1));
		ExportDataAuto(&context->DREQ0, data, offset, sizeof(context->DREQ0));
		ExportDataAuto(&context->DREQ1, data, offset, sizeof(context->DREQ1));
		ExportDataAuto(&context->DMAOR, data, offset, sizeof(context->DMAOR));
		ExportDataAuto(&context->SAR0, data, offset, sizeof(context->SAR0));
		ExportDataAuto(&context->DAR0, data, offset, sizeof(context->DAR0));
		ExportDataAuto(&context->TCR0, data, offset, sizeof(context->TCR0));
		ExportDataAuto(&context->CHCR0, data, offset, sizeof(context->CHCR0));
		ExportDataAuto(&context->SAR1, data, offset, sizeof(context->SAR1));
		ExportDataAuto(&context->DAR1, data, offset, sizeof(context->DAR1));
		ExportDataAuto(&context->TCR1, data, offset, sizeof(context->TCR1));
		ExportDataAuto(&context->CHCR1, data, offset, sizeof(context->CHCR1));
		ExportDataAuto(&context->VCRDIV, data, offset, sizeof(context->VCRDIV));
		ExportDataAuto(&context->VCRDMA0, data, offset, sizeof(context->VCRDMA0));
		ExportDataAuto(&context->VCRDMA1, data, offset, sizeof(context->VCRDMA1));
		ExportDataAuto(&context->VCRWDT, data, offset, sizeof(context->VCRWDT));
		ExportDataAuto(&context->IPDIV, data, offset, sizeof(context->IPDIV));
		ExportDataAuto(&context->IPDMA, data, offset, sizeof(context->IPDMA));
		ExportDataAuto(&context->IPWDT, data, offset, sizeof(context->IPWDT));
		ExportDataAuto(&context->IPBSC, data, offset, sizeof(context->IPBSC));
		ExportDataAuto(&context->BARA, data, offset, sizeof(context->BARA));
		ExportDataAuto(&context->BAMRA, data, offset, sizeof(context->BAMRA));
		ExportDataAuto(context->WDT_Tab, data, offset, sizeof(context->WDT_Tab));
		ExportDataAuto(&context->WDTCNT, data, offset, sizeof(context->WDTCNT));
		ExportDataAuto(&context->WDT_Sft, data, offset, sizeof(context->WDT_Sft));
		ExportDataAuto(&context->WDTSR, data, offset, sizeof(context->WDTSR));
		ExportDataAuto(&context->WDTRST, data, offset, sizeof(context->WDTRST));
		ExportDataAuto(context->FRT_Tab, data, offset, sizeof(context->FRT_Tab));
		ExportDataAuto(&context->FRTCNT, data, offset, sizeof(context->FRTCNT));
		ExportDataAuto(&context->FRTOCRA, data, offset, sizeof(context->FRTOCRA));
		ExportDataAuto(&context->FRTOCRB, data, offset, sizeof(context->FRTOCRB));
		ExportDataAuto(&context->FRTTIER, data, offset, sizeof(context->FRTTIER));
		ExportDataAuto(&context->FRTCSR, data, offset, sizeof(context->FRTCSR));
		ExportDataAuto(&context->FRTTCR, data, offset, sizeof(context->FRTTCR));
		ExportDataAuto(&context->FRTTOCR, data, offset, sizeof(context->FRTTOCR));
		ExportDataAuto(&context->FRTICR, data, offset, sizeof(context->FRTICR));
		ExportDataAuto(&context->FRT_Sft, data, offset, sizeof(context->FRT_Sft));
		ExportDataAuto(&context->BCR1, data, offset, sizeof(context->BCR1));
		ExportDataAuto(&context->FRTCSR, data, offset, sizeof(context->FRTCSR));
	}
	
	ExportDataAuto(_32X_Ram, data, offset, sizeof(_32X_Ram));
	ExportDataAuto(_MSH2_Reg, data, offset, sizeof(_MSH2_Reg));
	ExportDataAuto(_SSH2_Reg, data, offset, sizeof(_SSH2_Reg));
	ExportDataAuto(_SH2_VDP_Reg, data, offset, sizeof(_SH2_VDP_Reg));
	ExportDataAuto(_32X_Comm, data, offset, sizeof(_32X_Comm));
	ExportDataAuto(&_32X_ADEN, data, offset, sizeof(_32X_ADEN));
	ExportDataAuto(&_32X_RES, data, offset, sizeof(_32X_RES));
	ExportDataAuto(&_32X_FM, data, offset, sizeof(_32X_FM));
	ExportDataAuto(&_32X_RV, data, offset, sizeof(_32X_RV));
	ExportDataAuto(&_32X_DREQ_ST, data, offset, sizeof(_32X_DREQ_ST));
	ExportDataAuto(&_32X_DREQ_SRC, data, offset, sizeof(_32X_DREQ_SRC));
	ExportDataAuto(&_32X_DREQ_DST, data, offset, sizeof(_32X_DREQ_DST));
	ExportDataAuto(&_32X_DREQ_LEN, data, offset, sizeof(_32X_DREQ_LEN));
	ExportDataAuto(_32X_FIFO_A, data, offset, sizeof(_32X_FIFO_A));
	ExportDataAuto(_32X_FIFO_B, data, offset, sizeof(_32X_FIFO_B));
	ExportDataAuto(&_32X_FIFO_Block, data, offset, sizeof(_32X_FIFO_Block));
	ExportDataAuto(&_32X_FIFO_Read, data, offset, sizeof(_32X_FIFO_Read));
	ExportDataAuto(&_32X_FIFO_Write, data, offset, sizeof(_32X_FIFO_Write));
	ExportDataAuto(&_32X_MINT, data, offset, sizeof(_32X_MINT));
	ExportDataAuto(&_32X_SINT, data, offset, sizeof(_32X_SINT));
	ExportDataAuto(&_32X_HIC, data, offset, sizeof(_32X_HIC));
	ExportDataAuto(&CPL_SSH2, data, offset, sizeof(CPL_SSH2));
	ExportDataAuto(&CPL_MSH2, data, offset, sizeof(CPL_MSH2));
	ExportDataAuto(&Cycles_MSH2, data, offset, sizeof(Cycles_MSH2));
	ExportDataAuto(&Cycles_SSH2, data, offset, sizeof(Cycles_SSH2));
	
	ExportDataAuto(&_32X_VDP, data, offset, sizeof(_32X_VDP));
	ExportDataAuto(_32X_VDP_Ram, data, offset, sizeof(_32X_VDP_Ram));
	ExportDataAuto(_32X_VDP_CRam, data, offset, sizeof(_32X_VDP_CRam));
	
	ExportDataAuto(Set_SR_Table, data, offset, sizeof(Set_SR_Table));
	ExportDataAuto(&Bank_SH2, data, offset, sizeof(Bank_SH2));
	
	ExportDataAuto(PWM_FIFO_R, data, offset, sizeof(PWM_FIFO_R));
	ExportDataAuto(PWM_FIFO_L, data, offset, sizeof(PWM_FIFO_L));
	ExportDataAuto(&PWM_RP_R, data, offset, sizeof(PWM_RP_R));
	ExportDataAuto(&PWM_WP_R, data, offset, sizeof(PWM_WP_R));
	ExportDataAuto(&PWM_RP_L, data, offset, sizeof(PWM_RP_L));
	ExportDataAuto(&PWM_WP_L, data, offset, sizeof(PWM_WP_L));
	ExportDataAuto(&PWM_Cycles, data, offset, sizeof(PWM_Cycles));
	ExportDataAuto(&PWM_Cycle, data, offset, sizeof(PWM_Cycle));
	ExportDataAuto(&PWM_Cycle_Cnt, data, offset, sizeof(PWM_Cycle_Cnt));
	ExportDataAuto(&PWM_Int, data, offset, sizeof(PWM_Int));
	ExportDataAuto(&PWM_Int_Cnt, data, offset, sizeof(PWM_Int_Cnt));
	ExportDataAuto(&PWM_Mode, data, offset, sizeof(PWM_Mode));
	ExportDataAuto(&PWM_Out_R, data, offset, sizeof(PWM_Out_R));
	ExportDataAuto(&PWM_Out_L, data, offset, sizeof(PWM_Out_L));
	
	ExportDataAuto(_32X_Rom, data, offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	ExportDataAuto(_32X_MSH2_Rom, data, offset, sizeof(_32X_MSH2_Rom));
	ExportDataAuto(_32X_SSH2_Rom, data, offset, sizeof(_32X_SSH2_Rom));
	
#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == G32X_LENGTH_EX);
#endif
}


/**
 * GetSRAMFilename(): Get the filename of the SRAM file.
 * @return Filename of the SRAM file.
 */
inline string Savestate::GetSRAMFilename(void)
{
	if (ROM_Filename[0] == 0x00)	// (strlen(ROM_Filename) == 0)
		return "";
	
	return string(State_Dir) + string(ROM_Filename) + ".srm";
}


/**
 * LoadSRAM(): Load the SRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::LoadSRAM(void)
{
	FILE* SRAM_File = 0;
	
	memset(SRAM, 0x00, sizeof(SRAM));
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = GetSRAMFilename();
	if (filename.empty())
		return 0;
	if ((SRAM_File = fopen(filename.c_str(), "rb")) == 0)
		return 0;
	
	fread(SRAM, sizeof(SRAM), 1, SRAM_File);
	fclose(SRAM_File);
	
	string msg = "SRAM loaded from " + filename;
	vdraw_text_write(msg.c_str(), 2000);
	return 1;
}


/**
 * SaveSRAM(): Save the SRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::SaveSRAM(void)
{
	FILE* SRAM_File = 0;
	int size_to_save, i;
	
	i = (64 * 1024) - 1;
	while ((i >= 0) && (SRAM[i] == 0))
		i--;
	
	if (i < 0)
		return 0;
	
	i++;
	
	size_to_save = 1;
	while (i > size_to_save)
		size_to_save <<= 1;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = GetSRAMFilename();
	if (filename.empty())
		return 0;
	if ((SRAM_File = fopen(filename.c_str(), "wb")) == 0)
		return 0;
	
	fwrite(SRAM, size_to_save, 1, SRAM_File);
	fclose(SRAM_File);
	
	string dispText = "SRAM saved in " + filename;
	vdraw_text_write(dispText.c_str(), 2000);
	return 1;
}


/**
 * FormatSegaCD_BRAM(): Format the internal SegaCD BRAM.
 * @param buf Pointer to location 0x1FC0 in the internal SegaCD BRAM.
 */
void Savestate::FormatSegaCD_BRAM(unsigned char *buf)
{
	// TODO: Format cartridge BRAM.
	static const char brmHeader[0x40] =
	{
		0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F,
		0x5F, 0x5F, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x40,
		0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7D, 0x00, 0x7D,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		
		'S', 'E', 'G', 'A', 0x5F, 'C', 'D', 0x5F, 'R', 'O', 'M', 0x00, 0x01, 0x00, 0x00, 0x00,
		'R', 'A', 'M', 0x5F, 'C', 'A', 'R', 'T', 'R', 'I', 'D', 'G', 'E', 0x5F, 0x5F, 0x5F
	};
	
	memcpy(buf, brmHeader, sizeof(brmHeader));
}


/**
 * FormatSegaCD_BackupRAM(): Format the SegaCD backup RAM.
 */
void Savestate::FormatSegaCD_BackupRAM(void)
{
	// SegaCD internal BRAM.
	memset(Ram_Backup, 0x00, sizeof(Ram_Backup));
	FormatSegaCD_BRAM(&Ram_Backup[0x1FC0]);
	
	// SegaCD cartridge memory.
	// TODO: Format the cartridge memory.
	memset(Ram_Backup_Ex, 0x00, sizeof(Ram_Backup_Ex));
}


/**
 * GetSRAMFilename(): Get the filename of the BRAM file.
 * @return Filename of the BRAM file.
 */
inline string Savestate::GetBRAMFilename(void)
{
	if (ROM_Filename[0] == 0x00)	// (strlen(ROM_Filename) == 0)
		return "";
	
	return string(State_Dir) + string(ROM_Filename) + ".brm";
}


/**
 * LoadBRAM(): Load the BRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::LoadBRAM(void)
{
	FILE* BRAM_File = 0;
	
	Savestate::FormatSegaCD_BackupRAM();
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = GetBRAMFilename();
	if (filename.empty())
		return 0;
	if ((BRAM_File = fopen(filename.c_str(), "rb")) == 0)
		return 0;
	
	fread(Ram_Backup, sizeof(Ram_Backup), 1, BRAM_File);
	fread(Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose(BRAM_File);
	
	string dispText = "BRAM loaded from " + filename;
	vdraw_text_write(dispText.c_str(), 2000);
	return 1;
}

/**
 * SaveBRAM(): Save the BRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::SaveBRAM(void)
{
	FILE* BRAM_File = 0;
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = GetBRAMFilename();
	if (filename.empty())
		return 0;
	if ((BRAM_File = fopen(filename.c_str(), "wb")) == 0)
		return 0;
	
	fwrite(Ram_Backup, 8 * 1024, 1, BRAM_File);
	fwrite(Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose(BRAM_File);
	
	string dispText = "BRAM saved in " + filename;
	vdraw_text_write(dispText.c_str(), 2000);
	return 1;
}
