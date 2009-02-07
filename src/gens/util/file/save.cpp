/**
 * GENS: Save file handler.
 */

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
#include "gens_core/sound/ym2612.h"
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

// Byteswapping
#include "gens_core/misc/byteswap.h"

// Video Drawing.
#include "video/vdraw.h"

// GSX savestate structs.
#include "gsx_struct.h"

// Needed for SetCurrentDirectory.
#ifdef GENS_OS_WIN32
#include <windows.h>
#endif /* GENS_OS_WIN32 */

#ifdef GENS_MP3
// Gens Rerecording
// fatal_mp3_error indicates an error occurred while reading an MP3 for a Sega CD game.
extern int fatal_mp3_error;	// cdda_mp3.c

// Various MP3 stuff, needed for Gens Rerecording
extern unsigned int Current_OUT_Pos, Current_OUT_Size;	// cdda_mp3.c
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


/*
int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
{
	int i;
	gchar* filename=NULL;
	static FileSelData fd;
	GtkWidget* widget;
	GList* extensions=NULL;

	widget = init_file_selector(&fd, Titre);
	extensions = g_list_append(extensions,Ext);
	file_selector_add_type(&fd,"", extensions);
	gtk_widget_show(widget);
	filename = fileselection_get_filename(fd.filesel);	

	if (filename)
		{
		i = strlen(Dest) - 1;
		while ((i > 0) && (Dest[i] != GENS_DIR_SEPARATOR_CHR)) i--;
		if (!i) return 0;
		Dest[++i] = 0;
		g_free(filename);
		return 1;
	}
	return 0;
}
*/


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
	return string(State_Dir) + string(ROM_Name) + ".gs" + (char)('0' + Current_State);
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
		static const unsigned char gsxHeader[5] = {'G', 'S', 'T', 0x40, 0xE0};
		if (memcmp(&buf[0], &gsxHeader[0], sizeof(gsxHeader)))
		{
			// Header does not match GSX.
			sprintf(Str_Tmp, "Error: State %d is not in GSX format.", Current_State);
			vdraw_write_text(Str_Tmp, 2000);
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
		
		/*
		// Commented out in Gens Rerecording...
		Flag_Clr_Scr = 1;
		CRam_Flag = 1;
		VRam_Flag = 1;
		*/
		
		sprintf(Str_Tmp, "STATE %d LOADED", Current_State);
		vdraw_write_text(Str_Tmp, 2000);
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
	
	sprintf(Str_Tmp, "STATE %d SAVED", Current_State);
	vdraw_write_text(Str_Tmp, 2000);
	
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
	
	// Copy the CRam, VSRam, and Z80 RAM.
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
	if ((m_Version >= 2) && (m_Version < 4))
	{
		// TODO: Create a struct fr this.
		ImportData(&Ctrl, data, 0x30, 7 * 4);
		
		// 0x440: Z80 busreq
		// 0x444: Z80 reset
		Z80_State &= ~6;
		if (data[0x440] & 1)
			Z80_State |= 2;
		if (data[0x444] & 1)
			Z80_State |= 4;
		
		// 0x448: Z80 bank.
		ImportData(&Bank_Z80, data, 0x448, 4);
		
		// 0x22488: PSG registers.
		// TODO: Import this using correct endianness.
		ImportData(&PSG_Save, data, 0x224B8, 8 * 4);
		PSG_Restore_State();
	}
	else if ((m_Version >= 4) || (m_Version == 0))
	{
		// New savestate version compatible with Kega.
		Z80_State &= ~6;
		
		if (m_Version == 4)
		{
			// Version 4 stores IFF and State differently.
			M_Z80.IM = data[0x437];
			M_Z80.IFF.b.IFF1 = (data[0x438] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (data[0x438] & 1) << 2;
			
			Z80_State |= (data[0x439] & 1) << 1;
		}
		else
		{
			// Other versions use the Gens v5 format.
			M_Z80.IM = 1;
			M_Z80.IFF.b.IFF1 = (md_save.z80_reg.IFF1 & 1) << 2;
			M_Z80.IFF.b.IFF2 = M_Z80.IFF.b.IFF1;
			
			Z80_State |= (md_save.z80_reg.state_busreq ^ 1) << 1;
			Z80_State |= (md_save.z80_reg.state_reset ^ 1) << 2;
		}
		
		// Clear VDP control.
		memset(&Ctrl, 0x00, sizeof(Ctrl));
		
		// Load VDP control settings.
		uint32_t lastCtrlData = le32_to_cpu(md_save.vdp_ctrl.ctrl_data);
		Write_VDP_Ctrl(lastCtrlData & 0xFFFF);
		Write_VDP_Ctrl(lastCtrlData >> 16);
		
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
			{
				PSG_Save[i] = (uint16_t)(le32_to_cpu(md_save.psg[i]));
			}
			PSG_Restore_State();
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
	
	// 68000 registers.
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
		// Gens v6 savestate
		//Modif N. - saving more stuff (although a couple of these are saved above in a weird way that I don't trust)
		ImportDataAuto(&Context_68K.dreg, data, offset, 4*8);
		ImportDataAuto(&Context_68K.areg, data, offset, 4*8);
		ImportDataAuto(&Context_68K.asp, data, offset, 4);
		ImportDataAuto(&Context_68K.pc, data, offset, 4);
		ImportDataAuto(&Context_68K.odometer, data, offset, 4);
		ImportDataAuto(&Context_68K.interrupts, data, offset, 8);
		ImportDataAuto(&Context_68K.sr, data, offset, 2);
		ImportDataAuto(&Context_68K.contextfiller00, data, offset, 2);
		
		ImportDataAuto(&VDP_Reg.H_Int, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Set1, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Set2, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_ScrA_Adr, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_ScrA_Adr, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_Win_Adr, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_ScrB_Adr, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Spr_Att_Adr, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Reg6, data, offset, 4);
		ImportDataAuto(&VDP_Reg.BG_Color, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Reg8, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Reg9, data, offset, 4);
		ImportDataAuto(&VDP_Reg.H_Int, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Set3, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Set4, data, offset, 4);
		ImportDataAuto(&VDP_Reg.H_Scr_Adr, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Reg14, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Auto_Inc, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Scr_Size, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Win_H_Pos, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Win_V_Pos, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length_L, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length_H, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_L, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_M, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_H, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Address, data, offset, 4);
		
		ImportDataAuto(&Controller_1_Counter, data, offset, 4);
		ImportDataAuto(&Controller_1_Delay, data, offset, 4);
		ImportDataAuto(&Controller_1_State, data, offset, 4);
		ImportDataAuto(&Controller_1_COM, data, offset, 4);
		ImportDataAuto(&Controller_2_Counter, data, offset, 4);
		ImportDataAuto(&Controller_2_Delay, data, offset, 4);
		ImportDataAuto(&Controller_2_State, data, offset, 4);
		ImportDataAuto(&Controller_2_COM, data, offset, 4);
		ImportDataAuto(&Memory_Control_Status, data, offset, 4);
		ImportDataAuto(&Cell_Conv_Tab, data, offset, 4);
		
		ImportDataAuto(&Controller_1_Type, data, offset, 4);
		/* TODO: Make this stuff use bitfields.
		         For now, it's disabled, especially since v6 is rare.
		ImportDataAuto(&Controller_1_Up, data, offset, 4);
		ImportDataAuto(&Controller_1_Down, data, offset, 4);
		ImportDataAuto(&Controller_1_Left, data, offset, 4);
		ImportDataAuto(&Controller_1_Right, data, offset, 4);
		ImportDataAuto(&Controller_1_Start, data, offset, 4);
		ImportDataAuto(&Controller_1_Mode, data, offset, 4);
		ImportDataAuto(&Controller_1_A, data, offset, 4);
		ImportDataAuto(&Controller_1_B, data, offset, 4);
		ImportDataAuto(&Controller_1_C, data, offset, 4);
		ImportDataAuto(&Controller_1_X, data, offset, 4);
		ImportDataAuto(&Controller_1_Y, data, offset, 4);
		ImportDataAuto(&Controller_1_Z, data, offset, 4);
		*/
		offset += 12*4;
		
		ImportDataAuto(&Controller_2_Type, data, offset, 4);
		/* TODO: Make this stuff use bitfields.
		         For now, it's disabled, especially since v6 is rare.
		ImportDataAuto(&Controller_2_Up, data, offset, 4);
		ImportDataAuto(&Controller_2_Down, data, offset, 4);
		ImportDataAuto(&Controller_2_Left, data, offset, 4);
		ImportDataAuto(&Controller_2_Right, data, offset, 4);
		ImportDataAuto(&Controller_2_Start, data, offset, 4);
		ImportDataAuto(&Controller_2_Mode, data, offset, 4);
		ImportDataAuto(&Controller_2_A, data, offset, 4);
		ImportDataAuto(&Controller_2_B, data, offset, 4);
		ImportDataAuto(&Controller_2_C, data, offset, 4);
		ImportDataAuto(&Controller_2_X, data, offset, 4);
		ImportDataAuto(&Controller_2_Y, data, offset, 4);
		ImportDataAuto(&Controller_2_Z, data, offset, 4);
		*/
		offset += 12*4;
		
		ImportDataAuto(&DMAT_Length, data, offset, 4);
		ImportDataAuto(&DMAT_Type, data, offset, 4);
		ImportDataAuto(&DMAT_Tmp, data, offset, 4);
		ImportDataAuto(&VDP_Current_Line, data, offset, 4);
		ImportDataAuto(&VDP_Num_Vis_Lines, data, offset, 4);
		ImportDataAuto(&VDP_Num_Vis_Lines, data, offset, 4);
		ImportDataAuto(&Bank_M68K, data, offset, 4);
		ImportDataAuto(&S68K_State, data, offset, 4);
		ImportDataAuto(&Z80_State, data, offset, 4);
		ImportDataAuto(&Last_BUS_REQ_Cnt, data, offset, 4);
		ImportDataAuto(&Last_BUS_REQ_St, data, offset, 4);
		ImportDataAuto(&Fake_Fetch, data, offset, 4);
		ImportDataAuto(&Game_Mode, data, offset, 4);
		ImportDataAuto(&CPU_Mode, data, offset, 4);
		ImportDataAuto(&CPL_M68K, data, offset, 4);
		ImportDataAuto(&CPL_S68K, data, offset, 4);
		ImportDataAuto(&CPL_Z80, data, offset, 4);
		ImportDataAuto(&Cycles_S68K, data, offset, 4);
		ImportDataAuto(&Cycles_M68K, data, offset, 4);
		ImportDataAuto(&Cycles_Z80, data, offset, 4);
		ImportDataAuto(&VDP_Status, data, offset, 4);
		ImportDataAuto(&VDP_Int, data, offset, 4);
		ImportDataAuto(&Ctrl.Write, data, offset, 4);
		ImportDataAuto(&Ctrl.DMA_Mode, data, offset, 4);
		ImportDataAuto(&Ctrl.DMA, data, offset, 4);
		//ImportDataAuto(&CRam_Flag, data, offset, 4); //Causes screen to blank
		//offset+=4;
		
		// TODO: LagCount from Gens Rerecording.
		//LagCount = ImportNumber_le32(data, &offset);
		offset += 4;
		
		ImportDataAuto(&VRam_Flag, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length, data, offset, 4);
		ImportDataAuto(&VDP_Reg.Auto_Inc, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length, data, offset, 4);
		//ImportDataAuto(VRam, data, offset, 65536);
		ImportDataAuto(CRam, data, offset, 512);
		//ImportDataAuto(VSRam, data, offset, 64);
		ImportDataAuto(H_Counter_Table, data, offset, 512 * 2);
		//ImportDataAuto(Spr_Link, data, offset, 4*256);
		//extern int DMAT_Tmp, VSRam_Over;
		//ImportDataAuto(&DMAT_Tmp, data, offset, 4);
		//ImportDataAuto(&VSRam_Over, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length_L, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length_H, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_L, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_M, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_H, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Length, data, offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Address, data, offset, 4);
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == GENESIS_V6_STATE_LENGTH);
#endif
	}
	else if (m_Version >= 7)
	{
		// Gens v7 savestate
		unsigned char Reg_2[sizeof(ym2612_)];
		ImportDataAuto(Reg_2, data, offset, sizeof(ym2612_)); // some Important parts of this weren't saved above
		YM2612_Restore_Full(Reg_2);
		
		ImportDataAuto(PSG_Save_Full, data, offset, sizeof(struct _psg)); // some Important parts of this weren't saved above
		PSG_Restore_State_Full();
		
		// BUG: The Gens v7 savestate stores M_Z80.BasePC, which is a *real* pointer.
		// Also, it stores M_Z80.PC, which is *relative* to M_Z80.BasePC.
		// Workaround: Save M_Z80.BasePC and M_Z80.PC, and restore them after.
		// The PC is set correctly by the older savestate code above via z80_Set_PC().
		unsigned int oldBasePC = M_Z80.BasePC;
		unsigned int oldPC = M_Z80.PC.d;
		ImportDataAuto(&M_Z80, data, offset, 0x5C); // some Important parts of this weren't saved above
		M_Z80.PC.d = oldPC;
		M_Z80.BasePC = oldBasePC;
		
		ImportDataAuto(&M_Z80.RetIC, data, offset, 4); // not sure about the last two variables, might as well save them too
		ImportDataAuto(&M_Z80.IntAckC, data, offset, 4);
		
		ImportDataAuto(&Context_68K.dreg[0], data, offset, 86); // some Important parts of this weren't saved above
		
		ImportDataAuto(&Controller_1_State, data, offset, 448); // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
		
		// apparently necessary
		ImportDataAuto(&VDP_Status, data, offset, 4);
		ImportDataAuto(&VDP_Int, data, offset, 4);
		ImportDataAuto(&VDP_Current_Line, data, offset, 4);
		ImportDataAuto(&VDP_Num_Lines, data, offset, 4);
		ImportDataAuto(&VDP_Num_Vis_Lines, data, offset, 4);
		ImportDataAuto(&DMAT_Length, data, offset, 4);
		ImportDataAuto(&DMAT_Type, data, offset, 4);
		//ImportDataAuto(&CRam_Flag. data. &offset, 4); //emulator flag which causes Gens not to update its draw palette, but doesn't affect sync state
		
		// TODO: LagCount for Gens Rerecording.
		//ImportDataAuto(&LagCount, data, offset, 4);
		offset += 4;
		
		ImportDataAuto(&VRam_Flag, data, offset, 4);
		ImportDataAuto(&CRam, data, offset, 256 * 2);
		
		// it's probably safer sync-wise to keep SRAM stuff in the savestate
		ImportDataAuto(&SRAM, data, offset, sizeof(SRAM));
		ImportDataAuto(&SRAM_Start, data, offset, 4);
		ImportDataAuto(&SRAM_End, data, offset, 4);
		ImportDataAuto(&SRAM_ON, data, offset, 4);
		ImportDataAuto(&SRAM_Write, data, offset, 4);
		ImportDataAuto(&SRAM_Custom, data, offset, 4);
		
		// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
		ImportDataAuto(&Bank_M68K, data, offset, 4);
		ImportDataAuto(&S68K_State, data, offset, 4);
		ImportDataAuto(&Z80_State, data, offset, 4);
		ImportDataAuto(&Last_BUS_REQ_Cnt, data, offset, 4);
		ImportDataAuto(&Last_BUS_REQ_St, data, offset, 4);
		ImportDataAuto(&Fake_Fetch, data, offset, 4);
		ImportDataAuto(&Game_Mode, data, offset, 4);
		ImportDataAuto(&CPU_Mode, data, offset, 4);
		ImportDataAuto(&CPL_M68K, data, offset, 4);
		ImportDataAuto(&CPL_S68K, data, offset, 4);
		ImportDataAuto(&CPL_Z80, data, offset, 4);
		ImportDataAuto(&Cycles_S68K, data, offset, 4);
		ImportDataAuto(&Cycles_M68K, data, offset, 4);
		ImportDataAuto(&Cycles_Z80, data, offset, 4);
		ImportDataAuto(&Gen_Mode, data, offset, 4);
		ImportDataAuto(&Gen_Version, data, offset, 4);
		ImportDataAuto(H_Counter_Table, data, offset, 512 * 2);
		ImportDataAuto(&VDP_Reg, data, offset, sizeof(VDP_Reg));
		ImportDataAuto(&Ctrl, data, offset, sizeof(Ctrl));
		
		ImportDataAuto(&Context_68K.cycles_needed, data, offset, 44);
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == GENESIS_STATE_LENGTH);
#endif		
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
	PSG_Save_State();
	
	// Copy the PSG state into the savestate buffer.
	for (int i = 0; i < 8; i++)
	{
		md_save.psg[i] = cpu_to_le16((uint16_t)PSG_Save[i]);
	}
	
#ifdef GENS_DEBUG_SAVESTATE
	int contextsize1 = main68k_GetContextSize();
	int contextsize2 = sizeof(Context_68K);
	assert(contextsize1 == contextsize2);
#endif
	
	main68k_GetContext(&Context_68K);
	
	// 68000 registers
	for (int i = 0; i < 8; i++)
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
	for (int i = 0; i < 24; i++)
	{
		md_save.vdp_reg[i] = (uint8_t)(*vdp_src);
		vdp_src++;
	}
	
	// CRAM and VSRAM.
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
	md_save.z80_reg.state_reset  = (((Z80_State & 4) >> 2) ^ 1);
	md_save.z80_reg.state_busreq = (((Z80_State & 2) >> 1) ^ 1);
	md_save.z80_reg.bank = cpu_to_le32(Bank_Z80);
	
	// Z80 RAM.
	memcpy(&md_save.z80_ram, &Ram_Z80, sizeof(md_save.z80_ram));
	
	// 68000 RAM.
	memcpy(&md_save.mc68000_ram, &Ram_68k, sizeof(md_save.mc68000_ram));
	cpu_to_be16_array(&md_save.mc68000_ram, sizeof(md_save.mc68000_ram));
	
	// VRAM.
	memcpy(&md_save.vram, &VRam, sizeof(md_save.vram));
	cpu_to_be16_array(&md_save.vram, sizeof(md_save.vram));
	
	// Copy md_save to the data variable.
	memcpy(&data[0], &md_save, sizeof(md_save));
	
	// TODO: This is from Gens Rerecording, and is probably not any use right now.
	/*
	data[0x22478]=unsigned char (FrameCount&0xFF);   //Modif
	data[0x22479]=unsigned char ((FrameCount>>8)&0xFF);   //Modif
	data[0x2247A]=unsigned char ((FrameCount>>16)&0xFF);   //Modif
	data[0x2247B]=unsigned char ((FrameCount>>24)&0xFF);   //Modif
	*/
	
	// everything after this should use this offset variable for ease of extensibility
	unsigned int offset = GENESIS_LENGTH_EX1; // Modif U. - got rid of about 12 KB of 00 bytes.
	
	// GENS v7 Savestate Additions
	// version 7 additions (version 6 additions deleted)
	//Modif N. - saving more stuff (added everything after this)
	
	unsigned char Reg_2[sizeof(ym2612_)];
	YM2612_Save_Full(Reg_2);
	ExportDataAuto(Reg_2, data, offset, sizeof(ym2612_)); // some Important parts of this weren't saved above
	
	PSG_Save_State_Full();
	ExportDataAuto(PSG_Save_Full, data, offset, sizeof(struct _psg));  // some Important parts of this weren't saved above
	
	ExportDataAuto(&M_Z80, data, offset, 0x5C); // some Important parts of this weren't saved above
	
	// BUG: The above ExportDataAuto call saves the PC and BasePC registers,
	// which are based on x86 memory locations and not emulated memory.
	// Set them to 0xDEADBEEF in the savestate, in big-endian format
	// so it's readable by a hex editor.
	
	// PC
	data[offset - 0x5C + 0x18] = 0xDE;
	data[offset - 0x5C + 0x19] = 0xAD;
	data[offset - 0x5C + 0x1A] = 0xBE;
	data[offset - 0x5C + 0x1B] = 0xEF;
	
	// BasePC
	data[offset - 0x5C + 0x40] = 0xDE;
	data[offset - 0x5C + 0x41] = 0xAD;
	data[offset - 0x5C + 0x42] = 0xBE;
	data[offset - 0x5C + 0x43] = 0xEF;
	
	ExportDataAuto(&M_Z80.RetIC, data, offset, 4); // not sure about the last two variables, might as well save them too
	ExportDataAuto(&M_Z80.IntAckC, data, offset, 4);
	
	ExportDataAuto(&Context_68K.dreg[0], data, offset, 86); // some Important parts of this weren't saved above
	
	ExportDataAuto(&Controller_1_State, data, offset, 448);   // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
	
	// apparently necessary
	ExportDataAuto(&VDP_Status, data, offset, 4);
	ExportDataAuto(&VDP_Int, data, offset, 4);
	ExportDataAuto(&VDP_Current_Line, data, offset, 4);
	ExportDataAuto(&VDP_Num_Lines, data, offset, 4);
	ExportDataAuto(&VDP_Num_Vis_Lines, data, offset, 4);
	ExportDataAuto(&DMAT_Length, data, offset, 4);
	ExportDataAuto(&DMAT_Type, data, offset, 4);
	//ExportDataAuto(&CRam_Flag, data, offset, 4);
	// TODO: LagCount for GENS ReRecording.
	//ExportDataAuto(&LagCount, data, offset, 4);
	offset += 4;
	ExportDataAuto(&VRam_Flag, data, offset, 4);
	ExportDataAuto(&CRam, data, offset, 256 * 2);
	
	// it's probably safer sync-wise to keep SRAM stuff in the savestate
	ExportDataAuto(&SRAM, data, offset, sizeof(SRAM));
	ExportDataAuto(&SRAM_Start, data, offset, 4);
	ExportDataAuto(&SRAM_End, data, offset, 4);
	ExportDataAuto(&SRAM_ON, data, offset, 4);
	ExportDataAuto(&SRAM_Write, data, offset, 4);
	ExportDataAuto(&SRAM_Custom, data, offset, 4);
	
	// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
	ExportDataAuto(&Bank_M68K, data, offset, 4);
	ExportDataAuto(&S68K_State, data, offset, 4);
	ExportDataAuto(&Z80_State, data, offset, 4);
	ExportDataAuto(&Last_BUS_REQ_Cnt, data, offset, 4);
	ExportDataAuto(&Last_BUS_REQ_St, data, offset, 4);
	ExportDataAuto(&Fake_Fetch, data, offset, 4);
	ExportDataAuto(&Game_Mode, data, offset, 4);
	ExportDataAuto(&CPU_Mode, data, offset, 4);
	ExportDataAuto(&CPL_M68K, data, offset, 4);
	ExportDataAuto(&CPL_S68K, data, offset, 4);
	ExportDataAuto(&CPL_Z80, data, offset, 4);
	ExportDataAuto(&Cycles_S68K, data, offset, 4);
	ExportDataAuto(&Cycles_M68K, data, offset, 4);
	ExportDataAuto(&Cycles_Z80, data, offset, 4);
	ExportDataAuto(&Gen_Mode, data, offset, 4);
	ExportDataAuto(&Gen_Version, data, offset, 4);
	ExportDataAuto(H_Counter_Table, data, offset, 512 * 2);
	ExportDataAuto(&VDP_Reg, data, offset, sizeof(VDP_Reg));
	ExportDataAuto(&Ctrl, data, offset, sizeof(Ctrl));
	
	ExportDataAuto(&Context_68K.cycles_needed, data, offset, 44);
	
	#ifdef GENS_DEBUG_SAVESTATE
		// assert that the final offset value equals our savestate size, otherwise we screwed up
		// if it fails, that probably means you have to add ((int)offset-(int)desiredoffset) to the last GENESIS_LENGTH_EX define
		assert(offset == GENESIS_STATE_LENGTH);
	#endif	
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

	ImportDataAuto(_32X_Rom, data, offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	ImportDataAuto(_32X_MSH2_Rom, data, offset, sizeof(_32X_MSH2_Rom));
	ImportDataAuto(_32X_SSH2_Rom, data, offset, sizeof(_32X_SSH2_Rom));

	M68K_32X_Mode();
	_32X_Set_FB();
	M68K_Set_32X_Rom_Bank();

	//Recalculate_Palettes();
	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
		_32X_VDP_CRam_Ajusted32[i] = _32X_Palette_32B[_32X_VDP_CRam[i]];
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
	if (strlen(ROM_Name) == 0)
		return "";
	return string(State_Dir) + string(ROM_Name) + ".srm";
}


/**
 * LoadSRAM(): Load the SRAM file.
 * @return 1 on success; 0 on error.
 */
int Savestate::LoadSRAM(void)
{
	FILE* SRAM_File = 0;
	
	memset(SRAM, 0, 64 * 1024);
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	string filename = GetSRAMFilename();
	if (filename.empty())
		return 0;
	if ((SRAM_File = fopen(filename.c_str(), "rb")) == 0)
		return 0;
	
	fread(SRAM, 64 * 1024, 1, SRAM_File);
	fclose(SRAM_File);
	
	string msg = "SRAM loaded from " + filename;
	vdraw_write_text(msg.c_str(), 2000);
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
	vdraw_write_text(dispText.c_str(), 2000);
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
	memset(Ram_Backup, 0, 8 * 1024);
	FormatSegaCD_BRAM(&Ram_Backup[0x1FC0]);
	
	// SegaCD cartridge memory.
	// TODO: Format the cartridge memory.
	memset(Ram_Backup_Ex, 0, 64 * 1024);
}


/**
 * GetSRAMFilename(): Get the filename of the BRAM file.
 * @return Filename of the BRAM file.
 */
inline string Savestate::GetBRAMFilename(void)
{
	if (strlen(ROM_Name) == 0)
		return "";
	return string(State_Dir) + string(ROM_Name) + ".brm";
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
	
	fread(Ram_Backup, 8 * 1024, 1, BRAM_File);
	fread(Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
	fclose(BRAM_File);
	
	string dispText = "BRAM loaded from " + filename;
	vdraw_write_text(dispText.c_str(), 2000);
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
	vdraw_write_text(dispText.c_str(), 2000);
	return 1;
}
