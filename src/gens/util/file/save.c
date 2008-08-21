#include <SDL.h>
#include "port.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.h"
#include "cpu_68k.h"
#include "cpu_sh2.h"
#include "sh2.h"
#include "z80.h"
#include "cd_aspi.h"
#include "gens.h"
#include "g_main.h"
#include "g_input.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"
#include "rom.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "mem_z80.h"
#include "ym2612.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"
#include "scrshot.h"
#include "ggenie.h"
#include "io.h"
#include "misc.h"
#include "cd_sys.h"
#include "gfx_cd.h"
#include "lc89510.h"
#include "cd_file.h"
#include "vdp_32x.h"
#include "support.h"
#include "ui_proxy.h"

// GENS Re-Recording
// fatal_mp3_error indicates an error occurred while reading an MP3 for a Sega CD game.
extern int fatal_mp3_error;	// cdda_mp3.c

// Various MP3 stuff, needed for GENS Re-Recording
extern unsigned int Current_OUT_Pos, Current_OUT_Size;	// cdda_mp3.c
extern char preloaded_tracks [100], played_tracks_linear [101]; // added for synchronous MP3 code

enum
{
  DIK_UP = SDLK_UP, DIK_DOWN = SDLK_DOWN,
  DIK_LEFT = SDLK_LEFT, DIK_RIGHT = SDLK_RIGHT,
  DIK_RETURN = SDLK_RETURN, DIK_A = SDLK_a,
  DIK_S = SDLK_s, DIK_D = SDLK_d,
  DIK_RSHIFT = SDLK_RSHIFT, DIK_Z = SDLK_z,
  DIK_X = SDLK_x, DIK_C = SDLK_c, DIK_Y = SDLK_y,
  DIK_H = SDLK_h, DIK_G = SDLK_g, DIK_J = SDLK_j,
  DIK_U = SDLK_u, DIK_K = SDLK_k, DIK_L = SDLK_l,
  DIK_M = SDLK_m, DIK_T = SDLK_t, DIK_I = SDLK_i,
  DIK_O = SDLK_o, DIK_P = SDLK_p
};

#define DIR_SEPARATOR '/'

int Current_State = 0;
char State_Dir[1024] = "";
char SRAM_Dir[1024] = "";
char BRAM_Dir[1024] = "";
unsigned char State_Buffer[MAX_STATE_FILE_LENGTH];


int
Change_File_S (char *Dest, char *Dir)
{
  gchar *filename = NULL;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Save state", GTK_FILE_CHOOSER_ACTION_SAVE);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addStateFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (filename != NULL)
    {
      strcpy (Dest, filename);
      g_free (filename);
      return 1;
    }
  return 0;
}


int
Change_File_L (char *Dest, char *Dir)
{
  gchar *filename = NULL;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Load state", GTK_FILE_CHOOSER_ACTION_OPEN);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addStateFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (filename)
    {
      strcpy (Dest, filename);
      g_free (filename);
      return 1;
    }
  return 0;
}

int
Change_File_L_CD (char *Dest, char *Dir)
{

  gchar *filename = NULL;
  GtkWidget *widget;
  gint res;
  widget =
    create_file_chooser_dialog ("Load SegaCD image file",
				GTK_FILE_CHOOSER_ACTION_OPEN);
  addIsoFilter (widget);
//      fileselection_set_dir (fd.filesel, Rom_Dir);
  addStateFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
    }
  gtk_widget_destroy (widget);

  if (filename)
    {
      strcpy (Dest, filename);
      g_free (filename);
      return 1;
    }
  return 0;
}

/*int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
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
int Load_State (char *Name)
{
	FILE *f;
	unsigned char *buf;
	int len;
	
	len = GENESIS_STATE_LENGTH;
	if (Genesis_Started);
	else if (SegaCD_Started)
		len += SEGACD_LENGTH_EX;
	else if (_32X_Started)
		len += G32X_LENGTH_EX;
	else
		return 0;
	
	buf = State_Buffer;
	
	if ((f = fopen (Name, "rb")) == NULL)
		return 0;
	
	memset (buf, 0, len);
	if (fread (buf, 1, len, f))
	{
		//z80_Reset (&M_Z80); // Commented out in Gens Re-Recording...
		/*
		main68k_reset();
		YM2612ResetChip(0);
		Reset_VDP();
		*/
		
		// Save functions updated from Gens Re-Recording
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
		// Commented out in Gens Re-Recording...
		Flag_Clr_Scr = 1;
		CRam_Flag = 1;
		VRam_Flag = 1;
		*/
		
		sprintf (Str_Tmp, "STATE %d LOADED", Current_State);
		Put_Info (Str_Tmp, 2000);
	}
	
	fclose (f);
	
	return 1;
}


/**
 * Save_State(): Save a savestate.
 * @param Name Filename of the savestate.
 * @return 1 if successful; 0 on error.
 */
int Save_State (char *Name)
{
	if (Name == NULL)
		return 0;
	FILE *f;
	unsigned char *buf;
	int len;
	
	buf = State_Buffer;
	if ((f = fopen (Name, "wb")) == NULL)
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
	memset (buf, 0, len);
	
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
	
	fwrite (State_Buffer, 1, len, f);
	fclose (f);
	
	sprintf (Str_Tmp, "STATE %d SAVED", Current_State);
	Put_Info (Str_Tmp, 2000);
	
	return 1;
}


