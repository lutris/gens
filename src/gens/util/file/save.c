/**
 * GENS: Save file handler.
 */

// Turn this on to enable savestate debugging.
#define GENS_DEBUG_SAVESTATE
#include <assert.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.h"

#include "emulator/gens.h"
#include "emulator/g_main.hpp"

// CPU
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"

// Memory
#include "gens_core/mem/mem_m68k.h"
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
#include "emulator/ui_proxy.hpp"
#include "ui-common.h"

// Byteswapping
#include "gens_core/misc/byteswap.h"

// Gens Rerecording
// fatal_mp3_error indicates an error occurred while reading an MP3 for a Sega CD game.
extern int fatal_mp3_error;	// cdda_mp3.c

// Various MP3 stuff, needed for Gens Rerecording
extern unsigned int Current_OUT_Pos, Current_OUT_Size;	// cdda_mp3.c
extern char preloaded_tracks [100], played_tracks_linear [101]; // added for synchronous MP3 code

#define DIR_SEPARATOR '/'

int Current_State = 0;
char State_Dir[GENS_PATH_MAX] = "";
char SRAM_Dir[GENS_PATH_MAX] = "";
char BRAM_Dir[GENS_PATH_MAX] = "";
unsigned char State_Buffer[MAX_STATE_FILE_LENGTH];


/**
 * Change_File_S(): Select a savestate to save.
 * @param Dest Destination buffer for the filename.
 * @param Dir ???
 * @return 1 if a file was selected.
 */
int Change_File_S(char *Dest, char *Dir)
{
	char filename[GENS_PATH_MAX];
	
	if (UI_SaveFile("Save Savetate", NULL /*Rom_Dir*/, SavestateFile, filename) != 0)
		return 0;
	
	// Filename selected for the savestate.
	strcpy(Dest, filename);
	return 1;
}


/**
 * Change_File_L(): Select a savestate to load.
 * @param Dest Destination buffer for the filename.
 * @param Dir ???
 * @return 1 if a file was selected.
 */
int Change_File_L(char *Dest, char *Dir)
{
	char filename[GENS_PATH_MAX];
	
	if (UI_OpenFile("Load Savestate", NULL /*Rom_Dir*/, SavestateFile, filename) != 0)
		return 0;
	
	// Filename selected for the savestate.
	strcpy(Dest, filename);
	return 1;
}


/**
 * Change_File_L_CD(): Select a CD image to load. (Used for multi-disc SegaCD games.)
 * @param Dest Destination buffer for the filename.
 * @param Dir ???
 * @return 1 if a file is selected.
 */
int Change_File_L_CD(char *Dest, char *Dir)
{
	char filename[GENS_PATH_MAX];
	
	if (UI_OpenFile("Load SegaCD Disc Image", NULL /*Rom_Dir*/, CDImage, filename) != 0)
		return 0;
	
	// Filename selected for the CD image.
	strcpy(Dest, filename);
	return 1;
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
		while ((i > 0) && (Dest[i] != DIR_SEPARATOR)) i--;
		if (!i) return 0;
		Dest[++i] = 0;
		g_free(filename);
		return 1;
	}
	return 0;
}
*/

FILE *
Get_State_File ()
{
  char Name[2048];
  char Ext[5] = ".gsX";

  //SetCurrentDirectory (Gens_Path);

  Ext[3] = '0' + Current_State;
  strcpy (Name, State_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, Ext);

  return fopen (Name, "rb");
}


void
Get_State_File_Name (char *name)
{
  char Ext[5] = ".gsX";

  //SetCurrentDirectory (Gens_Path);

  Ext[3] = '0' + Current_State;
  strcpy (name, State_Dir);
  strcat (name, Rom_Name);
  strcat (name, Ext);
}


/**
 * Load_State(): Load a savestate.
 * @param Name Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Load_State(char *Name)
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
	
	if ((f = fopen(Name, "rb")) == NULL)
		return 0;
	
	memset(buf, 0, len);
	if (fread(buf, 1, len, f))
	{
		//z80_Reset (&M_Z80); // Commented out in Gens Rerecording...
		/*
		main68k_reset();
		YM2612ResetChip(0);
		Reset_VDP();
		*/
		
		// Save functions updated from Gens Rerecording
		buf += Import_Genesis(buf);
		if (SegaCD_Started)
		{
			Import_SegaCD(buf);
			buf += SEGACD_LENGTH_EX;
		}
		if (_32X_Started)
		{
			Import_32X(buf);
			buf += G32X_LENGTH_EX;
		}
		
		/*
		// Commented out in Gens Rerecording...
		Flag_Clr_Scr = 1;
		CRam_Flag = 1;
		VRam_Flag = 1;
		*/
		
		sprintf(Str_Tmp, "STATE %d LOADED", Current_State);
		Put_Info(Str_Tmp, 2000);
	}
	
	fclose(f);
	
	return 1;
}


/**
 * Save_State(): Save a savestate.
 * @param Name Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Save_State(char *Name)
{
	if (Name == NULL)
		return 0;
	
	FILE *f;
	unsigned char *buf;
	int len;
	
	ice = 0;
	
	buf = State_Buffer;
	if ((f = fopen(Name, "wb")) == NULL)
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
	
	Export_Genesis(buf);
	buf += GENESIS_STATE_LENGTH;
	if (SegaCD_Started)
	{
		Export_SegaCD(buf);
		buf += SEGACD_LENGTH_EX;
	}
	if (_32X_Started)
	{
		Export_32X (buf);
		buf += G32X_LENGTH_EX;
	}
	
	fwrite(State_Buffer, 1, len, f);
	fclose(f);
	
	sprintf(Str_Tmp, "STATE %d SAVED", Current_State);
	Put_Info(Str_Tmp, 2000);
	
	return 1;
}


// See doc/genecyst_save_file_format.txt for information
// on the Genecyst save file format.


// Version field is initialized in Import_Genesis(),
// but is also used in Import_SegaCD() and Import_32X().
unsigned char Version;


// ImportData and ExportData functions from Gens ReRecording

//Modif N. - added ImportData and ExportData because the import/export code was getting a little hairy without these
// The main advantage to using these, besides less lines of code, is that
// you can replace ImportData with ExportData, without changing anything else in the arguments,
// to go from import code to export code.
inline void ImportData(void* into, const void* data, unsigned int offset, unsigned int numBytes)
{
	unsigned char* dst = (unsigned char*)into;
	const unsigned char* src = ((const unsigned char*)data) + offset;
	while(numBytes--) *dst++ = *src++;
}
inline void ExportData(const void* from, void* data, unsigned int offset, unsigned int numBytes)
{
	const unsigned char* src = (const unsigned char *) from;
	unsigned char* dst = ((unsigned char*) data) + offset;
#ifdef GENS_DEBUG_SAVESTATE
	while(numBytes--)
	{
		assert((*dst == 0 || *dst == 0xFF) && "error: saved over the same byte twice");
		*dst++ = *src++;
	}
#else
	while(numBytes--) *dst++ = *src++;
#endif
}
// versions that auto-increment the offset
inline void ImportDataAuto(void* into, const void* data, unsigned int *offset, unsigned int numBytes)
{
	ImportData(into, data, *offset, numBytes);
	*offset += numBytes;
}
inline void ExportDataAuto(const void* from, void* data, unsigned int *offset, unsigned int numBytes)
{
	ExportData(from, data, *offset, numBytes);
	*offset += numBytes;
}

// Save/load a 32-bit little-endian number.
inline int ImportNumber_le32(void* data, unsigned int *offset)
{
	// Converts the input data to an int via pointer casting.
	int tmp = *((int*)(&(((unsigned char*)data)[*offset])));
	*offset += 4;
	
#if GENS_BYTEORDER == GENS_BIG_ENDIAN
	// Savestate is in little-endian format. Byteswap the data.
	return (((tmp & 0xFF000000) >> 24) |
		((tmp & 0x00FF0000) >> 8) |
		((tmp & 0x0000FF00) << 8) |
		 (tmp & 0x000000FF) << 24);
#else /* GENS_BYTEORDER == GENS_LIL_ENDIAN */
	// Savestate is in little-endian format. Return the data as-is.
	return tmp;
#endif
}


/**
 * Import_Genesis(): Load Genesis data from a savestate. (Portions ported from Gens Rerecording.)
 * @param Data Savestate data.
 * @return Number of bytes read.
 */
