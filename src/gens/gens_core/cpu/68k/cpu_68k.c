#include <stdio.h>
#include <string.h>
#include "cpu_68k.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_m68k_32x.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"

#include "util/file/save.hpp"
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/gfx/gfx_cd.h"

#include "gens_core/io/io.h"
#include "segacd/cd_sys.hpp"

#define GENESIS 0
#define _32X    1
#define SEGACD  2


/** global variables **/

// TODO: This is not 64-bit clean!

struct S68000CONTEXT Context_68K;

/** Main 68000 **/

struct STARSCREAM_PROGRAMREGION M68K_Fetch[] =
{
	// RAM, including mirrors.
	{0xE00000, 0xE0FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE00000},
	{0xE10000, 0xE1FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE10000},
	{0xE20000, 0xE2FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE20000},
	{0xE30000, 0xE3FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE30000},
	{0xE40000, 0xE4FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE40000},
	{0xE50000, 0xE5FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE50000},
	{0xE60000, 0xE6FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE60000},
	{0xE70000, 0xE7FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE70000},
	{0xE80000, 0xE8FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE80000},
	{0xE90000, 0xE9FFFF, (unsigned int)&Ram_68k.u8[0] - 0xE90000},
	{0xEA0000, 0xEAFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEA0000},
	{0xEB0000, 0xEBFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEB0000},
	{0xEC0000, 0xECFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEC0000},
	{0xED0000, 0xEDFFFF, (unsigned int)&Ram_68k.u8[0] - 0xED0000},
	{0xEE0000, 0xEEFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEE0000},
	{0xEF0000, 0xEFFFFF, (unsigned int)&Ram_68k.u8[0] - 0xEF0000},
	{0xF00000, 0xF0FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF00000},
	{0xF10000, 0xF1FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF10000},
	{0xF20000, 0xF2FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF20000},
	{0xF30000, 0xF3FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF30000},
	{0xF40000, 0xF4FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF40000},
	{0xF50000, 0xF5FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF50000},
	{0xF60000, 0xF6FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF60000},
	{0xF70000, 0xF7FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF70000},
	{0xF80000, 0xF8FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF80000},
	{0xF90000, 0xF9FFFF, (unsigned int)&Ram_68k.u8[0] - 0xF90000},
	{0xFA0000, 0xFAFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFA0000},
	{0xFB0000, 0xFBFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFB0000},
	{0xFC0000, 0xFCFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFC0000},
	{0xFD0000, 0xFDFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFD0000},
	{0xFE0000, 0xFEFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFE0000},
	{0xFF0000, 0xFFFFFF, (unsigned int)&Ram_68k.u8[0] - 0xFF0000},
	
	// The following four entries are available for the various different systems.
	{-1, -1, (unsigned int)NULL},	// 32
	{-1, -1, (unsigned int)NULL},	// 33
	{-1, -1, (unsigned int)NULL},	// 34
	{-1, -1, (unsigned int)NULL},	// 35
	
	// Terminator.
	{-1, -1, (unsigned int)NULL}
};

// M68K Starscream has a hack for RAM mirroring for data read.
struct STARSCREAM_DATAREGION M68K_Read_Byte[5] =
{
	{0x000000, 0x3FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x400000, 0xFEFFFF, (void*)M68K_RB, NULL},
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data read.
struct STARSCREAM_DATAREGION M68K_Read_Word[5] =
{
	{0x000000, 0x3FFFFF, NULL, NULL},
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x400000, 0xFEFFFF, (void*)M68K_RW, NULL},
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data write.
struct STARSCREAM_DATAREGION M68K_Write_Byte[] =
{
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x000000, 0xFEFFFF, (void*)M68K_WB, NULL},
	{-1, -1, NULL, NULL}
};

// M68K Starscream has a hack for RAM mirroring for data write.
struct STARSCREAM_DATAREGION M68K_Write_Word[] =
{
	{0xFF0000, 0xFFFFFF, NULL, &Ram_68k.u8[0]},
	{0x000000, 0xFEFFFF, (void*)M68K_WW, NULL},
	{-1, -1, NULL, NULL}
};

/** Sub 68000 **/

struct STARSCREAM_PROGRAMREGION S68K_Fetch[] =
{
	{0x000000, 0x07FFFF, (unsigned int)&Ram_Prg.u8[0]},
	{-1, -1, (unsigned int)NULL},
	{-1, -1, (unsigned int)NULL}
};

