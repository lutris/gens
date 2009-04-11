/***************************************************************************
 * Gens: Z80 memory handler functions.                                     *
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

#ifndef GENS_MEM_Z80_H
#define GENS_MEM_Z80_H

#include "macros/fastcall.h"

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char Ram_Z80[8 * 1024];
extern int Bank_Z80;

unsigned char Z80_ReadB(unsigned int Adr);
unsigned short Z80_ReadW(unsigned int Adr);
void Z80_WriteB(unsigned int Adr, unsigned char Data);
void Z80_WriteW(unsigned int Adr, unsigned short Data);

unsigned char FASTCALL Z80_ReadB_Bad(unsigned int Adr);
unsigned char FASTCALL Z80_ReadB_Ram(unsigned int Adr);
unsigned char FASTCALL Z80_ReadB_Bank(unsigned int Adr);
unsigned char FASTCALL Z80_ReadB_YM2612(unsigned int Adr);
unsigned char FASTCALL Z80_ReadB_PSG(unsigned int Adr);
unsigned char FASTCALL Z80_ReadB_68K_Ram(unsigned int Adr);

unsigned short FASTCALL Z80_ReadW_Bad(unsigned int Adr);
unsigned short FASTCALL Z80_ReadW_Ram(unsigned int Adr);
unsigned short FASTCALL Z80_ReadW_Bank(unsigned int Adr);
unsigned short FASTCALL Z80_ReadW_YM2612(unsigned int Adr);
unsigned short FASTCALL Z80_ReadW_PSG(unsigned int Adr);
unsigned short FASTCALL Z80_ReadW_68K_Ram(unsigned int Adr);

void FASTCALL Z80_WriteB_Bad(unsigned int Adr, unsigned char Data);
void FASTCALL Z80_WriteB_Ram(unsigned int Adr, unsigned char Data);
void FASTCALL Z80_WriteB_Bank(unsigned int Adr, unsigned char Data);
void FASTCALL Z80_WriteB_YM2612(unsigned int Adr, unsigned char Data);
void FASTCALL Z80_WriteB_PSG(unsigned int Adr, unsigned char Data);
void FASTCALL Z80_WriteB_68K_Ram(unsigned int Adr, unsigned char Data);

void FASTCALL Z80_WriteW_Bad(unsigned int Adr, unsigned short Data);
void FASTCALL Z80_WriteW_Bank(unsigned int Adr, unsigned short Data);
void FASTCALL Z80_WriteW_Ram(unsigned int Adr, unsigned short Data);
void FASTCALL Z80_WriteW_YM2612(unsigned int Adr, unsigned short Data);
void FASTCALL Z80_WriteW_PSG(unsigned int Adr, unsigned short Data);
void FASTCALL Z80_WriteW_68K_Ram(unsigned int Adr, unsigned short Data);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MEM_Z80_H */