int Import_Genesis (unsigned char *Data)
{
	unsigned char Reg_1[0x200], *src;
	int i;
	
	// Savestate V6 and V7 code from Gens Rerecording.
	
	/*
	// Commented out in Gens Rerecording.
	VDP_Int = 0;
	DMAT_Length = 0;
	*/
	
	// Length of the savestate.
	int len = GENESIS_STATE_LENGTH;
	
	// Get the savestate version.
	Version = Data[0x50];
	
	// Savestates earlier than Version 6 are shitty.
	if (Version < 6)
		len -= 0x10000;
	
	ImportData(CRam, Data, 0x112, 0x80);
	ImportData(VSRam, Data, 0x192, 0x50);
	ImportData(Ram_Z80, Data, 0x474, 0x2000);
	
	// 68000 RAM
	for (i = 0; i < 0x10000; i += 2)
	{
		Ram_68k[i + 0] = Data[i + 0x2478 + 1];
		Ram_68k[i + 1] = Data[i + 0x2478 + 0];
	}
	
	// VRAM
	for (i = 0; i < 0x10000; i += 2)
	{
		VRam[i + 0] = Data[i + 0x12478 + 1];
		VRam[i + 1] = Data[i + 0x12478 + 0];
	}
	
	ImportData(Reg_1, Data, 0x1E4, 0x200);
	YM2612_Restore(Reg_1);
	
	// Version 2, 3, and 4 save files
	if ((Version >= 2) && (Version < 4))
	{
		ImportData(&Ctrl, Data, 0x30, 7 * 4);
		
		Z80_State &= ~6;
		if (Data[0x440] & 1)
			Z80_State |= 2;
		if (Data[0x444] & 1)
			Z80_State |= 4;
		
		ImportData(&Bank_Z80, Data, 0x448, 4);
		
		ImportData(&PSG_Save, Data, 0x224B8, 8 * 4);
		PSG_Restore_State();
	}
	else if ((Version >= 4) || (Version == 0))
	{
		// New savestate version compatible with Kega
		Z80_State &= ~6;
		
		if (Version == 4)
		{
			M_Z80.IM = Data[0x437];
			M_Z80.IFF.b.IFF1 = (Data[0x438] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (Data[0x438] & 1) << 2;
			
			Z80_State |= (Data[0x439] & 1) << 1;
		}
		else
		{
			M_Z80.IM = 1;
			M_Z80.IFF.b.IFF1 = (Data[0x436] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (Data[0x436] & 1) << 2;
			
			Z80_State |= ((Data[0x439] & 1) ^ 1) << 1;
			Z80_State |= ((Data[0x438] & 1) ^ 1) << 2;
		}
		
		src = (unsigned char *) &Ctrl;
		for (i = 0; i < 7 * 4; i++)
			*src++ = 0;
		
		Write_VDP_Ctrl(Data[0x40] + (Data[0x41] << 8));
		Write_VDP_Ctrl(Data[0x42] + (Data[0x43] << 8));
		
		Ctrl.Flag = Data[0x44];
		Ctrl.DMA = (Data[0x45] & 1) << 2;
		Ctrl.Access = Data[0x46] + (Data[0x47] << 8); //Nitsuja added this
		Ctrl.Address = Data[0x48] + (Data[0x49] << 8);
		
		ImportData(&Bank_Z80, Data, 0x43C, 4);
		
		if (Version >= 4)
		{
			for (i = 0; i < 8; i++)
				PSG_Save[i] = Data[i * 2 + 0x60] + (Data[i * 2 + 0x61] << 8);
			PSG_Restore_State();
		}
	}

	z80_Set_AF (&M_Z80, Data[0x404] + (Data[0x405] << 8));
	M_Z80.AF.b.FXY = Data[0x406]; //Modif N [Gens Rerecording]
	M_Z80.BC.w.BC = Data[0x408] + (Data[0x409] << 8);
	M_Z80.DE.w.DE = Data[0x40C] + (Data[0x40D] << 8);
	M_Z80.HL.w.HL = Data[0x410] + (Data[0x411] << 8);
	M_Z80.IX.w.IX = Data[0x414] + (Data[0x415] << 8);
	M_Z80.IY.w.IY = Data[0x418] + (Data[0x419] << 8);
	z80_Set_PC(&M_Z80, Data[0x41C] + (Data[0x41D] << 8));
	M_Z80.SP.w.SP = Data[0x420] + (Data[0x421] << 8);
	z80_Set_AF2(&M_Z80, Data[0x424] + (Data[0x425] << 8));
	M_Z80.BC2.w.BC2 = Data[0x428] + (Data[0x429] << 8);
	M_Z80.DE2.w.DE2 = Data[0x42C] + (Data[0x42D] << 8);
	M_Z80.HL2.w.HL2 = Data[0x430] + (Data[0x431] << 8);
	M_Z80.I = Data[0x434] & 0xFF;
	
	// Gens Rerecording: This seems to only be used for movies (e.g. *.giz), so ignore it for now.
	//FrameCount = Data[0x22478] + (Data[0x22479] << 8) + (Data[0x2247A] << 16) + (Data[0x2247B] << 24);
	
	main68k_GetContext (&Context_68K);
	
	// VDP registers
	for (i = 0; i < 24; i++)
		Set_VDP_Reg(i, Data[0xFA + i]);
	
	// 68000 registers
	ImportData(&Context_68K.dreg[0], Data, 0x80, 8 * 2 * 4);
	ImportData(&Context_68K.pc, Data, 0xC8, 4);
	ImportData(&Context_68K.sr, Data, 0xD0, 2);
	
	if ((Version >= 3) || (Version == 0))
	{
		if (Data[0xD1] & 0x20)
		{
			// Supervisor
			ImportData(&Context_68K.asp, Data, 0xD2, 2);
		}
		else
		{
			// User
			ImportData(&Context_68K.asp, Data, 0xD6, 2);
		}
	}
	
	// NEW AND IMPROVED! GENS v6 and v7 savestate formats are here!
	// Ported from Gens Rerecording.
	unsigned int offset = GENESIS_LENGTH_EX1;
	if (Version == 6)
	{
		// GENS v6 savestate
		//Modif N. - saving more stuff (although a couple of these are saved above in a weird way that I don't trust)
		ImportDataAuto(&Context_68K.dreg, Data, &offset, 4*8);
		ImportDataAuto(&Context_68K.areg, Data, &offset, 4*8);
		ImportDataAuto(&Context_68K.asp, Data, &offset, 4);
		ImportDataAuto(&Context_68K.pc, Data, &offset, 4);
		ImportDataAuto(&Context_68K.odometer, Data, &offset, 4);
		ImportDataAuto(&Context_68K.interrupts, Data, &offset, 8);
		ImportDataAuto(&Context_68K.sr, Data, &offset, 2);
		ImportDataAuto(&Context_68K.contextfiller00, Data, &offset, 2);
		
		VDP_Reg.H_Int = ImportNumber_le32(Data, &offset);
		VDP_Reg.Set1 = ImportNumber_le32(Data, &offset);
		VDP_Reg.Set2 = ImportNumber_le32(Data, &offset);
		VDP_Reg.Pat_ScrA_Adr = ImportNumber_le32(Data, &offset);
		VDP_Reg.Pat_ScrA_Adr = ImportNumber_le32(Data, &offset);
		VDP_Reg.Pat_Win_Adr = ImportNumber_le32(Data, &offset);
		VDP_Reg.Pat_ScrB_Adr = ImportNumber_le32(Data, &offset);
		VDP_Reg.Spr_Att_Adr = ImportNumber_le32(Data, &offset);
		VDP_Reg.Reg6 = ImportNumber_le32(Data, &offset);
		VDP_Reg.BG_Color = ImportNumber_le32(Data, &offset);
		VDP_Reg.Reg8 = ImportNumber_le32(Data, &offset);
		VDP_Reg.Reg9 = ImportNumber_le32(Data, &offset);
		VDP_Reg.H_Int = ImportNumber_le32(Data, &offset);
		VDP_Reg.Set3 = ImportNumber_le32(Data, &offset);
		VDP_Reg.Set4 = ImportNumber_le32(Data, &offset);
		VDP_Reg.H_Scr_Adr = ImportNumber_le32(Data, &offset);
		VDP_Reg.Reg14 = ImportNumber_le32(Data, &offset);
		VDP_Reg.Auto_Inc = ImportNumber_le32(Data, &offset);
		VDP_Reg.Scr_Size = ImportNumber_le32(Data, &offset);
		VDP_Reg.Win_H_Pos = ImportNumber_le32(Data, &offset);
		VDP_Reg.Win_V_Pos = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length_L = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length_H = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Src_Adr_L = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Src_Adr_M = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Src_Adr_H = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Address = ImportNumber_le32(Data, &offset);
		
		Controller_1_Counter = ImportNumber_le32(Data, &offset);
		Controller_1_Delay = ImportNumber_le32(Data, &offset);
		Controller_1_State = ImportNumber_le32(Data, &offset);
		Controller_1_COM = ImportNumber_le32(Data, &offset);
		Controller_2_Counter = ImportNumber_le32(Data, &offset);
		Controller_2_Delay = ImportNumber_le32(Data, &offset);
		Controller_2_State = ImportNumber_le32(Data, &offset);
		Controller_2_COM = ImportNumber_le32(Data, &offset);
		Memory_Control_Status = ImportNumber_le32(Data, &offset);
		ImportDataAuto(&Cell_Conv_Tab, Data, &offset, 4);
		
		Controller_1_Type = ImportNumber_le32(Data, &offset);
		/* TODO: Make this stuff use bitfields.
		         For now, it's disabled, especially since v6 is rare.
		ImportDataAuto(&Controller_1_Up, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Down, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Left, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Right, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Start, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Mode, Data, &offset, 4);
		ImportDataAuto(&Controller_1_A, Data, &offset, 4);
		ImportDataAuto(&Controller_1_B, Data, &offset, 4);
		ImportDataAuto(&Controller_1_C, Data, &offset, 4);
		ImportDataAuto(&Controller_1_X, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Y, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Z, Data, &offset, 4);
		*/
		offset += 12*4;
		
		Controller_2_Type = ImportNumber_le32(Data, &offset);
		/* TODO: Make this stuff use bitfields.
		         For now, it's disabled, especially since v6 is rare.
		ImportDataAuto(&Controller_2_Up, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Down, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Left, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Right, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Start, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Mode, Data, &offset, 4);
		ImportDataAuto(&Controller_2_A, Data, &offset, 4);
		ImportDataAuto(&Controller_2_B, Data, &offset, 4);
		ImportDataAuto(&Controller_2_C, Data, &offset, 4);
		ImportDataAuto(&Controller_2_X, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Y, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Z, Data, &offset, 4);
		*/
		offset += 12*4;
		
		DMAT_Length = ImportNumber_le32(Data, &offset);
		DMAT_Type = ImportNumber_le32(Data, &offset);
		DMAT_Tmp = ImportNumber_le32(Data, &offset);
		VDP_Current_Line = ImportNumber_le32(Data, &offset);
		VDP_Num_Vis_Lines = ImportNumber_le32(Data, &offset);
		VDP_Num_Vis_Lines = ImportNumber_le32(Data, &offset);
		Bank_M68K = ImportNumber_le32(Data, &offset);
		S68K_State = ImportNumber_le32(Data, &offset);
		Z80_State = ImportNumber_le32(Data, &offset);
		Last_BUS_REQ_Cnt = ImportNumber_le32(Data, &offset);
		Last_BUS_REQ_St = ImportNumber_le32(Data, &offset);
		Fake_Fetch = ImportNumber_le32(Data, &offset);
		Game_Mode = ImportNumber_le32(Data, &offset);
		CPU_Mode = ImportNumber_le32(Data, &offset);
		CPL_M68K = ImportNumber_le32(Data, &offset);
		CPL_S68K = ImportNumber_le32(Data, &offset);
		CPL_Z80 = ImportNumber_le32(Data, &offset);
		Cycles_S68K = ImportNumber_le32(Data, &offset);
		Cycles_M68K = ImportNumber_le32(Data, &offset);
		Cycles_Z80 = ImportNumber_le32(Data, &offset);
		VDP_Status = ImportNumber_le32(Data, &offset);
		VDP_Int = ImportNumber_le32(Data, &offset);
		Ctrl.Write = ImportNumber_le32(Data, &offset);
		Ctrl.DMA_Mode = ImportNumber_le32(Data, &offset);
		Ctrl.DMA = ImportNumber_le32(Data, &offset);
		//CRam_Flag = ImportNumber_le32(Data, &offset); //Causes screen to blank
		//offset+=4;
		
		// TODO: LagCount from Gens Rerecording.
		//LagCount = ImportNumber_le32(Data, &offset);
		offset += 4;
		
		VRam_Flag = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length = ImportNumber_le32(Data, &offset);
		VDP_Reg.Auto_Inc = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length = ImportNumber_le32(Data, &offset);
		//ImportDataAuto(VRam, Data, &offset, 65536);
		ImportDataAuto(CRam, Data, &offset, 512);
		//ImportDataAuto(VSRam, Data, &offset, 64);
		ImportDataAuto(H_Counter_Table, Data, &offset, 512 * 2);
		//ImportDataAuto(Spr_Link, Data, &offset, 4*256);
		//extern int DMAT_Tmp, VSRam_Over;
		//ImportDataAuto(&DMAT_Tmp, Data, &offset, 4);
		//ImportDataAuto(&VSRam_Over, Data, &offset, 4);
		VDP_Reg.DMA_Length_L = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length_H = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Src_Adr_L = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Src_Adr_M = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Src_Adr_H = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Length = ImportNumber_le32(Data, &offset);
		VDP_Reg.DMA_Address = ImportNumber_le32(Data, &offset);
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == GENESIS_V6_STATE_LENGTH);
#endif
	}
	else if (Version >= 7)
	{
		// GENS v7 savestate
		unsigned char Reg_2[sizeof(ym2612_)];
		ImportDataAuto(Reg_2, Data, &offset, sizeof(ym2612_)); // some important parts of this weren't saved above
		YM2612_Restore_Full(Reg_2);
		
		ImportDataAuto(PSG_Save_Full, Data, &offset, sizeof(struct _psg)); // some important parts of this weren't saved above
		PSG_Restore_State_Full();
		
		// BUG: The Gens v7 savestate stores M_Z80.BasePC, which is a *real* pointer.
		// Also, it stores M_Z80.PC, which is *relative* to M_Z80.BasePC.
		// Workaround: Save M_Z80.BasePC and M_Z80.PC, and restore them after.
		// The PC is set correctly by the older savestate code above via z80_Set_PC().
		unsigned int oldBasePC = M_Z80.BasePC;
		unsigned int oldPC = M_Z80.PC.d;
		ImportDataAuto(&M_Z80, Data, &offset, 0x5C); // some important parts of this weren't saved above
		M_Z80.PC.d = oldPC;
		M_Z80.BasePC = oldBasePC;
		
		M_Z80.RetIC = ImportNumber_le32(Data, &offset); // not sure about the last two variables, might as well save them too
		M_Z80.IntAckC = ImportNumber_le32(Data, &offset);
		
		ImportDataAuto(&Context_68K.dreg[0], Data, &offset, 86); // some important parts of this weren't saved above
		
		ImportDataAuto(&Controller_1_State, Data, &offset, 448); // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
		
		// apparently necessary
		VDP_Status = ImportNumber_le32(Data, &offset);
		VDP_Int = ImportNumber_le32(Data, &offset);
		VDP_Current_Line = ImportNumber_le32(Data, &offset);
		VDP_Num_Lines = ImportNumber_le32(Data, &offset);
		VDP_Num_Vis_Lines = ImportNumber_le32(Data, &offset);
		DMAT_Length = ImportNumber_le32(Data, &offset);
		DMAT_Type = ImportNumber_le32(Data, &offset);
		//CRam_Flag = ImportNumber_le32(Data, &offset); //emulator flag which causes Gens not to update its draw palette, but doesn't affect sync state
		
		// TODO: LagCount for Gens Rerecording.
		//LagCount = ImportNumber_le32(Data, &offset);
		offset += 4;
		
		VRam_Flag = ImportNumber_le32(Data, &offset);
		ImportDataAuto(&CRam, Data, &offset, 256 * 2);
		
		// it's probably safer sync-wise to keep SRAM stuff in the savestate
		ImportDataAuto(&SRAM, Data, &offset, sizeof(SRAM));
		SRAM_Start = ImportNumber_le32(Data, &offset);
		SRAM_End = ImportNumber_le32(Data, &offset);
		SRAM_ON = ImportNumber_le32(Data, &offset);
		SRAM_Write = ImportNumber_le32(Data, &offset);
		SRAM_Custom = ImportNumber_le32(Data, &offset);
		
		// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
		Bank_M68K = ImportNumber_le32(Data, &offset);
		S68K_State = ImportNumber_le32(Data, &offset);
		Z80_State = ImportNumber_le32(Data, &offset);
		Last_BUS_REQ_Cnt = ImportNumber_le32(Data, &offset);
		Last_BUS_REQ_St = ImportNumber_le32(Data, &offset);
		Fake_Fetch = ImportNumber_le32(Data, &offset);
		Game_Mode = ImportNumber_le32(Data, &offset);
		CPU_Mode = ImportNumber_le32(Data, &offset);
		CPL_M68K = ImportNumber_le32(Data, &offset);
		CPL_S68K = ImportNumber_le32(Data, &offset);
		CPL_Z80 = ImportNumber_le32(Data, &offset);
		Cycles_S68K = ImportNumber_le32(Data, &offset);
		Cycles_M68K = ImportNumber_le32(Data, &offset);
		Cycles_Z80 = ImportNumber_le32(Data, &offset);
		Gen_Mode = ImportNumber_le32(Data, &offset);
		Gen_Version = ImportNumber_le32(Data, &offset);
		ImportDataAuto(H_Counter_Table, Data, &offset, 512 * 2);
		ImportDataAuto(&VDP_Reg, Data, &offset, sizeof(VDP_Reg));
		ImportDataAuto(&Ctrl, Data, &offset, sizeof(Ctrl));
		
		ImportDataAuto(&Context_68K.cycles_needed, Data, &offset, 44);
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == GENESIS_STATE_LENGTH);
#endif		
	}
	
	main68k_SetContext (&Context_68K);
	return len;
}