struct STARSCREAM_DATAREGION S68K_Read_Byte[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg.u8[0]},
	{0x080000, 0xFFFFFF, (void*)S68K_RB, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION S68K_Read_Word[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg.u8[0]},
	{0x080000, 0xFFFFFF, (void*)S68K_RW, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION S68K_Write_Byte[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg.u8[0]},
	{0x080000, 0xFFFFFF, (void*) S68K_WB, NULL},
	{-1, -1, NULL, NULL}
};

struct STARSCREAM_DATAREGION S68K_Write_Word[] =
{
	{0x000000, 0x07FFFF, NULL, &Ram_Prg.u8[0]},
	{0x080000, 0xFFFFFF, (void*) S68K_WW, NULL},
	{-1, -1, NULL, NULL}
};


static void M68K_Reset_Handler(void)
{
	//Init_Memory_M68K(GENESIS);
}


static void S68K_Reset_Handler(void)
{
	//Init_Memory_M68K(SEGACD);
}


/**
 * M68K_Init(): Initialize the Main 68000.
 * @return 1 on success; 0 on error.
 */
int M68K_Init(void)
{
	memset(&Context_68K, 0, sizeof(Context_68K));
	
	Context_68K.s_fetch = Context_68K.u_fetch =
		Context_68K.fetch = M68K_Fetch;
	
	Context_68K.s_readbyte = Context_68K.u_readbyte =
		Context_68K.readbyte = M68K_Read_Byte;
	
	Context_68K.s_readword = Context_68K.u_readword =
		Context_68K.readword = M68K_Read_Word;
	
	Context_68K.s_writebyte = Context_68K.u_writebyte =
		Context_68K.writebyte = M68K_Write_Byte;
	
	Context_68K.s_writeword = Context_68K.u_writeword =
		Context_68K.writeword = M68K_Write_Word;
	
	Context_68K.resethandler = M68K_Reset_Handler;
	
	main68k_SetContext(&Context_68K);
	main68k_init();
	
	return 1;
}


/**
 * S68K_Init(): Initialize the Sub 68000.
 * @return 1 on success; 0 on error.
 */
int S68K_Init(void)
{
	memset(&Context_68K, 0, sizeof(Context_68K));
	
	Context_68K.s_fetch = Context_68K.u_fetch =
		Context_68K.fetch = S68K_Fetch;
	
	Context_68K.s_readbyte = Context_68K.u_readbyte =
		Context_68K.readbyte = S68K_Read_Byte;
	
	Context_68K.s_readword = Context_68K.u_readword =
		Context_68K.readword = S68K_Read_Word;
	
	Context_68K.s_writebyte = Context_68K.u_writebyte =
		Context_68K.writebyte = S68K_Write_Byte;
	
	Context_68K.s_writeword = Context_68K.u_writeword =
		Context_68K.writeword = S68K_Write_Word;
	
	Context_68K.resethandler = S68K_Reset_Handler;
	
	sub68k_SetContext(&Context_68K);
	sub68k_init();
	
	return 1;
}


/**
 * M68K_Reset(): General reset of the Main 68000 CPU.
 * @param System_ID System ID. [TODO: Make this an enum and/or a bitfield.]
 */
void M68K_Reset(int System_ID)
{
	// TODO: This is not 64-bit clean!
	
	// Clear M68K RAM.
	memset(Ram_68k.u8, 0x00, sizeof(Ram_68k));
	
	// Set the ROM fetch.
	M68K_Fetch[32].lowaddr  = 0x000000;
	M68K_Fetch[32].highaddr = (Rom_Size - 1);
	M68K_Fetch[32].offset   = (unsigned int)&Rom_Data.u8[0] - 0x000000;
	
	if (System_ID == GENESIS)
	{
		// Nothing else is required. Terminate the list.
		M68K_Fetch[33].lowaddr  = -1;
		M68K_Fetch[33].highaddr = -1;
		M68K_Fetch[33].offset   = (unsigned int)NULL;
	}
	else if (System_ID == _32X)
	{
		Bank_SH2 = 0;
		
		// Nothing else is required. Terminate the list.
		M68K_Fetch[33].lowaddr  = -1;
		M68K_Fetch[33].highaddr = -1;
		M68K_Fetch[33].offset   = (unsigned int)NULL;
	}
	else if (System_ID == SEGACD)
	{
		Bank_M68K = 0;
		
		// Set up Word RAM. (Entry 33)
		MS68K_Set_Word_Ram();
		
		// Set up Program RAM. (Entry 34)
		M68K_Fetch[34].lowaddr = 0x020000;
		M68K_Fetch[34].highaddr = 0x03FFFF;
		M68K_Set_Prg_Ram();
		
		// Terminate the list.
		M68K_Fetch[35].lowaddr = -1;
		M68K_Fetch[35].highaddr = -1;
		M68K_Fetch[35].offset = (unsigned int)NULL;
	}
	
	main68k_reset();
	Init_Memory_M68K(System_ID);
}


/**
 * M68K_Reset(): General reset of the Sub 68000 CPU.
 */
void S68K_Reset(void)
{
	memset(Ram_Prg.u8,     0x00, sizeof(Ram_Prg));
	memset(Ram_Word_2M.u8, 0x00, sizeof(Ram_Word_2M));
	memset(Ram_Word_1M.u8, 0x00, sizeof(Ram_Word_1M));
	
	memset(COMM.Command, 0x00, sizeof(COMM.Command));
	memset(COMM.Status,  0x00, sizeof(COMM.Status));
	
	LED_Status = S68K_State = S68K_Mem_WP = S68K_Mem_PM = Ram_Word_State = 0;
	COMM.Flag = Init_Timer_INT3 = Timer_INT3 = Int_Mask_S68K = 0;
	Font_COLOR = Font_BITS = 0;
	
	MS68K_Set_Word_Ram();
	sub68k_reset();
}


/**
 * M68K_32X_Mode(): Modify 32X mode.
 * This function is only called during 32X emulation.
 */
void M68K_32X_Mode(void)
{
	//if (_32X_ADEN && !_32X_RV) // 32X ON
	if (_32X_ADEN)
	{
		// 32X ON
		if (!_32X_RV)
		{
			// 32X Firmware: Genesis ROM.
			M68K_Fetch[32].lowaddr = 0x00;
			M68K_Fetch[32].highaddr = 0xFF;
			M68K_Fetch[32].offset = (unsigned int)&_32X_Genesis_Rom[0] - 0x000000;
			
			// ROM MOVED
			M68K_Fetch[33].lowaddr = 0x880000;
			M68K_Fetch[33].highaddr = 0x8FFFFF;
			M68K_Fetch[33].offset = (unsigned int)&Rom_Data.u8[0] - 0x880000;
			
			M68K_Fetch[34].lowaddr = 0x900000;
			M68K_Fetch[34].highaddr = 0x9FFFFF;
			M68K_Set_32X_Rom_Bank();
			
			M68K_Fetch[35].lowaddr = -1;
			M68K_Fetch[35].highaddr = -1;
			M68K_Fetch[35].offset = (unsigned int)NULL;
			
			M68K_Read_Byte_Table[0] = _32X_M68K_Read_Byte_Table[4 * 2];
			M68K_Read_Word_Table[0] = _32X_M68K_Read_Word_Table[4 * 2];
		}
		else
		{
			// 32X Firmware: Genesis ROM.
			M68K_Fetch[32].lowaddr = 0x00;
			M68K_Fetch[32].highaddr = 0xFF;
			M68K_Fetch[32].offset = (unsigned int)&_32X_Genesis_Rom[0] - 0x000000;
			
			// ROM NOT MOVED BUT BIOS PRESENT
			M68K_Fetch[33].lowaddr = 0x000100;
			M68K_Fetch[33].highaddr = Rom_Size - 1;
			M68K_Fetch[33].offset = (unsigned int)&Rom_Data.u8[0] - 0x000000;
			
			M68K_Fetch[34].lowaddr = -1;
			M68K_Fetch[34].highaddr = -1;
			M68K_Fetch[34].offset = (unsigned int)NULL;
			
			M68K_Read_Byte_Table[0] = _32X_M68K_Read_Byte_Table[4 * 2 + 1];
			M68K_Read_Word_Table[0] = _32X_M68K_Read_Word_Table[4 * 2 + 1];
		}
	}
	else
	{
		// 32X OFF
		M68K_Fetch[32].lowaddr = 0x000000;
		M68K_Fetch[32].highaddr = Rom_Size - 1;
		M68K_Fetch[32].offset = (unsigned int)&Rom_Data.u8[0] - 0x000000;
		
		M68K_Fetch[33].lowaddr = -1;
		M68K_Fetch[33].highaddr = -1;
		M68K_Fetch[33].offset = (unsigned int)NULL;
		
		M68K_Read_Byte_Table[0] = _32X_M68K_Read_Byte_Table[0];
		M68K_Read_Word_Table[0] = _32X_M68K_Read_Word_Table[0];
	}
}


/**
 * M68K_32X_Mode(): Modify 32X ROM bank.
 * This function is only called during 32X emulation.
 */
void M68K_Set_32X_Rom_Bank(void)
{
	if (!(_32X_ADEN && !_32X_RV))
		return;
	
	// 32X ROM bank uses M68K_Fetch[34].
	M68K_Fetch[34].offset = (unsigned int)&Rom_Data.u8[Bank_SH2 << 20] - 0x900000;
	
	M68K_Read_Byte_Table[(9 * 2) + 0] =
		_32X_M68K_Read_Byte_Table[(Bank_SH2 << 1) + 0];
	M68K_Read_Byte_Table[(9 * 2) + 1] =
		_32X_M68K_Read_Byte_Table[(Bank_SH2 << 1) + 1];
	M68K_Read_Word_Table[(9 * 2) + 0] =
		_32X_M68K_Read_Word_Table[(Bank_SH2 << 1) + 0];
	M68K_Read_Word_Table[(9 * 2) + 1] =
		_32X_M68K_Read_Word_Table[(Bank_SH2 << 1) + 1];
}


/**
 * M68K_Set_Prg_Ram(): Modify bank Prg_Ram fetch.
 * This function is only called during SegaCD emulation.
 */
void M68K_Set_Prg_Ram(void)
{
	// Program RAM uses M68K_Fetch[34].
	M68K_Fetch[34].offset = (unsigned int)&Ram_Prg.u8[Bank_M68K] - 0x020000;
}


/**
 * MS68K_Set_Word_Ram(): Modify bank Word_Ram fetch.
 * This function is only called during SegaCD emulation.
 */
void MS68K_Set_Word_Ram(void)
{
	// Word RAM uses M68K_Fetch[33].
	switch (Ram_Word_State)
	{
		case 0:
			// Mode 2M -> Assigned to Main CPU
			M68K_Fetch[33].lowaddr = 0x200000;
			M68K_Fetch[33].highaddr = 0x23FFFF;
			M68K_Fetch[33].offset = (unsigned int)&Ram_Word_2M.u8[0] - 0x200000;
			
			//S68K_Fetch[1].lowaddr = -1;
			//S68K_Fetch[1].highaddr = -1;
			//S68K_Fetch[1].offset = (unsigned) NULL;
			
			S68K_Fetch[1].lowaddr = 0x080000;	// why not after all...
			S68K_Fetch[1].highaddr = 0x0BFFFF;
			S68K_Fetch[1].offset = (unsigned int)&Ram_Word_2M.u8[0] - 0x080000;
			break;
		
		case 1:
			// Mode 2M -> Assigned to Sub CPU
			//M68K_Fetch[33].lowaddr = -1;
			//M68K_Fetch[33].highaddr = -1;
			//M68K_Fetch[33].offset = (unsigned) NULL;
			
			M68K_Fetch[33].lowaddr = 0x200000;	// why not after all...
			M68K_Fetch[33].highaddr = 0x23FFFF;
			M68K_Fetch[33].offset = (unsigned int)&Ram_Word_2M.u8[0] - 0x200000;
			
			S68K_Fetch[1].lowaddr = 0x080000;
			S68K_Fetch[1].highaddr = 0x0BFFFF;
			S68K_Fetch[1].offset = (unsigned int)&Ram_Word_2M.u8[0] - 0x080000;
			break;
		
		case 2:
			// Mode 1M -> Bank 0 to Main CPU
			M68K_Fetch[33].lowaddr = 0x200000;	// Bank 0
			M68K_Fetch[33].highaddr = 0x21FFFF;
			M68K_Fetch[33].offset = (unsigned int)&Ram_Word_1M.u8[0] - 0x200000;
			
			S68K_Fetch[1].lowaddr = 0x0C0000;	// Bank 1
			S68K_Fetch[1].highaddr = 0x0DFFFF;
			S68K_Fetch[1].offset = (unsigned int)&Ram_Word_1M.u8[0x20000] - 0x0C0000;
			break;
		
		case 3:
			// Mode 1M -> Bank 0 to Sub CPU
			M68K_Fetch[33].lowaddr = 0x200000;	// Bank 1
			M68K_Fetch[33].highaddr = 0x21FFFF;
			M68K_Fetch[33].offset = (unsigned int)&Ram_Word_1M.u8[0x20000] - 0x200000;
			
			S68K_Fetch[1].lowaddr = 0x0C0000;	// Bank 0
			S68K_Fetch[1].highaddr = 0x0DFFFF;
			S68K_Fetch[1].offset = (unsigned int) &Ram_Word_1M.u8[0] - 0x0C0000;
			break;
		
		default:
			// to make gcc shut up
			break;
	}
}


/**
 * M68K_Reset_CPU(): Reset the Main 68000 CPU.
 */
void M68K_Reset_CPU(void)
{
	main68k_reset();
}


/**
 * S68K_Reset_CPU(): Reset the Sub 68000 CPU.
 */
void S68K_Reset_CPU(void)
{
	sub68k_reset();
}