/*

GST genecyst save file (Documentation updated from GENS Re-Recording)

Range        Size   Description
-----------  -----  -----------
00000-00002  3      "GST"
00006-00007  2      "\xE0\x40"
000FA-00112  24     VDP registers
00112-00191  128    Color RAM
00192-001E1  80     Vertical scroll RAM
001E4-003E3  512    YM2612 registers
00474-02473  8192   Z80 RAM
02478-12477  65536  68K RAM
12478-22477  65536  Video RAM

main 68000 registers
--------------------

00080-0009F : D0-D7
000A0-000BF : A0-A7
000C8 : PC
000D0 : SR
000D2 : USP
000D6 : SSP

Z80 registers
-------------

00404 : AF
00408 : BC
0040C : DE
00410 : HL
00414 : IX
00418 : IY
0041C : PC
00420 : SP
00424 : AF'
00428 : BC'
0042C : DE'
00430 : HL'

00434 : I
00435 : Unknow
00436 : IFF1 = IFF2
00437 : Unknow

The 'R' register is not supported.

Z80 State
---------

00438 : Z80 RESET
00439 : Z80 BUSREQ
0043A : Unknow
0043B : Unknow

0043C : Z80 BANK (DWORD)

Gens and Kega ADD
-----------------

00040 : last VDP Control data written (DWORD)
00044 : second write flag (1 for second write) 
00045 : DMA Fill flag (1 mean next data write will cause a DMA fill)
00048 : VDP write address (DWORD)

00050 : Version       (Genecyst=0 ; Kega=5 ; Gens=5)
00051 : Emulator ID   (Genecyst=0 ; Kega=0 ; Gens=1)
00052 : System ID     (Genesis=0 ; SegaCD=1 ; 32X=2 ; SegaCD32X=3)

00060-00070 : PSG registers (WORD).


SEGA CD
-------

+00000-00FFF : Gate array & sub 68K
+01000-80FFF : Prg RAM
+81000-C0FFF : Word RAM (2M mode arrangement)
+C1000-D0FFF : PCM RAM
+D1000-DFFFF : CDD & CDC data (16 kB cache include)

32X
---

main SH2
--------

+00000-00FFF : cache
+01000-011FF : IO registers
+01200-0123F : R0-R15
+01240 : SR
+01244 : GBR
+01248 : VBR
+0124C : MACL
+01250 : MACH
+01254 : PR
+01258 : PC
+0125C : State

sub SH2
-------

+01400-023FF : cache
+02400-025FF : IO registers
+02600-0263F : R0-R15
+02640 : SR
+02644 : GBR
+02648 : VBR
+0264C : MACL
+02650 : MACH
+02654 : PR
+02658 : PC
+0265C : State

others
------
// Fix 32X save state :
// enregistrer correctement les registres syst�mes ...

+02700 : ADEN bit (bit 0)
+02701 : FM bit (bit 7)
+02702 : Master SH2 INT mask register
+02703 : Slave SH2 INT mask register
+02704 : 68000 32X rom bank register
+02705 : RV (Rom to VRAM DMA allowed) bit (bit 0)
+02710-0273F : FIFO stuff (not yet done)
+02740-0274F : 32X communication buffer
+02750-02759 : PWM registers
+02760-0276F : 32X VDP registers
+02800-029FF : 32X palette
+02A00-429FF : SDRAM
+42A00-829FF : FB1 & FB2

*/

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
	unsigned char* dst = (unsigned char *) into;
	const unsigned char* src = ((const unsigned char*) data) + offset;
	while(numBytes--) *dst++ = *src++;
}
inline void ExportData(const void* from, void* data, unsigned int offset, unsigned int numBytes)
{
	const unsigned char* src = (const unsigned char *) from;
	unsigned char* dst = ((unsigned char*) data) + offset;
#ifdef _DEBUG
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
inline void ImportDataAuto(void* into, const void* data, unsigned int* offset, unsigned int numBytes)
{
	ImportData(into, data, *offset, numBytes);
	*offset += numBytes;
}
inline void ExportDataAuto(const void* from, void* data, unsigned int* offset, unsigned int numBytes)
{
	ExportData(from, data, *offset, numBytes);
	*offset += numBytes;
}

/**
 * Import_Genesis(): Load Genesis data from a savestate. (Portions ported from GENS Re-Recording.)
 * @param Data Savestate data.
 * @return Number of bytes read.
 */
int Import_Genesis (unsigned char *Data)
{
	unsigned char Reg_1[0x200], *src;
	int i;
	
	// Savestate V6 and V7 code from Gens Re-Recording.
	
	/*
	// Commented out in GENS Re-Recording.
	VDP_Int = 0;
	DMAT_Lenght = 0;
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
	YM2612_Restore (Reg_1);
	
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
		
		Write_VDP_Ctrl (Data[0x40] + (Data[0x41] << 8));
		Write_VDP_Ctrl (Data[0x42] + (Data[0x43] << 8));
		
		Ctrl.Flag = Data[0x44];
		Ctrl.DMA = (Data[0x45] & 1) << 2;
		Ctrl.Address = Data[0x48] + (Data[0x49] << 8);
		
		ImportData(&Bank_Z80, Data, 0x43C, 4);
		
		if (Version >= 4)
		{
			for (i = 0; i < 8; i++)
				PSG_Save[i] = Data[i * 2 + 0x60] + (Data[i * 2 + 0x61] << 8);
			PSG_Restore_State ();
		}
	}
	
	z80_Set_AF (&M_Z80, Data[0x404] + (Data[0x405] << 8));
	M_Z80.AF.b.FXY = Data[0x406]; //Modif N [GENS Re-Recording]
	M_Z80.BC.w.BC = Data[0x408] + (Data[0x409] << 8);
	M_Z80.DE.w.DE = Data[0x40C] + (Data[0x40D] << 8);
	M_Z80.HL.w.HL = Data[0x410] + (Data[0x411] << 8);
	M_Z80.IX.w.IX = Data[0x414] + (Data[0x415] << 8);
	M_Z80.IY.w.IY = Data[0x418] + (Data[0x419] << 8);
	z80_Set_PC (&M_Z80, Data[0x41C] + (Data[0x41D] << 8));
	M_Z80.SP.w.SP = Data[0x420] + (Data[0x421] << 8);
	z80_Set_AF2 (&M_Z80, Data[0x424] + (Data[0x425] << 8));
	M_Z80.BC2.w.BC2 = Data[0x428] + (Data[0x429] << 8);
	M_Z80.DE2.w.DE2 = Data[0x42C] + (Data[0x42D] << 8);
	M_Z80.HL2.w.HL2 = Data[0x430] + (Data[0x431] << 8);
	M_Z80.I = Data[0x434] & 0xFF;
	
	// GENS Re-Recording: This seems to only be used for movies (e.g. *.giz), so ignore it for now.
	//FrameCount = Data[0x22478] + (Data[0x22479] << 8) + (Data[0x2247A] << 16) + (Data[0x2247B] << 24);
	
	main68k_GetContext (&Context_68K);
	
	// VDP registers
	for (i = 0; i < 24; i++)
		Set_VDP_Reg (i, Data[0xFA + i]);
	
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
	// Ported from GENS Re-Recording.
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
		
		ImportDataAuto(&VDP_Reg.H_Int, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Set1, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Set2, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_ScrA_Adr, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_ScrA_Adr, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_Win_Adr, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Pat_ScrB_Adr, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Spr_Att_Adr, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Reg6, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.BG_Color, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Reg8, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Reg9, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.H_Int, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Set3, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Set4, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.H_Scr_Adr, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Reg14, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Auto_Inc, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Scr_Size, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Win_H_Pos, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Win_V_Pos, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght_L, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght_H, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_L, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_M, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_H, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Address, Data, &offset, 4);
		
		ImportDataAuto(&Controller_1_Counter, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Delay, Data, &offset, 4);
		ImportDataAuto(&Controller_1_State, Data, &offset, 4);
		ImportDataAuto(&Controller_1_COM, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Counter, Data, &offset, 4);
		ImportDataAuto(&Controller_2_Delay, Data, &offset, 4);
		ImportDataAuto(&Controller_2_State, Data, &offset, 4);
		ImportDataAuto(&Controller_2_COM, Data, &offset, 4);
		ImportDataAuto(&Memory_Control_Status, Data, &offset, 4);
		ImportDataAuto(&Cell_Conv_Tab, Data, &offset, 4);
		ImportDataAuto(&Controller_1_Type, Data, &offset, 4);
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
		ImportDataAuto(&Controller_2_Type, Data, &offset, 4);
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
		
		ImportDataAuto(&DMAT_Lenght, Data, &offset, 4);
		ImportDataAuto(&DMAT_Type, Data, &offset, 4);
		ImportDataAuto(&DMAT_Tmp, Data, &offset, 4);
		ImportDataAuto(&VDP_Current_Line, Data, &offset, 4);
		ImportDataAuto(&VDP_Num_Vis_Lines, Data, &offset, 4);
		ImportDataAuto(&VDP_Num_Vis_Lines, Data, &offset, 4);
		ImportDataAuto(&Bank_M68K, Data, &offset, 4);
		ImportDataAuto(&S68K_State, Data, &offset, 4);
		ImportDataAuto(&Z80_State, Data, &offset, 4);
		ImportDataAuto(&Last_BUS_REQ_Cnt, Data, &offset, 4);
		ImportDataAuto(&Last_BUS_REQ_St, Data, &offset, 4);
		ImportDataAuto(&Fake_Fetch, Data, &offset, 4);
		ImportDataAuto(&Game_Mode, Data, &offset, 4);
		ImportDataAuto(&CPU_Mode, Data, &offset, 4);
		ImportDataAuto(&CPL_M68K, Data, &offset, 4);
		ImportDataAuto(&CPL_S68K, Data, &offset, 4);
		ImportDataAuto(&CPL_Z80, Data, &offset, 4);
		ImportDataAuto(&Cycles_S68K, Data, &offset, 4);
		ImportDataAuto(&Cycles_M68K, Data, &offset, 4);
		ImportDataAuto(&Cycles_Z80, Data, &offset, 4);
		ImportDataAuto(&VDP_Status, Data, &offset, 4);
		ImportDataAuto(&VDP_Int, Data, &offset, 4);
		ImportDataAuto(&Ctrl.Write, Data, &offset, 4);
		ImportDataAuto(&Ctrl.DMA_Mode, Data, &offset, 4);
		ImportDataAuto(&Ctrl.DMA, Data, &offset, 4);
		//ImportDataAuto(&CRam_Flag, Data, &offset, 4); //Causes screen to blank
		//&offset+=4;
		// TODO: LagCount for GENS ReRecording.
		//ImportDataAuto(&LagCount, Data, &offset, 4);
		offset += 4;
		ImportDataAuto(&VRam_Flag, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.Auto_Inc, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght, Data, &offset, 4);
	////	ImportDataAuto(VRam, Data, &offset, 65536);
		ImportDataAuto(CRam, Data, &offset, 512);
	////	ImportDataAuto(VSRam, Data, &offset, 64);
		ImportDataAuto(H_Counter_Table, Data, &offset, 512 * 2);
	////	ImportDataAuto(Spr_Link, Data, &offset, 4*256);
	////	extern int DMAT_Tmp, VSRam_Over;
	////	ImportDataAuto(&DMAT_Tmp, Data, &offset, 4);
	////	ImportDataAuto(&VSRam_Over, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght_L, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght_H, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_L, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_M, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Src_Adr_H, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Lenght, Data, &offset, 4);
		ImportDataAuto(&VDP_Reg.DMA_Address, Data, &offset, 4);
		
#ifdef _DEBUG
		int desiredoffset = GENESIS_V6_STATE_LENGTH;
		assert(offset == desiredoffset);
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
		
		ImportDataAuto(&M_Z80, Data, &offset, 0x5C); // some important parts of this weren't saved above
		ImportDataAuto(&M_Z80.RetIC, Data, &offset, 4); // not sure about the last two variables, might as well save them too
		ImportDataAuto(&M_Z80.IntAckC, Data, &offset, 4);
		
		ImportDataAuto(&Context_68K.dreg[0], Data, &offset, 86); // some important parts of this weren't saved above
		
		ImportDataAuto(&Controller_1_State, Data, &offset, 448); // apparently necessary (note: 448 == (((char*)&Controller_2D_Z)+sizeof(Controller_2D_Z) - (char*)&Controller_1_State))
		
		// apparently necessary
		ImportDataAuto(&VDP_Status, Data, &offset, 4);
		ImportDataAuto(&VDP_Int, Data, &offset, 4);
		ImportDataAuto(&VDP_Current_Line, Data, &offset, 4);
		ImportDataAuto(&VDP_Num_Lines, Data, &offset, 4);
		ImportDataAuto(&VDP_Num_Vis_Lines, Data, &offset, 4);
		ImportDataAuto(&DMAT_Lenght, Data, &offset, 4);
		ImportDataAuto(&DMAT_Type, Data, &offset, 4);
		//ImportDataAuto(&CRam_Flag, Data, &offset, 4); //emulator flag which causes Gens not to update its draw palette, but doesn't affect sync state
		// TODO: LagCount for GENS ReRecording.
		//ImportDataAuto(&LagCount, Data, &offset, 4);
		offset += 4;
		ImportDataAuto(&VRam_Flag, Data, &offset, 4);
		ImportDataAuto(&CRam, Data, &offset, 256 * 2);
		
		// it's probably safer sync-wise to keep SRAM stuff in the savestate
		ImportDataAuto(&SRAM, Data, &offset, sizeof(SRAM));
		ImportDataAuto(&SRAM_Start, Data, &offset, 4);
		ImportDataAuto(&SRAM_End, Data, &offset, 4);
		ImportDataAuto(&SRAM_ON, Data, &offset, 4);
		ImportDataAuto(&SRAM_Write, Data, &offset, 4);
		ImportDataAuto(&SRAM_Custom, Data, &offset, 4);
		
		// this group I'm not sure about, they don't seem to be necessary but I'm keeping them around just in case
		ImportDataAuto(&Bank_M68K, Data, &offset, 4);
		ImportDataAuto(&S68K_State, Data, &offset, 4);
		ImportDataAuto(&Z80_State, Data, &offset, 4);
		ImportDataAuto(&Last_BUS_REQ_Cnt, Data, &offset, 4);
		ImportDataAuto(&Last_BUS_REQ_St, Data, &offset, 4);
		ImportDataAuto(&Fake_Fetch, Data, &offset, 4);
		ImportDataAuto(&Game_Mode, Data, &offset, 4);
		ImportDataAuto(&CPU_Mode, Data, &offset, 4);
		ImportDataAuto(&CPL_M68K, Data, &offset, 4);
		ImportDataAuto(&CPL_S68K, Data, &offset, 4);
		ImportDataAuto(&CPL_Z80, Data, &offset, 4);
		ImportDataAuto(&Cycles_S68K, Data, &offset, 4);
		ImportDataAuto(&Cycles_M68K, Data, &offset, 4);
		ImportDataAuto(&Cycles_Z80, Data, &offset, 4);
		ImportDataAuto(&Gen_Mode, Data, &offset, 4);
		ImportDataAuto(&Gen_Version, Data, &offset, 4);
		ImportDataAuto(H_Counter_Table, Data, &offset, 512 * 2);
		ImportDataAuto(&VDP_Reg, Data, &offset, sizeof(VDP_Reg));
		ImportDataAuto(&Ctrl, Data, &offset, sizeof(Ctrl));
		
		ImportDataAuto(&Context_68K.cycles_needed, Data, &offset, 44);
		
#ifdef _DEBUG
		int desiredoffset = GENESIS_STATE_LENGTH;
		assert(offset == desiredoffset);
#endif		
	}
	
	main68k_SetContext (&Context_68K);
	return len;
}


/**
 * Export_Genesis(): Save Genesis data to a savestate. (Portions ported from GENS Re-Recording.)
 * @param Data Savestate data.
 */
void Export_Genesis (unsigned char *Data)
{
	// This savestate function uses the GENS v7 savestate format.
	// Note about GENS v7 savestate format:
	// - Plain MD savestates will work in older versions of GENS.
	// - Sega CD savestates won't work in older versions, but then again, older versions didn't properly support it.
	// - 32X savestates will *not* work in older versions of GENS. :(
	
	// from GENS Re-Recording
	struct S68000CONTEXT Context_68K; // Modif N.: apparently no longer necessary but I'm leaving it here just to be safe: purposely shadows the global Context_68K variable with this local copy to avoid tampering with it while saving
	
	unsigned char Reg_1[0x200], *src;
	int i;
	
	// Be sure to finish DMA before save
	while (DMAT_Lenght)
		Update_DMA ();
	
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
	
#ifdef _DEBUG
	int contextsize1 = main68k_GetContextSize();
	int contextsize2 = sizeof(Context_68K);
	assert(contextsize1 == contextsize2);
#endif
	
	// Copy the PSG state into the savestate buffer.
	for (i = 0; i < 8; i++)
	{
		Data[0x60 + i * 2] = PSG_Save[i] & 0xFF;
		Data[0x61 + i * 2] = (PSG_Save[i] >> 8) & 0xFF;
	}
	
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
	
	// GENS Re-Recording
	Data[0x46] = Ctrl.Access & 0xFF; //Nitsuja added this
	Data[0x47] = (Ctrl.Access >> 8) & 0xFF; //Nitsuja added this
	
	Data[0x48] = Ctrl.Address & 0xFF;
	Data[0x49] = (Ctrl.Address >> 8) & 0xFF;
	
	VDP_Reg.DMA_Lenght_L = VDP_Reg.DMA_Lenght & 0xFF;
	VDP_Reg.DMA_Lenght_H = (VDP_Reg.DMA_Lenght >> 8) & 0xFF;
	
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
		Data[i + 0x112] = CRam[i];
	for (i = 0; i < 0x50; i++)
		Data[i + 0x192] = VSRam[i];
	
	YM2612_Save (Reg_1);
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
	Data[0x41D] = (unsigned char) (z80_Get_PC (&M_Z80) >> 8);
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
	
	// TODO: This is from GENS Re-Recording, and is probably not any use right now.
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
	ExportDataAuto(&DMAT_Lenght, Data, &offset, 4);
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
	
	#ifdef _DEBUG
		// assert that the final offset value equals our savestate size, otherwise we screwed up
		// if it fails, that probably means you have to add ((int)offset-(int)desiredoffset) to the last GENESIS_LENGTH_EX define
		int desiredoffset = GENESIS_STATE_LENGTH;
		assert(offset == desiredoffset);
	#endif	
}


/**
 * Import_SegaCD(): Load Sega CD data from a savestate. (Ported from GENS Re-Recording.)
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
		
#ifdef _DEBUG
		int desiredoffset = SEGACD_LENGTH_EX;
		assert(offset == desiredoffset);
#endif
	}
	
	sub68k_SetContext(&Context_sub68K); // Modif N. -- moved here from earlier in the function
	
	M68K_Set_Prg_Ram();
	MS68K_Set_Word_Ram();	
}


/**
 * Export_SegaCD(): Save Sega CD data to a savestate. (Uses GENS v7 format from GENS Re-Recording.)
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
	
#ifdef _DEBUG
	int desiredoffset = SEGACD_LENGTH_EX;
	assert(offset == desiredoffset);
#endif
}


/**
 * Import_32X(): Load 32X data from a savestate. (Ported from GENS Re-Recording.)
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

#ifdef _DEBUG
	int desiredoffset = G32X_LENGTH_EX;
	assert(offset == desiredoffset);
#endif
}


/**
 * Export_32X(): Save 32X data to a savestate. (Ported from GENS Re-Recording.)
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
	
#ifdef _DEBUG
	int desired&offset = G32X_LENGTH_EX;
	assert(&offset == desired&offset);
#endif
}


int
Save_Config (char *File_Name)
{
  char Conf_File[1024];

  strcpy (Conf_File, File_Name);

  WritePrivateProfileString ("General", "Rom path", Rom_Dir, Conf_File);
  WritePrivateProfileString ("General", "Save path", State_Dir, Conf_File);
  WritePrivateProfileString ("General", "SRAM path", SRAM_Dir, Conf_File);
  WritePrivateProfileString ("General", "BRAM path", BRAM_Dir, Conf_File);
  WritePrivateProfileString ("General", "Dump path", Dump_Dir, Conf_File);
  WritePrivateProfileString ("General", "Dump GYM path", Dump_GYM_Dir,
			     Conf_File);
  WritePrivateProfileString ("General", "Screen Shot path", ScrShot_Dir,
			     Conf_File);
  WritePrivateProfileString ("General", "Patch path", Patch_Dir, Conf_File);
  WritePrivateProfileString ("General", "IPS Patch path", IPS_Dir, Conf_File);

  WritePrivateProfileString ("General", "Genesis Bios", Genesis_Bios,
			     Conf_File);

  WritePrivateProfileString ("General", "USA CD Bios", US_CD_Bios, Conf_File);
  WritePrivateProfileString ("General", "EUROPE CD Bios", EU_CD_Bios,
			     Conf_File);
  WritePrivateProfileString ("General", "JAPAN CD Bios", JA_CD_Bios,
			     Conf_File);

  WritePrivateProfileString ("General", "32X 68000 Bios", _32X_Genesis_Bios,
			     Conf_File);
  WritePrivateProfileString ("General", "32X Master SH2 Bios",
			     _32X_Master_Bios, Conf_File);
  WritePrivateProfileString ("General", "32X Slave SH2 Bios", _32X_Slave_Bios,
			     Conf_File);

  WritePrivateProfileString ("General", "Rom 1", Recent_Rom[0], Conf_File);
  WritePrivateProfileString ("General", "Rom 2", Recent_Rom[1], Conf_File);
  WritePrivateProfileString ("General", "Rom 3", Recent_Rom[2], Conf_File);
  WritePrivateProfileString ("General", "Rom 4", Recent_Rom[3], Conf_File);
  WritePrivateProfileString ("General", "Rom 5", Recent_Rom[4], Conf_File);
  WritePrivateProfileString ("General", "Rom 6", Recent_Rom[5], Conf_File);
  WritePrivateProfileString ("General", "Rom 7", Recent_Rom[6], Conf_File);
  WritePrivateProfileString ("General", "Rom 8", Recent_Rom[7], Conf_File);
  WritePrivateProfileString ("General", "Rom 9", Recent_Rom[8], Conf_File);

  WritePrivateProfileString ("General", "CD Drive", CDROM_DEV, Conf_File);
  sprintf (Str_Tmp, "%d", CDROM_SPEED);
  WritePrivateProfileString ("General", "CD Speed", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Current_State);
  WritePrivateProfileString ("General", "State Number", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Language);
  WritePrivateProfileString ("General", "Language", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Window_Pos.x);
  WritePrivateProfileString ("General", "Window X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Window_Pos.y);
  WritePrivateProfileString ("General", "Window Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Effect_Color);
  WritePrivateProfileString ("General", "Free Mode Color", Str_Tmp,
			     Conf_File);

  sprintf (Str_Tmp, "%d", Full_Screen & 1);
  WritePrivateProfileString ("Graphics", "Full Screen", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", FS_VSync & 1);
  WritePrivateProfileString ("Graphics", "Full Screen VSync", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", W_VSync & 1);
  WritePrivateProfileString ("Graphics", "Windows VSync", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Render_Mode);
  WritePrivateProfileString ("Graphics", "Render Mode", Str_Tmp,
			     Conf_File);
  sprintf(Str_Tmp, "%d", Bpp);
  WritePrivateProfileString("Graphics", "Bits Per Pixel", Str_Tmp, Conf_File);
  sprintf(Str_Tmp, "%d", Opengl & 1);
  WritePrivateProfileString("Graphics", "Render Opengl", Str_Tmp, Conf_File);
  sprintf(Str_Tmp, "%d", Width_gl);
  WritePrivateProfileString("Graphics", "Opengl Width", Str_Tmp, Conf_File);
  sprintf(Str_Tmp, "%d", Height_gl);
  WritePrivateProfileString("Graphics", "Opengl Height", Str_Tmp, Conf_File);			     
//  sprintf(Str_Tmp, "%d", gl_linear_filter);
//  WritePrivateProfileString("Graphics", "Opengl Filter", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Stretch & 1);  
  WritePrivateProfileString ("Graphics", "Stretch", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Blit_Soft & 1);
  WritePrivateProfileString ("Graphics", "Software Blit", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Contrast_Level);
  WritePrivateProfileString ("Graphics", "Contrast", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Brightness_Level);
  WritePrivateProfileString ("Graphics", "Brightness", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Greyscale & 1);
  WritePrivateProfileString ("Graphics", "Greyscale", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Invert_Color & 1);
  WritePrivateProfileString ("Graphics", "Invert", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Sprite_Over & 1);
  WritePrivateProfileString ("Graphics", "Sprite limit", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Frame_Skip);
  WritePrivateProfileString ("Graphics", "Frame skip", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Sound_Enable & 1);
  WritePrivateProfileString ("Sound", "State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Sound_Rate);
  WritePrivateProfileString ("Sound", "Rate", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Sound_Stereo);
  WritePrivateProfileString ("Sound", "Stereo", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Z80_State & 1);
  WritePrivateProfileString ("Sound", "Z80 State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", YM2612_Enable & 1);
  WritePrivateProfileString ("Sound", "YM2612 State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", PSG_Enable & 1);
  WritePrivateProfileString ("Sound", "PSG State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", DAC_Enable & 1);
  WritePrivateProfileString ("Sound", "DAC State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", PCM_Enable & 1);
  WritePrivateProfileString ("Sound", "PCM State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", PWM_Enable & 1);
  WritePrivateProfileString ("Sound", "PWM State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", CDDA_Enable & 1);
  WritePrivateProfileString ("Sound", "CDDA State", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", YM2612_Improv & 1);
  WritePrivateProfileString ("Sound", "YM2612 Improvement", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", DAC_Improv & 1);
  WritePrivateProfileString ("Sound", "DAC Improvement", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", PSG_Improv & 1);
  WritePrivateProfileString ("Sound", "PSG Improvement", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Country);
  WritePrivateProfileString ("CPU", "Country", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Country_Order[0]);
  WritePrivateProfileString ("CPU", "Prefered Country 1", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Country_Order[1]);
  WritePrivateProfileString ("CPU", "Prefered Country 2", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Country_Order[2]);
  WritePrivateProfileString ("CPU", "Prefered Country 3", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", SegaCD_Accurate);
  WritePrivateProfileString ("CPU",
			     "Perfect synchro between main and sub CPU (Sega CD)",
			     Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", MSH2_Speed);
  WritePrivateProfileString ("CPU", "Main SH2 Speed", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", SSH2_Speed);
  WritePrivateProfileString ("CPU", "Slave SH2 Speed", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Fast_Blur & 1);
  WritePrivateProfileString ("Options", "Fast Blur", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Show_FPS & 1);
  WritePrivateProfileString ("Options", "FPS", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", FPS_Style);
  WritePrivateProfileString ("Options", "FPS Style", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Show_Message & 1);
  WritePrivateProfileString ("Options", "Message", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Message_Style);
  WritePrivateProfileString ("Options", "Message Style", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Show_LED & 1);
  WritePrivateProfileString ("Options", "LED", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Auto_Fix_CS & 1);
  WritePrivateProfileString ("Options", "Auto Fix Checksum", Str_Tmp,
			     Conf_File);
  sprintf (Str_Tmp, "%d", Auto_Pause & 1);
  WritePrivateProfileString ("Options", "Auto Pause", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", CUR_DEV);
  WritePrivateProfileString ("Options", "CD Drive", Str_Tmp, Conf_File);

  if (BRAM_Ex_State & 0x100)
    {
      sprintf (Str_Tmp, "%d", BRAM_Ex_Size);
      WritePrivateProfileString ("Options", "Ram Cart Size", Str_Tmp,
				 Conf_File);
    }
  else
    {
      WritePrivateProfileString ("Options", "Ram Cart Size", "-1", Conf_File);
    }

  WritePrivateProfileString ("Options", "GCOffline path", CGOffline_Path,
			     Conf_File);
  WritePrivateProfileString ("Options", "Gens manual path", Manual_Path,
			     Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1_Type & 0x13);
  WritePrivateProfileString ("Input", "P1.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Up);
  WritePrivateProfileString ("Input", "P1.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Down);
  WritePrivateProfileString ("Input", "P1.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Left);
  WritePrivateProfileString ("Input", "P1.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Right);
  WritePrivateProfileString ("Input", "P1.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Start);
  WritePrivateProfileString ("Input", "P1.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].A);
  WritePrivateProfileString ("Input", "P1.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].B);
  WritePrivateProfileString ("Input", "P1.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].C);
  WritePrivateProfileString ("Input", "P1.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Mode);
  WritePrivateProfileString ("Input", "P1.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].X);
  WritePrivateProfileString ("Input", "P1.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Y);
  WritePrivateProfileString ("Input", "P1.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[0].Z);
  WritePrivateProfileString ("Input", "P1.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1B_Type & 0x03);
  WritePrivateProfileString ("Input", "P1B.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Up);
  WritePrivateProfileString ("Input", "P1B.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Down);
  WritePrivateProfileString ("Input", "P1B.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Left);
  WritePrivateProfileString ("Input", "P1B.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Right);
  WritePrivateProfileString ("Input", "P1B.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Start);
  WritePrivateProfileString ("Input", "P1B.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].A);
  WritePrivateProfileString ("Input", "P1B.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].B);
  WritePrivateProfileString ("Input", "P1B.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].C);
  WritePrivateProfileString ("Input", "P1B.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Mode);
  WritePrivateProfileString ("Input", "P1B.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].X);
  WritePrivateProfileString ("Input", "P1B.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Y);
  WritePrivateProfileString ("Input", "P1B.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[2].Z);
  WritePrivateProfileString ("Input", "P1B.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1C_Type & 0x03);
  WritePrivateProfileString ("Input", "P1C.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Up);
  WritePrivateProfileString ("Input", "P1C.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Down);
  WritePrivateProfileString ("Input", "P1C.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Left);
  WritePrivateProfileString ("Input", "P1C.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Right);
  WritePrivateProfileString ("Input", "P1C.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Start);
  WritePrivateProfileString ("Input", "P1C.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].A);
  WritePrivateProfileString ("Input", "P1C.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].B);
  WritePrivateProfileString ("Input", "P1C.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].C);
  WritePrivateProfileString ("Input", "P1C.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Mode);
  WritePrivateProfileString ("Input", "P1C.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].X);
  WritePrivateProfileString ("Input", "P1C.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Y);
  WritePrivateProfileString ("Input", "P1C.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[3].Z);
  WritePrivateProfileString ("Input", "P1C.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_1D_Type & 0x03);
  WritePrivateProfileString ("Input", "P1D.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Up);
  WritePrivateProfileString ("Input", "P1D.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Down);
  WritePrivateProfileString ("Input", "P1D.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Left);
  WritePrivateProfileString ("Input", "P1D.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Right);
  WritePrivateProfileString ("Input", "P1D.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Start);
  WritePrivateProfileString ("Input", "P1D.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].A);
  WritePrivateProfileString ("Input", "P1D.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].B);
  WritePrivateProfileString ("Input", "P1D.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].C);
  WritePrivateProfileString ("Input", "P1D.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Mode);
  WritePrivateProfileString ("Input", "P1D.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].X);
  WritePrivateProfileString ("Input", "P1D.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Y);
  WritePrivateProfileString ("Input", "P1D.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[4].Z);
  WritePrivateProfileString ("Input", "P1D.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2_Type & 0x13);
  WritePrivateProfileString ("Input", "P2.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Up);
  WritePrivateProfileString ("Input", "P2.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Down);
  WritePrivateProfileString ("Input", "P2.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Left);
  WritePrivateProfileString ("Input", "P2.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Right);
  WritePrivateProfileString ("Input", "P2.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Start);
  WritePrivateProfileString ("Input", "P2.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].A);
  WritePrivateProfileString ("Input", "P2.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].B);
  WritePrivateProfileString ("Input", "P2.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].C);
  WritePrivateProfileString ("Input", "P2.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Mode);
  WritePrivateProfileString ("Input", "P2.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].X);
  WritePrivateProfileString ("Input", "P2.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Y);
  WritePrivateProfileString ("Input", "P2.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[1].Z);
  WritePrivateProfileString ("Input", "P2.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2B_Type & 0x03);
  WritePrivateProfileString ("Input", "P2B.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Up);
  WritePrivateProfileString ("Input", "P2B.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Down);
  WritePrivateProfileString ("Input", "P2B.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Left);
  WritePrivateProfileString ("Input", "P2B.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Right);
  WritePrivateProfileString ("Input", "P2B.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Start);
  WritePrivateProfileString ("Input", "P2B.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].A);
  WritePrivateProfileString ("Input", "P2B.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].B);
  WritePrivateProfileString ("Input", "P2B.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].C);
  WritePrivateProfileString ("Input", "P2B.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Mode);
  WritePrivateProfileString ("Input", "P2B.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].X);
  WritePrivateProfileString ("Input", "P2B.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Y);
  WritePrivateProfileString ("Input", "P2B.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[5].Z);
  WritePrivateProfileString ("Input", "P2B.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2C_Type & 0x03);
  WritePrivateProfileString ("Input", "P2C.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Up);
  WritePrivateProfileString ("Input", "P2C.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Down);
  WritePrivateProfileString ("Input", "P2C.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Left);
  WritePrivateProfileString ("Input", "P2C.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Right);
  WritePrivateProfileString ("Input", "P2C.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Start);
  WritePrivateProfileString ("Input", "P2C.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].A);
  WritePrivateProfileString ("Input", "P2C.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].B);
  WritePrivateProfileString ("Input", "P2C.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].C);
  WritePrivateProfileString ("Input", "P2C.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Mode);
  WritePrivateProfileString ("Input", "P2C.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].X);
  WritePrivateProfileString ("Input", "P2C.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Y);
  WritePrivateProfileString ("Input", "P2C.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[6].Z);
  WritePrivateProfileString ("Input", "P2C.Z", Str_Tmp, Conf_File);

  sprintf (Str_Tmp, "%d", Controller_2D_Type & 0x03);
  WritePrivateProfileString ("Input", "P2D.Type", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Up);
  WritePrivateProfileString ("Input", "P2D.Up", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Down);
  WritePrivateProfileString ("Input", "P2D.Down", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Left);
  WritePrivateProfileString ("Input", "P2D.Left", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Right);
  WritePrivateProfileString ("Input", "P2D.Right", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Start);
  WritePrivateProfileString ("Input", "P2D.Start", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].A);
  WritePrivateProfileString ("Input", "P2D.A", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].B);
  WritePrivateProfileString ("Input", "P2D.B", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].C);
  WritePrivateProfileString ("Input", "P2D.C", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Mode);
  WritePrivateProfileString ("Input", "P2D.Mode", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].X);
  WritePrivateProfileString ("Input", "P2D.X", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Y);
  WritePrivateProfileString ("Input", "P2D.Y", Str_Tmp, Conf_File);
  sprintf (Str_Tmp, "%d", Keys_Def[7].Z);
  WritePrivateProfileString ("Input", "P2D.Z", Str_Tmp, Conf_File);

  return 1;
}



int
Save_As_Config ()
{
  char Name[2048];
  GtkWidget *widget;
  gint res;

  widget =
    create_file_chooser_dialog ("Save Config As",
				GTK_FILE_CHOOSER_ACTION_SAVE);
  addCfgFilter (widget);

  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
      char* filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
      strncpy (Name, filename, 2048);
      g_free(filename);
    }
    else {
    	strcpy(Name, "\n");
    }
  gtk_widget_destroy (widget);
  if (res == GTK_RESPONSE_CANCEL)
  	return 0;

  if (strlen (Name))
    {
      Save_Config (Name);
      strcpy (Str_Tmp, "config saved in ");
      strcat (Str_Tmp, Name);
      Put_Info (Str_Tmp, 2000);
      return 1;
    }
  return 0;
}


int
Load_Config (char *File_Name, void *Game_Active)
{
  int new_val;
  char Conf_File[1024];
  char Home_Dir[1024];
  strncpy(Home_Dir, getenv("HOME"), 900);
  strcat(Home_Dir, "/");

  //SetCurrentDirectory (Gens_Path);
  strcpy (Conf_File, File_Name);

  CRam_Flag = 1;

  GetPrivateProfileString ("General", "Rom path", Home_Dir, &Rom_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "Save path", Gens_Path, &State_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "SRAM path", Gens_Path, &SRAM_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "BRAM path", Gens_Path, &BRAM_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "Dump path", Gens_Path, &Dump_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "Dump GYM path", Gens_Path,
			   &Dump_GYM_Dir[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "Screen Shot path", Gens_Path,
			   &ScrShot_Dir[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "Patch path", Gens_Path, &Patch_Dir[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "IPS Patch path", Gens_Path, &IPS_Dir[0],
			   1024, Conf_File);

  GetPrivateProfileString ("General", "Genesis Bios", Gens_Path,
			   &Genesis_Bios[0], 1024, Conf_File);

  GetPrivateProfileString ("General", "USA CD Bios", Gens_Path, &US_CD_Bios[0],
			   1024, Conf_File);
  GetPrivateProfileString ("General", "EUROPE CD Bios", Gens_Path,
			   &EU_CD_Bios[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "JAPAN CD Bios", Gens_Path,
			   &JA_CD_Bios[0], 1024, Conf_File);

  GetPrivateProfileString ("General", "32X 68000 Bios", Gens_Path,
			   &_32X_Genesis_Bios[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "32X Master SH2 Bios", Gens_Path,
			   &_32X_Master_Bios[0], 1024, Conf_File);
  GetPrivateProfileString ("General", "32X Slave SH2 Bios", Gens_Path,
			   &_32X_Slave_Bios[0], 1024, Conf_File);

  GetPrivateProfileString ("General", "Rom 1", "", &Recent_Rom[0][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 2", "", &Recent_Rom[1][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 3", "", &Recent_Rom[2][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 4", "", &Recent_Rom[3][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 5", "", &Recent_Rom[4][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 6", "", &Recent_Rom[5][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 7", "", &Recent_Rom[6][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 8", "", &Recent_Rom[7][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "Rom 9", "", &Recent_Rom[8][0], 1024,
			   Conf_File);
  GetPrivateProfileString ("General", "CD Drive", Rom_Dir, &CDROM_DEV[0],
			   16, Conf_File);
  CDROM_SPEED =   GetPrivateProfileInt ("General", "CD Speed", 0, Conf_File);
  Current_State =
    GetPrivateProfileInt ("General", "State Number", 0, Conf_File);
  Language = GetPrivateProfileInt ("General", "Language", 0, Conf_File);
  Window_Pos.x = GetPrivateProfileInt ("General", "Window X", 0, Conf_File);
  Window_Pos.y = GetPrivateProfileInt ("General", "Window Y", 0, Conf_File);
  Intro_Style = GetPrivateProfileInt ("General", "Intro Style", 0, Conf_File);
  Effect_Color =
    GetPrivateProfileInt ("General", "Free Mode Color", 7, Conf_File);
  Sleep_Time =
    GetPrivateProfileInt ("General", "Allow Idle", 0, Conf_File) & 1;

  if (GetPrivateProfileInt ("Graphics", "Force 555", 0, Conf_File))
    Mode_555 = 3;
  else if (GetPrivateProfileInt ("Graphics", "Force 565", 0, Conf_File))
    Mode_555 = 2;
  else
    Mode_555 = 0;

  RMax_Level = GetPrivateProfileInt ("Graphics", "Red Max", 255, Conf_File);
  GMax_Level = GetPrivateProfileInt ("Graphics", "Green Max", 255, Conf_File);
  BMax_Level = GetPrivateProfileInt ("Graphics", "Blue Max", 255, Conf_File);
  Contrast_Level =
    GetPrivateProfileInt ("Graphics", "Contrast", 100, Conf_File);
  Brightness_Level =
    GetPrivateProfileInt ("Graphics", "Brightness", 100, Conf_File);
  Greyscale = GetPrivateProfileInt ("Graphics", "Greyscale", 0, Conf_File);
  Invert_Color = GetPrivateProfileInt ("Graphics", "Invert", 0, Conf_File);

  Recalculate_Palettes ();

  FS_VSync =
    GetPrivateProfileInt ("Graphics", "Full Screen VSync", 0, Conf_File);
  W_VSync = GetPrivateProfileInt ("Graphics", "Windows VSync", 0, Conf_File);
  Full_Screen =
    GetPrivateProfileInt ("Graphics", "Full Screen", 0, Conf_File);
  Render_Mode =
    GetPrivateProfileInt ("Graphics", "Render Mode", 1, Conf_File);
  Bpp = GetPrivateProfileInt("Graphics", "Bits Per Pixel", 16, Conf_File);
  Opengl = GetPrivateProfileInt("Graphics", "Render Opengl", 0, Conf_File);
  Width_gl = GetPrivateProfileInt("Graphics", "Opengl Width", 640, Conf_File);
  Height_gl = GetPrivateProfileInt("Graphics", "Opengl Height", 480, Conf_File);
//  gl_linear_filter = GetPrivateProfileInt("Graphics", "Opengl Filter", 1, Conf_File);
  //Set_Render (Full_Screen, -1, 1);

  Stretch = GetPrivateProfileInt ("Graphics", "Stretch", 0, Conf_File);
  Blit_Soft =
    GetPrivateProfileInt ("Graphics", "Software Blit", 0, Conf_File);
  Sprite_Over =
    GetPrivateProfileInt ("Graphics", "Sprite limit", 1, Conf_File);
  Frame_Skip = GetPrivateProfileInt ("Graphics", "Frame skip", -1, Conf_File);

  Sound_Rate = GetPrivateProfileInt ("Sound", "Rate", 22050, Conf_File);
  Sound_Stereo = GetPrivateProfileInt ("Sound", "Stereo", 1, Conf_File);

  if (GetPrivateProfileInt ("Sound", "Z80 State", 1, Conf_File))
    Z80_State |= 1;
  else
    Z80_State &= ~1;

  new_val = GetPrivateProfileInt ("Sound", "State", 1, Conf_File);
  if (new_val != Sound_Enable)
    {
      if (Change_Sound ())
	{
	  YM2612_Enable =
	    GetPrivateProfileInt ("Sound", "YM2612 State", 1, Conf_File);
	  PSG_Enable =
	    GetPrivateProfileInt ("Sound", "PSG State", 1, Conf_File);
	  DAC_Enable =
	    GetPrivateProfileInt ("Sound", "DAC State", 1, Conf_File);
	  PCM_Enable =
	    GetPrivateProfileInt ("Sound", "PCM State", 1, Conf_File);
	  PWM_Enable =
	    GetPrivateProfileInt ("Sound", "PWM State", 1, Conf_File);
	  CDDA_Enable =
	    GetPrivateProfileInt ("Sound", "CDDA State", 1, Conf_File);
	}
    }
  else
    {
      YM2612_Enable =
	GetPrivateProfileInt ("Sound", "YM2612 State", 1, Conf_File);
      PSG_Enable = GetPrivateProfileInt ("Sound", "PSG State", 1, Conf_File);
      DAC_Enable = GetPrivateProfileInt ("Sound", "DAC State", 1, Conf_File);
      PCM_Enable = GetPrivateProfileInt ("Sound", "PCM State", 1, Conf_File);
      PWM_Enable = GetPrivateProfileInt ("Sound", "PWM State", 1, Conf_File);
      CDDA_Enable =
	GetPrivateProfileInt ("Sound", "CDDA State", 1, Conf_File);
    }

  YM2612_Improv =
    GetPrivateProfileInt ("Sound", "YM2612 Improvement", 0, Conf_File);
  DAC_Improv =
    GetPrivateProfileInt ("Sound", "DAC Improvement", 0, Conf_File);
  PSG_Improv =
    GetPrivateProfileInt ("Sound", "PSG Improvement", 0, Conf_File);

  Country = GetPrivateProfileInt ("CPU", "Country", -1, Conf_File);
  Country_Order[0] =
    GetPrivateProfileInt ("CPU", "Prefered Country 1", 0, Conf_File);
  Country_Order[1] =
    GetPrivateProfileInt ("CPU", "Prefered Country 2", 1, Conf_File);
  Country_Order[2] =
    GetPrivateProfileInt ("CPU", "Prefered Country 3", 2, Conf_File);

  SegaCD_Accurate =
    GetPrivateProfileInt ("CPU",
			  "Perfect synchro between main and sub CPU (Sega CD)",
			  0, Conf_File);

  MSH2_Speed = GetPrivateProfileInt ("CPU", "Main SH2 Speed", 100, Conf_File);
  SSH2_Speed =
    GetPrivateProfileInt ("CPU", "Slave SH2 Speed", 100, Conf_File);

  if (MSH2_Speed < 0)
    MSH2_Speed = 0;
  if (SSH2_Speed < 0)
    SSH2_Speed = 0;

  Check_Country_Order ();

  Fast_Blur = GetPrivateProfileInt ("Options", "Fast Blur", 0, Conf_File);
  Show_FPS = GetPrivateProfileInt ("Options", "FPS", 0, Conf_File);
  FPS_Style = GetPrivateProfileInt ("Options", "FPS Style", 0, Conf_File);
  Show_Message = GetPrivateProfileInt ("Options", "Message", 1, Conf_File);
  Message_Style =
    GetPrivateProfileInt ("Options", "Message Style", 0, Conf_File);
  Show_LED = GetPrivateProfileInt ("Options", "LED", 1, Conf_File);
  Auto_Fix_CS =
    GetPrivateProfileInt ("Options", "Auto Fix Checksum", 0, Conf_File);
  Auto_Pause = GetPrivateProfileInt ("Options", "Auto Pause", 0, Conf_File);
  CUR_DEV = GetPrivateProfileInt ("Options", "CD Drive", 0, Conf_File);
  BRAM_Ex_Size =
    GetPrivateProfileInt ("Options", "Ram Cart Size", 3, Conf_File);

  if (BRAM_Ex_Size == -1)
    {
      BRAM_Ex_State &= 1;
      BRAM_Ex_Size = 0;
    }
  else
    BRAM_Ex_State |= 0x100;

  GetPrivateProfileString ("Options", "GCOffline path", "GCOffline.chm",
			   CGOffline_Path, 1024, Conf_File);
  GetPrivateProfileString ("Options", "Gens manual path", "manual.exe",
			   Manual_Path, 1024, Conf_File);

  Controller_1_Type = GetPrivateProfileInt ("Input", "P1.Type", 1, Conf_File);
  Keys_Def[0].Up = GetPrivateProfileInt ("Input", "P1.Up", DIK_UP, Conf_File);
  Keys_Def[0].Down =
    GetPrivateProfileInt ("Input", "P1.Down", DIK_DOWN, Conf_File);
  Keys_Def[0].Left =
    GetPrivateProfileInt ("Input", "P1.Left", DIK_LEFT, Conf_File);
  Keys_Def[0].Right =
    GetPrivateProfileInt ("Input", "P1.Right", DIK_RIGHT, Conf_File);
  Keys_Def[0].Start =
    GetPrivateProfileInt ("Input", "P1.Start", DIK_RETURN, Conf_File);
  Keys_Def[0].A = GetPrivateProfileInt ("Input", "P1.A", DIK_A, Conf_File);
  Keys_Def[0].B = GetPrivateProfileInt ("Input", "P1.B", DIK_S, Conf_File);
  Keys_Def[0].C = GetPrivateProfileInt ("Input", "P1.C", DIK_D, Conf_File);
  Keys_Def[0].Mode =
    GetPrivateProfileInt ("Input", "P1.Mode", DIK_RSHIFT, Conf_File);
  Keys_Def[0].X = GetPrivateProfileInt ("Input", "P1.X", DIK_Z, Conf_File);
  Keys_Def[0].Y = GetPrivateProfileInt ("Input", "P1.Y", DIK_X, Conf_File);
  Keys_Def[0].Z = GetPrivateProfileInt ("Input", "P1.Z", DIK_C, Conf_File);

  Controller_1B_Type =
    GetPrivateProfileInt ("Input", "P1B.Type", 0, Conf_File);
  Keys_Def[2].Up = GetPrivateProfileInt ("Input", "P1B.Up", 0, Conf_File);
  Keys_Def[2].Down = GetPrivateProfileInt ("Input", "P1B.Down", 0, Conf_File);
  Keys_Def[2].Left = GetPrivateProfileInt ("Input", "P1B.Left", 0, Conf_File);
  Keys_Def[2].Right =
    GetPrivateProfileInt ("Input", "P1B.Right", 0, Conf_File);
  Keys_Def[2].Start =
    GetPrivateProfileInt ("Input", "P1B.Start", 0, Conf_File);
  Keys_Def[2].A = GetPrivateProfileInt ("Input", "P1B.A", 0, Conf_File);
  Keys_Def[2].B = GetPrivateProfileInt ("Input", "P1B.B", 0, Conf_File);
  Keys_Def[2].C = GetPrivateProfileInt ("Input", "P1B.C", 0, Conf_File);
  Keys_Def[2].Mode = GetPrivateProfileInt ("Input", "P1B.Mode", 0, Conf_File);
  Keys_Def[2].X = GetPrivateProfileInt ("Input", "P1B.X", 0, Conf_File);
  Keys_Def[2].Y = GetPrivateProfileInt ("Input", "P1B.Y", 0, Conf_File);
  Keys_Def[2].Z = GetPrivateProfileInt ("Input", "P1B.Z", 0, Conf_File);

  Controller_1C_Type =
    GetPrivateProfileInt ("Input", "P1C.Type", 0, Conf_File);
  Keys_Def[3].Up = GetPrivateProfileInt ("Input", "P1C.Up", 0, Conf_File);
  Keys_Def[3].Down = GetPrivateProfileInt ("Input", "P1C.Down", 0, Conf_File);
  Keys_Def[3].Left = GetPrivateProfileInt ("Input", "P1C.Left", 0, Conf_File);
  Keys_Def[3].Right =
    GetPrivateProfileInt ("Input", "P1C.Right", 0, Conf_File);
  Keys_Def[3].Start =
    GetPrivateProfileInt ("Input", "P1C.Start", 0, Conf_File);
  Keys_Def[3].A = GetPrivateProfileInt ("Input", "P1C.A", 0, Conf_File);
  Keys_Def[3].B = GetPrivateProfileInt ("Input", "P1C.B", 0, Conf_File);
  Keys_Def[3].C = GetPrivateProfileInt ("Input", "P1C.C", 0, Conf_File);
  Keys_Def[3].Mode = GetPrivateProfileInt ("Input", "P1C.Mode", 0, Conf_File);
  Keys_Def[3].X = GetPrivateProfileInt ("Input", "P1C.X", 0, Conf_File);
  Keys_Def[3].Y = GetPrivateProfileInt ("Input", "P1C.Y", 0, Conf_File);
  Keys_Def[3].Z = GetPrivateProfileInt ("Input", "P1C.Z", 0, Conf_File);

  Controller_1D_Type =
    GetPrivateProfileInt ("Input", "P1D.Type", 0, Conf_File);
  Keys_Def[4].Up = GetPrivateProfileInt ("Input", "P1D.Up", 0, Conf_File);
  Keys_Def[4].Down = GetPrivateProfileInt ("Input", "P1D.Down", 0, Conf_File);
  Keys_Def[4].Left = GetPrivateProfileInt ("Input", "P1D.Left", 0, Conf_File);
  Keys_Def[4].Right =
    GetPrivateProfileInt ("Input", "P1D.Right", 0, Conf_File);
  Keys_Def[4].Start =
    GetPrivateProfileInt ("Input", "P1D.Start", 0, Conf_File);
  Keys_Def[4].A = GetPrivateProfileInt ("Input", "P1D.A", 0, Conf_File);
  Keys_Def[4].B = GetPrivateProfileInt ("Input", "P1D.B", 0, Conf_File);
  Keys_Def[4].C = GetPrivateProfileInt ("Input", "P1D.C", 0, Conf_File);
  Keys_Def[4].Mode = GetPrivateProfileInt ("Input", "P1D.Mode", 0, Conf_File);
  Keys_Def[4].X = GetPrivateProfileInt ("Input", "P1D.X", 0, Conf_File);
  Keys_Def[4].Y = GetPrivateProfileInt ("Input", "P1D.Y", 0, Conf_File);
  Keys_Def[4].Z = GetPrivateProfileInt ("Input", "P1D.Z", 0, Conf_File);

  Controller_2_Type = GetPrivateProfileInt ("Input", "P2.Type", 1, Conf_File);
  Keys_Def[1].Up = GetPrivateProfileInt ("Input", "P2.Up", DIK_Y, Conf_File);
  Keys_Def[1].Down =
    GetPrivateProfileInt ("Input", "P2.Down", DIK_H, Conf_File);
  Keys_Def[1].Left =
    GetPrivateProfileInt ("Input", "P2.Left", DIK_G, Conf_File);
  Keys_Def[1].Right =
    GetPrivateProfileInt ("Input", "P2.Right", DIK_J, Conf_File);
  Keys_Def[1].Start =
    GetPrivateProfileInt ("Input", "P2.Start", DIK_U, Conf_File);
  Keys_Def[1].A = GetPrivateProfileInt ("Input", "P2.A", DIK_K, Conf_File);
  Keys_Def[1].B = GetPrivateProfileInt ("Input", "P2.B", DIK_L, Conf_File);
  Keys_Def[1].C = GetPrivateProfileInt ("Input", "P2.C", DIK_M, Conf_File);
  Keys_Def[1].Mode =
    GetPrivateProfileInt ("Input", "P2.Mode", DIK_T, Conf_File);
  Keys_Def[1].X = GetPrivateProfileInt ("Input", "P2.X", DIK_I, Conf_File);
  Keys_Def[1].Y = GetPrivateProfileInt ("Input", "P2.Y", DIK_O, Conf_File);
  Keys_Def[1].Z = GetPrivateProfileInt ("Input", "P2.Z", DIK_P, Conf_File);

  Controller_2B_Type =
    GetPrivateProfileInt ("Input", "P2B.Type", 0, Conf_File);
  Keys_Def[5].Up = GetPrivateProfileInt ("Input", "P2B.Up", 0, Conf_File);
  Keys_Def[5].Down = GetPrivateProfileInt ("Input", "P2B.Down", 0, Conf_File);
  Keys_Def[5].Left = GetPrivateProfileInt ("Input", "P2B.Left", 0, Conf_File);
  Keys_Def[5].Right =
    GetPrivateProfileInt ("Input", "P2B.Right", 0, Conf_File);
  Keys_Def[5].Start =
    GetPrivateProfileInt ("Input", "P2B.Start", 0, Conf_File);
  Keys_Def[5].A = GetPrivateProfileInt ("Input", "P2B.A", 0, Conf_File);
  Keys_Def[5].B = GetPrivateProfileInt ("Input", "P2B.B", 0, Conf_File);
  Keys_Def[5].C = GetPrivateProfileInt ("Input", "P2B.C", 0, Conf_File);
  Keys_Def[5].Mode = GetPrivateProfileInt ("Input", "P2B.Mode", 0, Conf_File);
  Keys_Def[5].X = GetPrivateProfileInt ("Input", "P2B.X", 0, Conf_File);
  Keys_Def[5].Y = GetPrivateProfileInt ("Input", "P2B.Y", 0, Conf_File);
  Keys_Def[5].Z = GetPrivateProfileInt ("Input", "P2B.Z", 0, Conf_File);

  Controller_2C_Type =
    GetPrivateProfileInt ("Input", "P2C.Type", 0, Conf_File);
  Keys_Def[6].Up = GetPrivateProfileInt ("Input", "P2C.Up", 0, Conf_File);
  Keys_Def[6].Down = GetPrivateProfileInt ("Input", "P2C.Down", 0, Conf_File);
  Keys_Def[6].Left = GetPrivateProfileInt ("Input", "P2C.Left", 0, Conf_File);
  Keys_Def[6].Right =
    GetPrivateProfileInt ("Input", "P2C.Right", 0, Conf_File);
  Keys_Def[6].Start =
    GetPrivateProfileInt ("Input", "P2C.Start", 0, Conf_File);
  Keys_Def[6].A = GetPrivateProfileInt ("Input", "P2C.A", 0, Conf_File);
  Keys_Def[6].B = GetPrivateProfileInt ("Input", "P2C.B", 0, Conf_File);
  Keys_Def[6].C = GetPrivateProfileInt ("Input", "P2C.C", 0, Conf_File);
  Keys_Def[6].Mode = GetPrivateProfileInt ("Input", "P2C.Mode", 0, Conf_File);
  Keys_Def[6].X = GetPrivateProfileInt ("Input", "P2C.X", 0, Conf_File);
  Keys_Def[6].Y = GetPrivateProfileInt ("Input", "P2C.Y", 0, Conf_File);
  Keys_Def[6].Z = GetPrivateProfileInt ("Input", "P2C.Z", 0, Conf_File);

  Controller_2D_Type =
    GetPrivateProfileInt ("Input", "P2D.Type", 0, Conf_File);
  Keys_Def[7].Up = GetPrivateProfileInt ("Input", "P2D.Up", 0, Conf_File);
  Keys_Def[7].Down = GetPrivateProfileInt ("Input", "P2D.Down", 0, Conf_File);
  Keys_Def[7].Left = GetPrivateProfileInt ("Input", "P2D.Left", 0, Conf_File);
  Keys_Def[7].Right =
    GetPrivateProfileInt ("Input", "P2D.Right", 0, Conf_File);
  Keys_Def[7].Start =
    GetPrivateProfileInt ("Input", "P2D.Start", 0, Conf_File);
  Keys_Def[7].A = GetPrivateProfileInt ("Input", "P2D.A", 0, Conf_File);
  Keys_Def[7].B = GetPrivateProfileInt ("Input", "P2D.B", 0, Conf_File);
  Keys_Def[7].C = GetPrivateProfileInt ("Input", "P2D.C", 0, Conf_File);
  Keys_Def[7].Mode = GetPrivateProfileInt ("Input", "P2D.Mode", 0, Conf_File);
  Keys_Def[7].X = GetPrivateProfileInt ("Input", "P2D.X", 0, Conf_File);
  Keys_Def[7].Y = GetPrivateProfileInt ("Input", "P2D.Y", 0, Conf_File);
  Keys_Def[7].Z = GetPrivateProfileInt ("Input", "P2D.Z", 0, Conf_File);

  Make_IO_Table ();
  return 1;
}


int
Load_As_Config (void *Game_Active)
{
  GtkWidget *widget;
  gint res;
  widget = create_file_chooser_dialog ("Load Config", GTK_FILE_CHOOSER_ACTION_OPEN);
  addCfgFilter (widget);
  res = gtk_dialog_run (GTK_DIALOG (widget));
  if (res == GTK_RESPONSE_OK)
    {
  	  char* filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));

  if (res == GTK_RESPONSE_CANCEL)
  	return 0;
  if (strlen (filename))
    {
      Load_Config (filename, Game_Active);
      strcpy (Str_Tmp, "config loaded from ");
      strcat (Str_Tmp, filename);
      Put_Info (Str_Tmp, 2000);
      g_free (filename);
      return 1;
    }
   }
  gtk_widget_destroy (widget);
  return 0;
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