/**
 * Export_Genesis(): Save Genesis data to a savestate. (Portions ported from Gens Rerecording.)
 * @param Data Savestate data.
 */
void Export_Genesis(unsigned char *Data)
{
	// This savestate function uses the GENS v7 savestate format.
	// Note about GENS v7 savestate format:
	// - Plain MD savestates will work in older versions of GENS.
	// - Sega CD savestates won't work in older versions, but then again, older versions didn't properly support it.
	// - 32X savestates will *not* work in older versions of GENS. :(
	
	// from Gens Rerecording
	struct S68000CONTEXT Context_68K; // Modif N.: apparently no longer necessary but I'm leaving it here just to be safe: purposely shadows the global Context_68K variable with this local copy to avoid tampering with it while saving
	
	unsigned char Reg_1[0x200], *src;
	int i;
	
	// Be sure to finish DMA before save
	// [from Gens Rerecording] commented out; this may cause the saving to change the current state
	/*
	while (DMAT_Length)
		Update_DMA();
	*/
	
	// Genecyst savestate header
	Data[0x00] = 'G';
	Data[0x01] = 'S';
	Data[0x02] = 'T';
	Data[0x03] = 0x40;
	Data[0x04] = 0xE0;
	
	Data[0x50] = 7;		// Version
	Data[0x51] = 0;		// Gens
	
	// Save the PSG state.
	PSG_Save_State ();
	
	// Copy the PSG state into the savestate buffer.
	for (i = 0; i < 8; i++)
	{
		Data[0x60 + i * 2] = PSG_Save[i] & 0xFF;
		Data[0x61 + i * 2] = (PSG_Save[i] >> 8) & 0xFF;
	}
	
#ifdef GENS_DEBUG_SAVESTATE
	int contextsize1 = main68k_GetContextSize();
	int contextsize2 = sizeof(Context_68K);
	assert(contextsize1 == contextsize2);
#endif
	
	main68k_GetContext (&Context_68K);
	
	// 68000 registers
	ExportData(&Context_68K.dreg[0], Data, 0x80, 8 * 2 * 4);
	ExportData(&Context_68K.pc, Data, 0xC8, 4);
	ExportData(&Context_68K.sr, Data, 0xD0, 2);
	
	if (Context_68K.sr & 0x2000)
	{
		// Supervisor
		ExportData(&Context_68K.asp, Data, 0xD2, 4);
		ExportData(&Context_68K.areg[7], Data, 0xD6, 4);
	}
	else
	{
		// User
		ExportData(&Context_68K.asp, Data, 0xD6, 4);
		ExportData(&Context_68K.areg[7], Data, 0xD2, 4);
	}
	
	ExportData(&Ctrl.Data, Data, 0x40, 4);
	
	Data[0x44] = Ctrl.Flag;
	Data[0x45] = (Ctrl.DMA >> 2) & 1;
	
	// Gens Rerecording
	Data[0x46] = Ctrl.Access & 0xFF; //Nitsuja added this
	Data[0x47] = (Ctrl.Access >> 8) & 0xFF; //Nitsuja added this
	
	Data[0x48] = Ctrl.Address & 0xFF;
	Data[0x49] = (Ctrl.Address >> 8) & 0xFF;
	
	VDP_Reg.DMA_Length_L = VDP_Reg.DMA_Length & 0xFF;
	VDP_Reg.DMA_Length_H = (VDP_Reg.DMA_Length >> 8) & 0xFF;
	
	VDP_Reg.DMA_Src_Adr_L = VDP_Reg.DMA_Address & 0xFF;
	VDP_Reg.DMA_Src_Adr_M = (VDP_Reg.DMA_Address >> 8) & 0xFF;
	VDP_Reg.DMA_Src_Adr_H = (VDP_Reg.DMA_Address >> 16) & 0xFF;
	
	VDP_Reg.DMA_Src_Adr_H |= Ctrl.DMA_Mode & 0xC0;
	
	src = (unsigned char *) &(VDP_Reg.Set1);
	for (i = 0; i < 24; i++)
	{
		Data[0xFA + i] = *src;
		src += 4;
	}
	
	for (i = 0; i < 0x80; i++)
		Data[i + 0x112] = (CRam[i] & 0xFF);
	for (i = 0; i < 0x50; i++)
		Data[i + 0x192] = VSRam[i];
	
	YM2612_Save(Reg_1);
	for (i = 0; i < 0x200; i++)
		Data[i + 0x1E4] = Reg_1[i];
	
	Data[0x404] = (unsigned char) (z80_Get_AF (&M_Z80) & 0xFF);
	Data[0x405] = (unsigned char) (z80_Get_AF (&M_Z80) >> 8);
	Data[0x406] = (unsigned char) (M_Z80.AF.b.FXY & 0xFF); //Modif N
	Data[0x407] = (unsigned char) 0; //Modif N
	Data[0x408] = (unsigned char) (M_Z80.BC.w.BC & 0xFF);
	Data[0x409] = (unsigned char) (M_Z80.BC.w.BC >> 8);
	Data[0x40C] = (unsigned char) (M_Z80.DE.w.DE & 0xFF);
	Data[0x40D] = (unsigned char) (M_Z80.DE.w.DE >> 8);
	Data[0x410] = (unsigned char) (M_Z80.HL.w.HL & 0xFF);
	Data[0x411] = (unsigned char) (M_Z80.HL.w.HL >> 8);
	Data[0x414] = (unsigned char) (M_Z80.IX.w.IX & 0xFF);
	Data[0x415] = (unsigned char) (M_Z80.IX.w.IX >> 8);
	Data[0x418] = (unsigned char) (M_Z80.IY.w.IY & 0xFF);
	Data[0x419] = (unsigned char) (M_Z80.IY.w.IY >> 8);
	Data[0x41C] = (unsigned char) (z80_Get_PC (&M_Z80) & 0xFF);
	Data[0x41D] = (unsigned char) ((z80_Get_PC (&M_Z80) >> 8) & 0xFF);
	Data[0x420] = (unsigned char) (M_Z80.SP.w.SP & 0xFF);
	Data[0x421] = (unsigned char) (M_Z80.SP.w.SP >> 8);
	Data[0x424] = (unsigned char) (z80_Get_AF2 (&M_Z80) & 0xFF);
	Data[0x425] = (unsigned char) (z80_Get_AF2 (&M_Z80) >> 8);
	Data[0x428] = (unsigned char) (M_Z80.BC2.w.BC2 & 0xFF);
	Data[0x429] = (unsigned char) (M_Z80.BC2.w.BC2 >> 8);
	Data[0x42C] = (unsigned char) (M_Z80.DE2.w.DE2 & 0xFF);
	Data[0x42D] = (unsigned char) (M_Z80.DE2.w.DE2 >> 8);
	Data[0x430] = (unsigned char) (M_Z80.HL2.w.HL2 & 0xFF);
	Data[0x431] = (unsigned char) (M_Z80.HL2.w.HL2 >> 8);
	Data[0x434] = (unsigned char) (M_Z80.I);
	Data[0x436] = (unsigned char) (M_Z80.IFF.b.IFF1 >> 2);
	
	Data[0x438] = (unsigned char) (((Z80_State & 4) >> 2) ^ 1);
	Data[0x439] = (unsigned char) (((Z80_State & 2) >> 1) ^ 1);
	
	ExportData(&Bank_Z80, Data, 0x43C, 4);
	
	for (i = 0; i < 0x2000; i++)
		Data[i + 0x474] = Ram_Z80[i];
	
	// 68000 RAM
	for (i = 0; i < 0x10000; i += 2)
	{
		Data[i + 0x2478 + 1] = Ram_68k[i + 0];
		Data[i + 0x2478 + 0] = Ram_68k[i + 1];
	}
	
	// VRAM
	for (i = 0; i < 0x10000; i += 2)
	{
		Data[i + 0x12478 + 1] = VRam[i + 0];
		Data[i + 0x12478 + 0] = VRam[i + 1];
	}
	
	// TODO: This is from Gens Rerecording, and is probably not any use right now.
	/*
	Data[0x22478]=unsigned char (FrameCount&0xFF);   //Modif
	Data[0x22479]=unsigned char ((FrameCount>>8)&0xFF);   //Modif
	Data[0x2247A]=unsigned char ((FrameCount>>16)&0xFF);   //Modif
	Data[0x2247B]=unsigned char ((FrameCount>>24)&0xFF);   //Modif
	*/
	
	// everything after this should use this offset variable for ease of extensibility
	unsigned int offset = GENESIS_LENGTH_EX1; // Modif U. - got rid of about 12 KB of 00 bytes.
	
	// GENS v7 Savestate Additions
	// version 7 additions (version 6 additions deleted)
	//Modif N. - saving more stuff (added everything after this)
	
	unsigned char Reg_2[sizeof(ym2612_)];
	YM2612_Save_Full(Reg_2);
	ExportDataAuto(Reg_2, Data, &offset, sizeof(ym2612_)); // some important parts of this weren't saved above
	
	PSG_Save_State_Full();
	ExportDataAuto(PSG_Save_Full, Data, &offset, sizeof(struct _psg));  // some important parts of this weren't saved above
	
	ExportDataAuto(&M_Z80, Data, &offset, 0x5C); // some important parts of this weren't saved above
	
	// BUG: The above ExportDataAuto call saves the PC and BasePC registers,
	// which are based on x86 memory locations and not emulated memory.
	// Set them to 0xDEADBEEF in the savestate, in big-endian format
	// so it's readable by a hex editor.
	
	// PC
	Data[offset - 0x5C + 0x18] = 0xDE;
	Data[offset - 0x5C + 0x19] = 0xAD;
	Data[offset - 0x5C + 0x1A] = 0xBE;
	Data[offset - 0x5C + 0x1B] = 0xEF;
	
	// BasePC
	Data[offset - 0x5C + 0x40] = 0xDE;
	Data[offset - 0x5C + 0x41] = 0xAD;
	Data[offset - 0x5C + 0x42] = 0xBE;
	Data[offset - 0x5C + 0x43] = 0xEF;
	
	ExportDataAuto(&M_Z80.RetIC, Data, &offset, 4); // not sure about the last two variables, might as well save them too
	ExportDataAuto(&M_Z80.IntAckC, Data, &offset, 4);
	
	ExportDataAuto(&Context_68K.dreg[0], Data, &offset, 86); // some important parts of this weren't saved above
	
	ExportDataAuto(&Controller_1_State, Data, &offset, 448);   // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
	
	// apparently necessary
	ExportDataAuto(&VDP_Status, Data, &offset, 4);
	ExportDataAuto(&VDP_Int, Data, &offset, 4);
	ExportDataAuto(&VDP_Current_Line, Data, &offset, 4);
	ExportDataAuto(&VDP_Num_Lines, Data, &offset, 4);
	ExportDataAuto(&VDP_Num_Vis_Lines, Data, &offset, 4);
	ExportDataAuto(&DMAT_Length, Data, &offset, 4);
	ExportDataAuto(&DMAT_Type, Data, &offset, 4);
	//ExportDataAuto(&CRam_Flag, Data, &offset, 4);
	// TODO: LagCount for GENS ReRecording.
	//ExportDataAuto(&LagCount, Data, &offset, 4);
	offset += 4;
	ExportDataAuto(&VRam_Flag, Data, &offset, 4);
	ExportDataAuto(&CRam, Data, &offset, 256 * 2);
	
	// it's probably safer sync-wise to keep SRAM stuff in the savestate
	ExportDataAuto(&SRAM, Data, &offset, sizeof(SRAM));
	ExportDataAuto(&SRAM_Start, Data, &offset, 4);
	ExportDataAuto(&SRAM_End, Data, &offset, 4);
	ExportDataAuto(&SRAM_ON, Data, &offset, 4);
	ExportDataAuto(&SRAM_Write, Data, &offset, 4);
	ExportDataAuto(&SRAM_Custom, Data, &offset, 4);
	
	// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
	ExportDataAuto(&Bank_M68K, Data, &offset, 4);
	ExportDataAuto(&S68K_State, Data, &offset, 4);
	ExportDataAuto(&Z80_State, Data, &offset, 4);
	ExportDataAuto(&Last_BUS_REQ_Cnt, Data, &offset, 4);
	ExportDataAuto(&Last_BUS_REQ_St, Data, &offset, 4);
	ExportDataAuto(&Fake_Fetch, Data, &offset, 4);
	ExportDataAuto(&Game_Mode, Data, &offset, 4);
	ExportDataAuto(&CPU_Mode, Data, &offset, 4);
	ExportDataAuto(&CPL_M68K, Data, &offset, 4);
	ExportDataAuto(&CPL_S68K, Data, &offset, 4);
	ExportDataAuto(&CPL_Z80, Data, &offset, 4);
	ExportDataAuto(&Cycles_S68K, Data, &offset, 4);
	ExportDataAuto(&Cycles_M68K, Data, &offset, 4);
	ExportDataAuto(&Cycles_Z80, Data, &offset, 4);
	ExportDataAuto(&Gen_Mode, Data, &offset, 4);
	ExportDataAuto(&Gen_Version, Data, &offset, 4);
	ExportDataAuto(H_Counter_Table, Data, &offset, 512 * 2);
	ExportDataAuto(&VDP_Reg, Data, &offset, sizeof(VDP_Reg));
	ExportDataAuto(&Ctrl, Data, &offset, sizeof(Ctrl));
	
	ExportDataAuto(&Context_68K.cycles_needed, Data, &offset, 44);
	
	#ifdef GENS_DEBUG_SAVESTATE
		// assert that the final offset value equals our savestate size, otherwise we screwed up
		// if it fails, that probably means you have to add ((int)offset-(int)desiredoffset) to the last GENESIS_LENGTH_EX define
		assert(offset == GENESIS_STATE_LENGTH);
	#endif	
}


/**
 * Import_SegaCD(): Load Sega CD data from a savestate. (Ported from Gens Rerecording.)
 * @param Data Savestate data.
 */
void Import_SegaCD(unsigned char *Data)
{
	struct S68000CONTEXT Context_sub68K;
	unsigned char *src;
	int i, j;
	
	sub68k_GetContext(&Context_sub68K);
	
	//sub68K bit goes here
	ImportData(&(Context_sub68K.dreg[0]), Data, 0x0, 8 * 4);
	ImportData(&(Context_sub68K.areg[0]), Data, 0x20, 8 * 4);
	ImportData(&(Context_sub68K.pc), Data, 0x48, 4);
	ImportData(&(Context_sub68K.sr), Data, 0x50, 2);
	
	if(Data[0x51] & 0x20)
	{
		// Supervisor
		ImportData(&Context_68K.asp, Data, 0x52, 4);
	}
	else
	{
		// User
		ImportData(&Context_68K.asp, Data, 0x56, 4);
	}
	
	
	ImportData(&Context_sub68K.odometer, Data, 0x5A, 4);
	ImportData(Context_sub68K.interrupts, Data, 0x60, 8);
	ImportData(&Ram_Word_State, Data, 0x6C, 4);
	
	//here ends sub68k bit
	
	//sub68k_SetContext(&Context_sub68K); // Modif N. -- moved to later
	
	//PCM Chip Load
	ImportData(&PCM_Chip.Rate, Data, 0x100, 4);
	ImportData(&PCM_Chip.Enable, Data, 0x104, 4);
	ImportData(&PCM_Chip.Cur_Chan, Data, 0x108, 4);
	ImportData(&PCM_Chip.Bank, Data, 0x10C, 4);
	
	for (j = 0; j < 8; j++)
	{
		ImportData(&PCM_Chip.Channel[j].ENV, Data, 0x120 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].PAN, Data, 0x124 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].MUL_L, Data, 0x128 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].MUL_R, Data, 0x12C + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].St_Addr, Data, 0x130 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Loop_Addr, Data, 0x134 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Addr, Data, 0x138 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Step, Data, 0x13C + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Step_B, Data, 0x140 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Enable, Data, 0x144 + (j * 4 * 11), 4);
		ImportData(&PCM_Chip.Channel[j].Data, Data, 0x148 + (j * 4 * 11), 4);
	}
	
	//End PCM Chip Load
	
	//Init_RS_GFX(); //purge old GFX data
	//GFX State Load
	ImportData(&(Rot_Comp.Stamp_Size), Data, 0x300, 4);
	ImportData(&(Rot_Comp.Stamp_Map_Adr), Data, 0x304, 4);
	ImportData(&(Rot_Comp.IB_V_Cell_Size), Data, 0x308, 4);
	ImportData(&(Rot_Comp.IB_Adr), Data, 0x30C, 4);
	ImportData(&(Rot_Comp.IB_Offset), Data, 0x310, 4);
	ImportData(&(Rot_Comp.IB_H_Dot_Size), Data, 0x314, 4);
	ImportData(&(Rot_Comp.IB_V_Dot_Size), Data, 0x318, 4);
	ImportData(&(Rot_Comp.Vector_Adr), Data, 0x31C, 4);
	ImportData(&(Rot_Comp.Rotation_Running), Data, 0x320, 4);
	//End GFX State Load
	
	//gate array bit
	ImportData(&COMM.Flag, Data, 0x0500, 4);
	src = (unsigned char *) &COMM.Command;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			*src++ = Data[i + 0x0504 + j];
		}
	}
	src = (unsigned char *) &COMM.Status;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			*src++ = Data[i + 0x0514 + j];
		}
	}
	ImportData(&Memory_Control_Status, Data, 0x0524, 4);
	ImportData(&Init_Timer_INT3, Data, 0x0528, 4);
	ImportData(&Timer_INT3, Data, 0x052C, 4);
	ImportData(&Timer_Step, Data, 0x0530, 4);
	ImportData(&Int_Mask_S68K, Data, 0x0534, 4);
	ImportData(&Font_COLOR, Data, 0x0538, 4);
	ImportData(&Font_BITS, Data, 0x053C, 4);
	ImportData(&CD_Access_Timer, Data, 0x0540, 4);
	ImportData(&SCD.Status_CDC, Data, 0x0544, 4);
	ImportData(&SCD.Status_CDD, Data, 0x0548, 4);
	ImportData(&SCD.Cur_LBA, Data, 0x054C, 4);
	ImportData(&SCD.Cur_Track, Data, 0x0550, 4);
	ImportData(&S68K_Mem_WP, Data, 0x0554, 4);
	ImportData(&S68K_Mem_PM, Data, 0x0558, 4);
	// More goes here when found
	//here ends gate array bit
	
	//Misc Status Flags
	ImportData(&Ram_Word_State, Data, 0xF00, 4); //For determining 1M or 2M
	ImportData(&LED_Status, Data, 0xF08, 4); //So the LED shows up properly
	//Word RAM state
	
	//Prg RAM
	ImportData(Ram_Prg, Data, 0x1000, 0x80000);
	
	//Word RAM
	if (Ram_Word_State >= 2)
		ImportData(Ram_Word_1M, Data, 0x81000, 0x40000); //1M mode
	else
		ImportData(Ram_Word_2M, Data, 0x81000, 0x40000); //2M mode
		//ImportData(Ram_Word_2M, Data, 0x81000, 0x40000); //2M mode
	//Word RAM end
	
	ImportData(Ram_PCM, Data, 0xC1000, 0x10000); //PCM RAM
	
	//CDD & CDC Data
	//CDD
	unsigned int CDD_Data[8]; //makes an array for reading CDD unsigned int Data into
	for (j = 0; j < 8; j++)
	{
		ImportData(&CDD_Data[j], Data, 0xD1000  + (4 * j), 4);
	}
	for(i = 0; i < 10; i++)
		CDD.Rcv_Status[i] = Data[0xD1020 + i];
	for(i = 0; i < 10; i++)
		CDD.Trans_Comm[i] = Data[0xD102A + i];
	CDD.Fader = CDD_Data[0];
	CDD.Control = CDD_Data[1];
	CDD.Cur_Comm = CDD_Data[2];
	CDD.Status = CDD_Data[3];
	CDD.Minute = CDD_Data[4];
	CDD.Seconde = CDD_Data[5];
	CDD.Frame = CDD_Data[6];
	CDD.Ext = CDD_Data[7];
	if (CDD.Status & PLAYING)
		if (IsAsyncAllowed()) // Modif N. -- disabled call to resume in synchronous mode (it's unnecessary there and can cause desyncs)
			FILE_Play_CD_LBA(0); // and replaced Resume_CDD_c7 with a call to preload the (new) current MP3 when a savestate is loaded (mainly for sound quality and camhack stability reasons), or do nothing if it's not an MP3
	//CDD end
	
	//CDC
	ImportData(&CDC.RS0, Data, 0xD1034, 4);
	ImportData(&CDC.RS1, Data, 0xD1038, 4);
	ImportData(&CDC.Host_Data, Data, 0xD103C, 4);
	ImportData(&CDC.DMA_Adr, Data, 0xD1040, 4);
	ImportData(&CDC.Stop_Watch, Data, 0xD1044, 4);
	ImportData(&CDC.COMIN, Data, 0xD1048, 4);
	ImportData(&CDC.IFSTAT, Data, 0xD104C, 4);
	ImportData(&CDC.DBC.N, Data, 0xD1050, 4);
	ImportData(&CDC.DAC.N, Data, 0xD1054, 4);
	ImportData(&CDC.HEAD.N, Data, 0xD1058, 4);
	ImportData(&CDC.PT.N, Data, 0xD105C, 4);
	ImportData(&CDC.WA.N, Data, 0xD1060, 4);
	ImportData(&CDC.STAT.N, Data, 0xD1064, 4);
	ImportData(&CDC.SBOUT, Data, 0xD1068, 4);
	ImportData(&CDC.IFCTRL, Data, 0xD106C, 4);
	ImportData(&CDC.CTRL.N, Data, 0xD1070, 4);
	ImportData(CDC.Buffer, Data, 0xD1074, ((32 * 1024 * 2) + 2352)); //Modif N. - added the *2 because the buffer appears to be that large
	//CDC end
	//CDD & CDC Data end
	
	if (Version >= 7)
	{
		//Modif N. - extra stuff added to save/set for synchronization reasons
		// I'm not sure how much of this really needs to be saved, should check it sometime
		unsigned int offset = SEGACD_LENGTH_EX1;
		
		ImportDataAuto(&File_Add_Delay, Data, &offset, 4);
//		ImportDataAuto(CD_Audio_Buffer_L, Data, &offset, 4*8192); // removed, seems to be unnecessary
//		ImportDataAuto(CD_Audio_Buffer_R, Data, &offset, 4*8192); // removed, seems to be unnecessary
		ImportDataAuto(&CD_Audio_Buffer_Read_Pos, Data, &offset, 4);
		ImportDataAuto(&CD_Audio_Buffer_Write_Pos, Data, &offset, 4);
		ImportDataAuto(&CD_Audio_Starting, Data, &offset, 4);
		ImportDataAuto(&CD_Present, Data, &offset, 4);
		ImportDataAuto(&CD_Load_System, Data, &offset, 4);
		ImportDataAuto(&CD_Timer_Counter, Data, &offset, 4);
		ImportDataAuto(&CDD_Complete, Data, &offset, 4);
		ImportDataAuto(&track_number, Data, &offset, 4);
		ImportDataAuto(&CD_timer_st, Data, &offset, 4);
		ImportDataAuto(&CD_LBA_st, Data, &offset, 4);
		ImportDataAuto(&CDC_Decode_Reg_Read, Data, &offset, 4);
		
		ImportDataAuto(&SCD, Data, &offset, sizeof(SCD));
		//ImportDataAuto(&CDC, Data, &offset, sizeof(CDC)); // removed, seems unnecessary/redundant
		ImportDataAuto(&CDD, Data, &offset, sizeof(CDD));
		ImportDataAuto(&COMM, Data, &offset, sizeof(COMM));
		
		ImportDataAuto(Ram_Backup, Data, &offset, sizeof(Ram_Backup));
		ImportDataAuto(Ram_Backup_Ex, Data, &offset, sizeof(Ram_Backup_Ex));
		
		ImportDataAuto(&Rot_Comp, Data, &offset, sizeof(Rot_Comp));
		ImportDataAuto(&Stamp_Map_Adr, Data, &offset, 4);
		ImportDataAuto(&Buffer_Adr, Data, &offset, 4);
		ImportDataAuto(&Vector_Adr, Data, &offset, 4);
		ImportDataAuto(&Jmp_Adr, Data, &offset, 4);
		ImportDataAuto(&Float_Part, Data, &offset, 4);
		ImportDataAuto(&Draw_Speed, Data, &offset, 4);
		ImportDataAuto(&XS, Data, &offset, 4);
		ImportDataAuto(&YS, Data, &offset, 4);
		ImportDataAuto(&DXS, Data, &offset, 4);
		ImportDataAuto(&DYS, Data, &offset, 4);
		ImportDataAuto(&XD, Data, &offset, 4);
		ImportDataAuto(&YD, Data, &offset, 4);
		ImportDataAuto(&XD_Mul, Data, &offset, 4);
		ImportDataAuto(&H_Dot, Data, &offset, 4);
		
		ImportDataAuto(&Context_sub68K.cycles_needed, Data, &offset, 44);
		ImportDataAuto(&Rom_Data[0x72], Data, &offset, 2); 	//Sega CD games can overwrite the low two bytes of the Horizontal Interrupt vector
		
		ImportDataAuto(&fatal_mp3_error, Data, &offset, 4);
		ImportDataAuto(&Current_OUT_Pos, Data, &offset, 4);
		ImportDataAuto(&Current_OUT_Size, Data, &offset, 4);
		ImportDataAuto(&Track_Played, Data, &offset, 1);
		ImportDataAuto(played_tracks_linear, Data, &offset, 100);
		//ImportDataAuto(&Current_IN_Pos, Data, &offset, 4)? // don't save this; bad things happen
		
#ifdef GENS_DEBUG_SAVESTATE
		assert(offset == SEGACD_LENGTH_EX);
#endif
	}
	
	sub68k_SetContext(&Context_sub68K); // Modif N. -- moved here from earlier in the function
	
	M68K_Set_Prg_Ram();
	MS68K_Set_Word_Ram();	
}


/**
 * Export_SegaCD(): Save Sega CD data to a savestate. (Uses GENS v7 format from Gens Rerecording.)
 * @param Data Savestate data.
 */
void Export_SegaCD (unsigned char *Data)
{
	struct S68000CONTEXT Context_sub68K;
	unsigned char *src;
	int i,j;
	
	sub68k_GetContext(&Context_sub68K);
	
	//sub68K bit goes here
	ExportData(&Context_sub68K.dreg[0], Data, 0x0, 8 * 4);
	ExportData(&Context_sub68K.areg[0], Data, 0x20, 8 * 4);
	ExportData(&Context_sub68K.pc, Data, 0x48, 4);
	ExportData(&Context_sub68K.sr, Data, 0x50, 2);
	
	if (Context_sub68K.sr & 0x2000)
	{
		ExportData(&Context_sub68K.asp, Data, 0x52, 4);
		ExportData(&Context_sub68K.areg[7], Data, 0x56, 4);
	}
	else
	{
		ExportData(&Context_sub68K.asp, Data, 0x56, 4);
		ExportData(&Context_sub68K.areg[7], Data, 0x52, 4);
	}
	
	ExportData(&Context_sub68K.odometer, Data, 0x5A, 4);
	ExportData(Context_sub68K.interrupts, Data, 0x60, 8);
	ExportData(&Ram_Word_State, Data, 0x6C, 4);
	//here ends sub68k bit
	
	//PCM Chip dump
	ExportData(&PCM_Chip.Rate, Data, 0x100, 4);
	ExportData(&PCM_Chip.Enable, Data, 0x104, 4);
	ExportData(&PCM_Chip.Cur_Chan, Data, 0x108, 4);
	ExportData(&PCM_Chip.Bank, Data, 0x10C, 4);
	
	for (j = 0; j < 8; j++)
	{
		ExportData(&PCM_Chip.Channel[j].ENV, Data, 0x120 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].PAN, Data, 0x124 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].MUL_L, Data, 0x128 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].MUL_R, Data, 0x12C + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].St_Addr, Data, 0x130 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Loop_Addr, Data, 0x134 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Addr, Data, 0x138 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Step, Data, 0x13C + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Step_B, Data, 0x140 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Enable, Data, 0x144 + (j * 4 * 11), 4);
		ExportData(&PCM_Chip.Channel[j].Data, Data, 0x148 + (j * 4 * 11), 4);
	}
	//End PCM Chip Dump

	//GFX State Dump
	ExportData(&Rot_Comp.Stamp_Size, Data, 0x300, 4);
	ExportData(&Rot_Comp.Stamp_Map_Adr, Data, 0x304, 4);
	ExportData(&Rot_Comp.IB_V_Cell_Size, Data, 0x308, 4);
	ExportData(&Rot_Comp.IB_Adr, Data, 0x30C, 4);
	ExportData(&Rot_Comp.IB_Offset, Data, 0x310, 4);
	ExportData(&Rot_Comp.IB_H_Dot_Size, Data, 0x314, 4);
	ExportData(&Rot_Comp.IB_V_Dot_Size, Data, 0x318, 4);
	ExportData(&Rot_Comp.Vector_Adr, Data, 0x31C, 4);
	ExportData(&Rot_Comp.Rotation_Running, Data, 0x320, 4);
	//End GFX State Dump

	//gate array bit
	ExportData(&COMM.Flag, Data, 0x0500, 4);
	src = (unsigned char *) &COMM.Command;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			Data[i + 0x0504 + j] = *src++;
		}
	}
	src = (unsigned char *) &COMM.Status;
	for (j = 0; j < 8 * 2; j += 2)
	{
		for (i = 0; i < 2; i++)
		{
			Data[i + 0x0514 + j] = *src++;
		}
	}
	ExportData(&Memory_Control_Status, Data, 0x0524, 4);
	ExportData(&Init_Timer_INT3, Data, 0x0528, 4);
	ExportData(&Timer_INT3, Data, 0x052C, 4);
	ExportData(&Timer_Step, Data, 0x0530, 4);
	ExportData(&Int_Mask_S68K, Data, 0x0534, 4);
	ExportData(&Font_COLOR, Data, 0x0538, 4);
	ExportData(&Font_BITS, Data, 0x053C, 4);
	ExportData(&CD_Access_Timer, Data, 0x0540, 4);
	ExportData(&SCD.Status_CDC, Data, 0x0544, 4);
	ExportData(&SCD.Status_CDD, Data, 0x0548, 4);
	ExportData(&SCD.Cur_LBA, Data, 0x054C, 4);
	ExportData(&SCD.Cur_Track, Data, 0x0550, 4);
	ExportData(&S68K_Mem_WP, Data, 0x0554, 4);
	ExportData(&S68K_Mem_PM, Data, 0x0558, 4);
	// More goes here When found
	//here ends gate array bit
	
	//Misc Status Flags
	ExportData(&Ram_Word_State, Data, 0xF00, 4); //For determining 1M or 2M
	ExportData(&LED_Status, Data, 0xF08, 4); //So the LED shows up properly
	//Word RAM state
	
	//Prg RAM
	ExportData(Ram_Prg, Data, 0x1000, 0x80000);
	
	//Word RAM
	if (Ram_Word_State >= 2)
		ExportData(Ram_Word_1M, Data, 0x81000, 0x40000); //1M mode
	else
		ExportData(Ram_Word_2M, Data, 0x81000, 0x40000); //2M mode
	//Word RAM end
	
	ExportData(Ram_PCM, Data, 0xC1000, 0x10000); //PCM RAM
	
	//CDD & CDC Data
	//CDD
	unsigned int CDD_src[8] = {CDD.Fader, CDD.Control, CDD.Cur_Comm, CDD.Status,
				   CDD.Minute, CDD.Seconde, CDD.Frame, CDD.Ext}; // Makes an array for easier loop construction
	for (j = 0; j < 8; j++)
	{
		ExportData(&CDD_src[j], Data, 0xD1000  + (4 * j), 4);
	}
	for(i = 0; i < 10; i++)
		Data[0xD1020 + i] = CDD.Rcv_Status[i];
	for(i = 0; i < 10; i++)
		Data[0xD102A + i] = CDD.Trans_Comm[i];
	//CDD end
	
	//CDC
	ExportData(&CDC.RS0, Data, 0xD1034, 4);
	ExportData(&CDC.RS1, Data, 0xD1038, 4);
	ExportData(&CDC.Host_Data, Data, 0xD103C, 4);
	ExportData(&CDC.DMA_Adr, Data, 0xD1040, 4);
	ExportData(&CDC.Stop_Watch, Data, 0xD1044, 4);
	ExportData(&CDC.COMIN, Data, 0xD1048, 4);
	ExportData(&CDC.IFSTAT, Data, 0xD104C, 4);
	ExportData(&CDC.DBC.N, Data, 0xD1050, 4);
	ExportData(&CDC.DAC.N, Data, 0xD1054, 4);
	ExportData(&CDC.HEAD.N, Data, 0xD1058, 4);
	ExportData(&CDC.PT.N, Data, 0xD105C, 4);
	ExportData(&CDC.WA.N, Data, 0xD1060, 4);
	ExportData(&CDC.STAT.N, Data, 0xD1064, 4);
	ExportData(&CDC.SBOUT, Data, 0xD1068, 4);
	ExportData(&CDC.IFCTRL, Data, 0xD106C, 4);
	ExportData(&CDC.CTRL.N, Data, 0xD1070, 4);
	ExportData(CDC.Buffer, Data, 0xD1074, ((32 * 1024 * 2) + 2352)); //Modif N. - added the *2 because the buffer appears to be that large
	//CDC end
	//CDD & CDC Data end
	
	//Modif N. - extra stuff added to save/set for synchronization reasons
	// I'm not sure how much of this really needs to be saved, should check it sometime
	
	unsigned int offset = SEGACD_LENGTH_EX1;
	
	ExportDataAuto(&File_Add_Delay, Data, &offset, 4);
//	ExportDataAuto(CD_Audio_Buffer_L, Data, &offset, 4*8192); // removed, seems to be unnecessary
//	ExportDataAuto(CD_Audio_Buffer_R, Data, &offset, 4*8192); // removed, seems to be unnecessary
	ExportDataAuto(&CD_Audio_Buffer_Read_Pos, Data, &offset, 4);
	ExportDataAuto(&CD_Audio_Buffer_Write_Pos, Data, &offset, 4);
	ExportDataAuto(&CD_Audio_Starting, Data, &offset, 4);
	ExportDataAuto(&CD_Present, Data, &offset, 4);
	ExportDataAuto(&CD_Load_System, Data, &offset, 4);
	ExportDataAuto(&CD_Timer_Counter, Data, &offset, 4);
	ExportDataAuto(&CDD_Complete, Data, &offset, 4);
	ExportDataAuto(&track_number, Data, &offset, 4);
	ExportDataAuto(&CD_timer_st, Data, &offset, 4);
	ExportDataAuto(&CD_LBA_st, Data, &offset, 4);
	ExportDataAuto(&CDC_Decode_Reg_Read, Data, &offset, 4);
	
	ExportDataAuto(&SCD, Data, &offset, sizeof(SCD));
//	ExportDataAuto(&CDC, Data, &offset, sizeof(CDC)); // removed, seems unnecessary/redundant
	ExportDataAuto(&CDD, Data, &offset, sizeof(CDD));
	ExportDataAuto(&COMM, Data, &offset, sizeof(COMM));
	
	ExportDataAuto(Ram_Backup, Data, &offset, sizeof(Ram_Backup));
	ExportDataAuto(Ram_Backup_Ex, Data, &offset, sizeof(Ram_Backup_Ex));
	
	ExportDataAuto(&Rot_Comp, Data, &offset, sizeof(Rot_Comp));
	ExportDataAuto(&Stamp_Map_Adr, Data, &offset, 4);
	ExportDataAuto(&Buffer_Adr, Data, &offset, 4);
	ExportDataAuto(&Vector_Adr, Data, &offset, 4);
	ExportDataAuto(&Jmp_Adr, Data, &offset, 4);
	ExportDataAuto(&Float_Part, Data, &offset, 4);
	ExportDataAuto(&Draw_Speed, Data, &offset, 4);
	ExportDataAuto(&XS, Data, &offset, 4);
	ExportDataAuto(&YS, Data, &offset, 4);
	ExportDataAuto(&DXS, Data, &offset, 4);
	ExportDataAuto(&DYS, Data, &offset, 4);
	ExportDataAuto(&XD, Data, &offset, 4);
	ExportDataAuto(&YD, Data, &offset, 4);
	ExportDataAuto(&XD_Mul, Data, &offset, 4);
	ExportDataAuto(&H_Dot, Data, &offset, 4);
	
	ExportDataAuto(&Context_sub68K.cycles_needed, Data, &offset, 44);
	ExportDataAuto(&Rom_Data[0x72], Data, &offset, 2);	//Sega CD games can overwrite the low two bytes of the Horizontal Interrupt vector
	
	ExportDataAuto(&fatal_mp3_error, Data, &offset, 4);
	ExportDataAuto(&Current_OUT_Pos, Data, &offset, 4);
	ExportDataAuto(&Current_OUT_Size, Data, &offset, 4);
	ExportDataAuto(&Track_Played, Data, &offset, 1);
	ExportDataAuto(played_tracks_linear, Data, &offset, 100);
	//ExportDataAuto(&Current_IN_Pos, Data, &offset, 4)? // don't save this; bad things happen
	
#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == SEGACD_LENGTH_EX);
#endif
}


/**
 * Import_32X(): Load 32X data from a savestate. (Ported from Gens Rerecording.)
 * @param Data Savestate data.
 */
void Import_32X (unsigned char *Data)
{
	unsigned int offset = 0;
	int i, contextNum;
	
	for (contextNum = 0; contextNum < 2; contextNum++)
	{
		SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;

		ImportDataAuto(context->Cache, Data, &offset, sizeof(context->Cache));
		ImportDataAuto(context->R, Data, &offset, sizeof(context->R));
		ImportDataAuto(&context->SR, Data, &offset, sizeof(context->SR));
		ImportDataAuto(&context->INT, Data, &offset, sizeof(context->INT));
		ImportDataAuto(&context->GBR, Data, &offset, sizeof(context->GBR));
		ImportDataAuto(&context->VBR, Data, &offset, sizeof(context->VBR));
		ImportDataAuto(context->INT_QUEUE, Data, &offset, sizeof(context->INT_QUEUE));
		ImportDataAuto(&context->MACH, Data, &offset, sizeof(context->MACH));
		ImportDataAuto(&context->MACL, Data, &offset, sizeof(context->MACL));
		ImportDataAuto(&context->PR, Data, &offset, sizeof(context->PR));
		ImportDataAuto(&context->PC, Data, &offset, sizeof(context->PC));
		ImportDataAuto(&context->Status, Data, &offset, sizeof(context->Status));
		ImportDataAuto(&context->Base_PC, Data, &offset, sizeof(context->Base_PC));
		ImportDataAuto(&context->Fetch_Start, Data, &offset, sizeof(context->Fetch_Start));
		ImportDataAuto(&context->Fetch_End, Data, &offset, sizeof(context->Fetch_End));
		ImportDataAuto(&context->DS_Inst, Data, &offset, sizeof(context->DS_Inst));
		ImportDataAuto(&context->DS_PC, Data, &offset, sizeof(context->DS_PC));
		ImportDataAuto(&context->Odometer, Data, &offset, sizeof(context->Odometer));
		ImportDataAuto(&context->Cycle_TD, Data, &offset, sizeof(context->Cycle_TD));
		ImportDataAuto(&context->Cycle_IO, Data, &offset, sizeof(context->Cycle_IO));
		ImportDataAuto(&context->Cycle_Sup, Data, &offset, sizeof(context->Cycle_Sup));
		ImportDataAuto(context->IO_Reg, Data, &offset, sizeof(context->IO_Reg));
		ImportDataAuto(&context->DVCR, Data, &offset, sizeof(context->DVCR));
		ImportDataAuto(&context->DVSR, Data, &offset, sizeof(context->DVSR));
		ImportDataAuto(&context->DVDNTH, Data, &offset, sizeof(context->DVDNTH));
		ImportDataAuto(&context->DVDNTL, Data, &offset, sizeof(context->DVDNTL));
		ImportDataAuto(&context->DRCR0, Data, &offset, sizeof(context->DRCR0));
		ImportDataAuto(&context->DRCR1, Data, &offset, sizeof(context->DRCR1));
		ImportDataAuto(&context->DREQ0, Data, &offset, sizeof(context->DREQ0));
		ImportDataAuto(&context->DREQ1, Data, &offset, sizeof(context->DREQ1));
		ImportDataAuto(&context->DMAOR, Data, &offset, sizeof(context->DMAOR));
		ImportDataAuto(&context->SAR0, Data, &offset, sizeof(context->SAR0));
		ImportDataAuto(&context->DAR0, Data, &offset, sizeof(context->DAR0));
		ImportDataAuto(&context->TCR0, Data, &offset, sizeof(context->TCR0));
		ImportDataAuto(&context->CHCR0, Data, &offset, sizeof(context->CHCR0));
		ImportDataAuto(&context->SAR1, Data, &offset, sizeof(context->SAR1));
		ImportDataAuto(&context->DAR1, Data, &offset, sizeof(context->DAR1));
		ImportDataAuto(&context->TCR1, Data, &offset, sizeof(context->TCR1));
		ImportDataAuto(&context->CHCR1, Data, &offset, sizeof(context->CHCR1));
		ImportDataAuto(&context->VCRDIV, Data, &offset, sizeof(context->VCRDIV));
		ImportDataAuto(&context->VCRDMA0, Data, &offset, sizeof(context->VCRDMA0));
		ImportDataAuto(&context->VCRDMA1, Data, &offset, sizeof(context->VCRDMA1));
		ImportDataAuto(&context->VCRWDT, Data, &offset, sizeof(context->VCRWDT));
		ImportDataAuto(&context->IPDIV, Data, &offset, sizeof(context->IPDIV));
		ImportDataAuto(&context->IPDMA, Data, &offset, sizeof(context->IPDMA));
		ImportDataAuto(&context->IPWDT, Data, &offset, sizeof(context->IPWDT));
		ImportDataAuto(&context->IPBSC, Data, &offset, sizeof(context->IPBSC));
		ImportDataAuto(&context->BARA, Data, &offset, sizeof(context->BARA));
		ImportDataAuto(&context->BAMRA, Data, &offset, sizeof(context->BAMRA));
		ImportDataAuto(context->WDT_Tab, Data, &offset, sizeof(context->WDT_Tab));
		ImportDataAuto(&context->WDTCNT, Data, &offset, sizeof(context->WDTCNT));
		ImportDataAuto(&context->WDT_Sft, Data, &offset, sizeof(context->WDT_Sft));
		ImportDataAuto(&context->WDTSR, Data, &offset, sizeof(context->WDTSR));
		ImportDataAuto(&context->WDTRST, Data, &offset, sizeof(context->WDTRST));
		ImportDataAuto(context->FRT_Tab, Data, &offset, sizeof(context->FRT_Tab));
		ImportDataAuto(&context->FRTCNT, Data, &offset, sizeof(context->FRTCNT));
		ImportDataAuto(&context->FRTOCRA, Data, &offset, sizeof(context->FRTOCRA));
		ImportDataAuto(&context->FRTOCRB, Data, &offset, sizeof(context->FRTOCRB));
		ImportDataAuto(&context->FRTTIER, Data, &offset, sizeof(context->FRTTIER));
		ImportDataAuto(&context->FRTCSR, Data, &offset, sizeof(context->FRTCSR));
		ImportDataAuto(&context->FRTTCR, Data, &offset, sizeof(context->FRTTCR));
		ImportDataAuto(&context->FRTTOCR, Data, &offset, sizeof(context->FRTTOCR));
		ImportDataAuto(&context->FRTICR, Data, &offset, sizeof(context->FRTICR));
		ImportDataAuto(&context->FRT_Sft, Data, &offset, sizeof(context->FRT_Sft));
		ImportDataAuto(&context->BCR1, Data, &offset, sizeof(context->BCR1));
		ImportDataAuto(&context->FRTCSR, Data, &offset, sizeof(context->FRTCSR));
	}

	ImportDataAuto(_32X_Ram, Data, &offset, sizeof(_32X_Ram));
	ImportDataAuto(_MSH2_Reg, Data, &offset, sizeof(_MSH2_Reg));
	ImportDataAuto(_SSH2_Reg, Data, &offset, sizeof(_SSH2_Reg));
	ImportDataAuto(_SH2_VDP_Reg, Data, &offset, sizeof(_SH2_VDP_Reg));
	ImportDataAuto(_32X_Comm, Data, &offset, sizeof(_32X_Comm));
	ImportDataAuto(&_32X_ADEN, Data, &offset, sizeof(_32X_ADEN));
	ImportDataAuto(&_32X_RES, Data, &offset, sizeof(_32X_RES));
	ImportDataAuto(&_32X_FM, Data, &offset, sizeof(_32X_FM));
	ImportDataAuto(&_32X_RV, Data, &offset, sizeof(_32X_RV));
	ImportDataAuto(&_32X_DREQ_ST, Data, &offset, sizeof(_32X_DREQ_ST));
	ImportDataAuto(&_32X_DREQ_SRC, Data, &offset, sizeof(_32X_DREQ_SRC));
	ImportDataAuto(&_32X_DREQ_DST, Data, &offset, sizeof(_32X_DREQ_DST));
	ImportDataAuto(&_32X_DREQ_LEN, Data, &offset, sizeof(_32X_DREQ_LEN));
	ImportDataAuto(_32X_FIFO_A, Data, &offset, sizeof(_32X_FIFO_A));
	ImportDataAuto(_32X_FIFO_B, Data, &offset, sizeof(_32X_FIFO_B));
	ImportDataAuto(&_32X_FIFO_Block, Data, &offset, sizeof(_32X_FIFO_Block));
	ImportDataAuto(&_32X_FIFO_Read, Data, &offset, sizeof(_32X_FIFO_Read));
	ImportDataAuto(&_32X_FIFO_Write, Data, &offset, sizeof(_32X_FIFO_Write));
	ImportDataAuto(&_32X_MINT, Data, &offset, sizeof(_32X_MINT));
	ImportDataAuto(&_32X_SINT, Data, &offset, sizeof(_32X_SINT));
	ImportDataAuto(&_32X_HIC, Data, &offset, sizeof(_32X_HIC));
	ImportDataAuto(&CPL_SSH2, Data, &offset, sizeof(CPL_SSH2));
	ImportDataAuto(&CPL_MSH2, Data, &offset, sizeof(CPL_MSH2));
	ImportDataAuto(&Cycles_MSH2, Data, &offset, sizeof(Cycles_MSH2));
	ImportDataAuto(&Cycles_SSH2, Data, &offset, sizeof(Cycles_SSH2));

	ImportDataAuto(&_32X_VDP, Data, &offset, sizeof(_32X_VDP));
	ImportDataAuto(_32X_VDP_Ram, Data, &offset, sizeof(_32X_VDP_Ram));
	ImportDataAuto(_32X_VDP_CRam, Data, &offset, sizeof(_32X_VDP_CRam));

	ImportDataAuto(Set_SR_Table, Data, &offset, sizeof(Set_SR_Table));
	ImportDataAuto(&Bank_SH2, Data, &offset, sizeof(Bank_SH2));

	ImportDataAuto(PWM_FIFO_R, Data, &offset, sizeof(PWM_FIFO_R));
	ImportDataAuto(PWM_FIFO_L, Data, &offset, sizeof(PWM_FIFO_L));
	ImportDataAuto(&PWM_RP_R, Data, &offset, sizeof(PWM_RP_R));
	ImportDataAuto(&PWM_WP_R, Data, &offset, sizeof(PWM_WP_R));
	ImportDataAuto(&PWM_RP_L, Data, &offset, sizeof(PWM_RP_L));
	ImportDataAuto(&PWM_WP_L, Data, &offset, sizeof(PWM_WP_L));
	ImportDataAuto(&PWM_Cycles, Data, &offset, sizeof(PWM_Cycles));
	ImportDataAuto(&PWM_Cycle, Data, &offset, sizeof(PWM_Cycle));
	ImportDataAuto(&PWM_Cycle_Cnt, Data, &offset, sizeof(PWM_Cycle_Cnt));
	ImportDataAuto(&PWM_Int, Data, &offset, sizeof(PWM_Int));
	ImportDataAuto(&PWM_Int_Cnt, Data, &offset, sizeof(PWM_Int_Cnt));
	ImportDataAuto(&PWM_Mode, Data, &offset, sizeof(PWM_Mode));
	ImportDataAuto(&PWM_Out_R, Data, &offset, sizeof(PWM_Out_R));
	ImportDataAuto(&PWM_Out_L, Data, &offset, sizeof(PWM_Out_L));

	ImportDataAuto(_32X_Rom, Data, &offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	ImportDataAuto(_32X_MSH2_Rom, Data, &offset, sizeof(_32X_MSH2_Rom));
	ImportDataAuto(_32X_SSH2_Rom, Data, &offset, sizeof(_32X_SSH2_Rom));

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
 * Export_32X(): Save 32X data to a savestate. (Ported from Gens Rerecording.)
 * @param Data Savestate data.
 */
void Export_32X (unsigned char *Data)
{
	// NOTE: GENS v7 Savestates with 32X data will *not* work on older versions of GENS!
	unsigned int offset = 0;
	int contextNum;
	
	for (contextNum = 0; contextNum < 2; contextNum++)
	{
		SH2_CONTEXT* context = (contextNum == 0) ? &M_SH2 : &S_SH2;
	
		ExportDataAuto(context->Cache, Data, &offset, sizeof(context->Cache));
		ExportDataAuto(context->R, Data, &offset, sizeof(context->R));
		ExportDataAuto(&context->SR, Data, &offset, sizeof(context->SR));
		ExportDataAuto(&context->INT, Data, &offset, sizeof(context->INT));
		ExportDataAuto(&context->GBR, Data, &offset, sizeof(context->GBR));
		ExportDataAuto(&context->VBR, Data, &offset, sizeof(context->VBR));
		ExportDataAuto(context->INT_QUEUE, Data, &offset, sizeof(context->INT_QUEUE));
		ExportDataAuto(&context->MACH, Data, &offset, sizeof(context->MACH));
		ExportDataAuto(&context->MACL, Data, &offset, sizeof(context->MACL));
		ExportDataAuto(&context->PR, Data, &offset, sizeof(context->PR));
		ExportDataAuto(&context->PC, Data, &offset, sizeof(context->PC));
		ExportDataAuto(&context->Status, Data, &offset, sizeof(context->Status));
		ExportDataAuto(&context->Base_PC, Data, &offset, sizeof(context->Base_PC));
		ExportDataAuto(&context->Fetch_Start, Data, &offset, sizeof(context->Fetch_Start));
		ExportDataAuto(&context->Fetch_End, Data, &offset, sizeof(context->Fetch_End));
		ExportDataAuto(&context->DS_Inst, Data, &offset, sizeof(context->DS_Inst));
		ExportDataAuto(&context->DS_PC, Data, &offset, sizeof(context->DS_PC));
		ExportDataAuto(&context->Odometer, Data, &offset, sizeof(context->Odometer));
		ExportDataAuto(&context->Cycle_TD, Data, &offset, sizeof(context->Cycle_TD));
		ExportDataAuto(&context->Cycle_IO, Data, &offset, sizeof(context->Cycle_IO));
		ExportDataAuto(&context->Cycle_Sup, Data, &offset, sizeof(context->Cycle_Sup));
		ExportDataAuto(context->IO_Reg, Data, &offset, sizeof(context->IO_Reg));
		ExportDataAuto(&context->DVCR, Data, &offset, sizeof(context->DVCR));
		ExportDataAuto(&context->DVSR, Data, &offset, sizeof(context->DVSR));
		ExportDataAuto(&context->DVDNTH, Data, &offset, sizeof(context->DVDNTH));
		ExportDataAuto(&context->DVDNTL, Data, &offset, sizeof(context->DVDNTL));
		ExportDataAuto(&context->DRCR0, Data, &offset, sizeof(context->DRCR0));
		ExportDataAuto(&context->DRCR1, Data, &offset, sizeof(context->DRCR1));
		ExportDataAuto(&context->DREQ0, Data, &offset, sizeof(context->DREQ0));
		ExportDataAuto(&context->DREQ1, Data, &offset, sizeof(context->DREQ1));
		ExportDataAuto(&context->DMAOR, Data, &offset, sizeof(context->DMAOR));
		ExportDataAuto(&context->SAR0, Data, &offset, sizeof(context->SAR0));
		ExportDataAuto(&context->DAR0, Data, &offset, sizeof(context->DAR0));
		ExportDataAuto(&context->TCR0, Data, &offset, sizeof(context->TCR0));
		ExportDataAuto(&context->CHCR0, Data, &offset, sizeof(context->CHCR0));
		ExportDataAuto(&context->SAR1, Data, &offset, sizeof(context->SAR1));
		ExportDataAuto(&context->DAR1, Data, &offset, sizeof(context->DAR1));
		ExportDataAuto(&context->TCR1, Data, &offset, sizeof(context->TCR1));
		ExportDataAuto(&context->CHCR1, Data, &offset, sizeof(context->CHCR1));
		ExportDataAuto(&context->VCRDIV, Data, &offset, sizeof(context->VCRDIV));
		ExportDataAuto(&context->VCRDMA0, Data, &offset, sizeof(context->VCRDMA0));
		ExportDataAuto(&context->VCRDMA1, Data, &offset, sizeof(context->VCRDMA1));
		ExportDataAuto(&context->VCRWDT, Data, &offset, sizeof(context->VCRWDT));
		ExportDataAuto(&context->IPDIV, Data, &offset, sizeof(context->IPDIV));
		ExportDataAuto(&context->IPDMA, Data, &offset, sizeof(context->IPDMA));
		ExportDataAuto(&context->IPWDT, Data, &offset, sizeof(context->IPWDT));
		ExportDataAuto(&context->IPBSC, Data, &offset, sizeof(context->IPBSC));
		ExportDataAuto(&context->BARA, Data, &offset, sizeof(context->BARA));
		ExportDataAuto(&context->BAMRA, Data, &offset, sizeof(context->BAMRA));
		ExportDataAuto(context->WDT_Tab, Data, &offset, sizeof(context->WDT_Tab));
		ExportDataAuto(&context->WDTCNT, Data, &offset, sizeof(context->WDTCNT));
		ExportDataAuto(&context->WDT_Sft, Data, &offset, sizeof(context->WDT_Sft));
		ExportDataAuto(&context->WDTSR, Data, &offset, sizeof(context->WDTSR));
		ExportDataAuto(&context->WDTRST, Data, &offset, sizeof(context->WDTRST));
		ExportDataAuto(context->FRT_Tab, Data, &offset, sizeof(context->FRT_Tab));
		ExportDataAuto(&context->FRTCNT, Data, &offset, sizeof(context->FRTCNT));
		ExportDataAuto(&context->FRTOCRA, Data, &offset, sizeof(context->FRTOCRA));
		ExportDataAuto(&context->FRTOCRB, Data, &offset, sizeof(context->FRTOCRB));
		ExportDataAuto(&context->FRTTIER, Data, &offset, sizeof(context->FRTTIER));
		ExportDataAuto(&context->FRTCSR, Data, &offset, sizeof(context->FRTCSR));
		ExportDataAuto(&context->FRTTCR, Data, &offset, sizeof(context->FRTTCR));
		ExportDataAuto(&context->FRTTOCR, Data, &offset, sizeof(context->FRTTOCR));
		ExportDataAuto(&context->FRTICR, Data, &offset, sizeof(context->FRTICR));
		ExportDataAuto(&context->FRT_Sft, Data, &offset, sizeof(context->FRT_Sft));
		ExportDataAuto(&context->BCR1, Data, &offset, sizeof(context->BCR1));
		ExportDataAuto(&context->FRTCSR, Data, &offset, sizeof(context->FRTCSR));
	}
	
	ExportDataAuto(_32X_Ram, Data, &offset, sizeof(_32X_Ram));
	ExportDataAuto(_MSH2_Reg, Data, &offset, sizeof(_MSH2_Reg));
	ExportDataAuto(_SSH2_Reg, Data, &offset, sizeof(_SSH2_Reg));
	ExportDataAuto(_SH2_VDP_Reg, Data, &offset, sizeof(_SH2_VDP_Reg));
	ExportDataAuto(_32X_Comm, Data, &offset, sizeof(_32X_Comm));
	ExportDataAuto(&_32X_ADEN, Data, &offset, sizeof(_32X_ADEN));
	ExportDataAuto(&_32X_RES, Data, &offset, sizeof(_32X_RES));
	ExportDataAuto(&_32X_FM, Data, &offset, sizeof(_32X_FM));
	ExportDataAuto(&_32X_RV, Data, &offset, sizeof(_32X_RV));
	ExportDataAuto(&_32X_DREQ_ST, Data, &offset, sizeof(_32X_DREQ_ST));
	ExportDataAuto(&_32X_DREQ_SRC, Data, &offset, sizeof(_32X_DREQ_SRC));
	ExportDataAuto(&_32X_DREQ_DST, Data, &offset, sizeof(_32X_DREQ_DST));
	ExportDataAuto(&_32X_DREQ_LEN, Data, &offset, sizeof(_32X_DREQ_LEN));
	ExportDataAuto(_32X_FIFO_A, Data, &offset, sizeof(_32X_FIFO_A));
	ExportDataAuto(_32X_FIFO_B, Data, &offset, sizeof(_32X_FIFO_B));
	ExportDataAuto(&_32X_FIFO_Block, Data, &offset, sizeof(_32X_FIFO_Block));
	ExportDataAuto(&_32X_FIFO_Read, Data, &offset, sizeof(_32X_FIFO_Read));
	ExportDataAuto(&_32X_FIFO_Write, Data, &offset, sizeof(_32X_FIFO_Write));
	ExportDataAuto(&_32X_MINT, Data, &offset, sizeof(_32X_MINT));
	ExportDataAuto(&_32X_SINT, Data, &offset, sizeof(_32X_SINT));
	ExportDataAuto(&_32X_HIC, Data, &offset, sizeof(_32X_HIC));
	ExportDataAuto(&CPL_SSH2, Data, &offset, sizeof(CPL_SSH2));
	ExportDataAuto(&CPL_MSH2, Data, &offset, sizeof(CPL_MSH2));
	ExportDataAuto(&Cycles_MSH2, Data, &offset, sizeof(Cycles_MSH2));
	ExportDataAuto(&Cycles_SSH2, Data, &offset, sizeof(Cycles_SSH2));
	
	ExportDataAuto(&_32X_VDP, Data, &offset, sizeof(_32X_VDP));
	ExportDataAuto(_32X_VDP_Ram, Data, &offset, sizeof(_32X_VDP_Ram));
	ExportDataAuto(_32X_VDP_CRam, Data, &offset, sizeof(_32X_VDP_CRam));
	
	ExportDataAuto(Set_SR_Table, Data, &offset, sizeof(Set_SR_Table));
	ExportDataAuto(&Bank_SH2, Data, &offset, sizeof(Bank_SH2));
	
	ExportDataAuto(PWM_FIFO_R, Data, &offset, sizeof(PWM_FIFO_R));
	ExportDataAuto(PWM_FIFO_L, Data, &offset, sizeof(PWM_FIFO_L));
	ExportDataAuto(&PWM_RP_R, Data, &offset, sizeof(PWM_RP_R));
	ExportDataAuto(&PWM_WP_R, Data, &offset, sizeof(PWM_WP_R));
	ExportDataAuto(&PWM_RP_L, Data, &offset, sizeof(PWM_RP_L));
	ExportDataAuto(&PWM_WP_L, Data, &offset, sizeof(PWM_WP_L));
	ExportDataAuto(&PWM_Cycles, Data, &offset, sizeof(PWM_Cycles));
	ExportDataAuto(&PWM_Cycle, Data, &offset, sizeof(PWM_Cycle));
	ExportDataAuto(&PWM_Cycle_Cnt, Data, &offset, sizeof(PWM_Cycle_Cnt));
	ExportDataAuto(&PWM_Int, Data, &offset, sizeof(PWM_Int));
	ExportDataAuto(&PWM_Int_Cnt, Data, &offset, sizeof(PWM_Int_Cnt));
	ExportDataAuto(&PWM_Mode, Data, &offset, sizeof(PWM_Mode));
	ExportDataAuto(&PWM_Out_R, Data, &offset, sizeof(PWM_Out_R));
	ExportDataAuto(&PWM_Out_L, Data, &offset, sizeof(PWM_Out_L));
	
	ExportDataAuto(_32X_Rom, Data, &offset, 1024); // just in case some of these bytes are not in fact read-only as was apparently the case with Sega CD games (1024 seems acceptably small)
	ExportDataAuto(_32X_MSH2_Rom, Data, &offset, sizeof(_32X_MSH2_Rom));
	ExportDataAuto(_32X_SSH2_Rom, Data, &offset, sizeof(_32X_SSH2_Rom));
	
#ifdef GENS_DEBUG_SAVESTATE
	assert(offset == G32X_LENGTH_EX);
#endif
}


int
Load_SRAM (void)
{
  FILE *SRAM_File = 0;
  char Name[2048];

  memset (SRAM, 0, 64 * 1024);

  strcpy (Name, SRAM_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, ".srm");

  if ((SRAM_File = fopen (Name, "rb")) == 0)
    return 0;
  fread (SRAM, 64 * 1024, 1, SRAM_File);
  fclose (SRAM_File);

  strcpy (Str_Tmp, "SRAM loaded from ");
  strcat (Str_Tmp, Name);
  Put_Info (Str_Tmp, 2000);
  return 1;
}

int
Save_SRAM (void)
{
  FILE *SRAM_File = 0;
  int size_to_save, i;
  char Name[2048];

  i = (64 * 1024) - 1;
  while ((i >= 0) && (SRAM[i] == 0))
    i--;

  if (i < 0)
    return 0;

  i++;

  size_to_save = 1;
  while (i > size_to_save)
    size_to_save <<= 1;

  strcpy (Name, SRAM_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, ".srm");

  if ((SRAM_File = fopen (Name, "wb")) == 0)
    return 0;

  fwrite (SRAM, size_to_save, 1, SRAM_File);
  fclose (SRAM_File);

  strcpy (Str_Tmp, "SRAM saved in ");
  strcat (Str_Tmp, Name);
  Put_Info (Str_Tmp, 2000);

  return 1;
}


void
S_Format_BRAM (unsigned char *buf)
{
  memset (buf, 0x5F, 11);

  buf[0x0F] = 0x40;

  buf[0x11] = 0x7D;
  buf[0x13] = 0x7D;
  buf[0x15] = 0x7D;
  buf[0x17] = 0x7D;

  sprintf ((char *) &buf[0x20], "SEGA CD ROM");
  sprintf ((char *) &buf[0x30], "RAM CARTRIDGE");

  buf[0x24] = 0x5F;
  buf[0x27] = 0x5F;

  buf[0x2C] = 0x01;

  buf[0x33] = 0x5F;
  buf[0x3D] = 0x5F;
  buf[0x3E] = 0x5F;
  buf[0x3F] = 0x5F;
}


void
Format_Backup_Ram (void)
{
  memset (Ram_Backup, 0, 8 * 1024);

  S_Format_BRAM (&Ram_Backup[0x1FC0]);

  memset (Ram_Backup_Ex, 0, 64 * 1024);
}


int
Load_BRAM (void)
{
  FILE *BRAM_File = 0;
  char Name[2048];

  Format_Backup_Ram ();

  strcpy (Name, BRAM_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, ".brm");

  if ((BRAM_File = fopen (Name, "rb")) == 0)
    return 0;

  fread (Ram_Backup, 8 * 1024, 1, BRAM_File);
  fread (Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
  fclose (BRAM_File);

  strcpy (Str_Tmp, "BRAM loaded from ");
  strcat (Str_Tmp, Name);
  Put_Info (Str_Tmp, 2000);
  return 1;
}

int
Save_BRAM (void)
{
  FILE *BRAM_File = 0;
  char Name[2048];

  strcpy (Name, BRAM_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, ".brm");

  if ((BRAM_File = fopen (Name, "wb")) == 0)
    return 0;

  fwrite (Ram_Backup, 8 * 1024, 1, BRAM_File);
  fwrite (Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, 1, BRAM_File);
  fclose (BRAM_File);

  strcpy (Str_Tmp, "BRAM saved in ");
  strcat (Str_Tmp, Name);
  Put_Info (Str_Tmp, 2000);
  return 1;
}
