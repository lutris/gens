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

#include <stdint.h>

#include "libgsft/gsft_fastcall.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t Ram_Z80[8 * 1024];
extern int Bank_Z80;

uint8_t Z80_ReadB(uint32_t address);
uint16_t Z80_ReadW(uint32_t address);
void Z80_WriteB(uint32_t address, uint8_t data);
void Z80_WriteW(uint32_t address, uint16_t data);

uint8_t FASTCALL Z80_ReadB_Bad(uint32_t address);
uint8_t FASTCALL Z80_ReadB_Ram(uint32_t address);
uint8_t FASTCALL Z80_ReadB_Bank(uint32_t address);
uint8_t FASTCALL Z80_ReadB_YM2612(uint32_t address);
uint8_t FASTCALL Z80_ReadB_PSG(uint32_t address);
uint8_t FASTCALL Z80_ReadB_68K_Ram(uint32_t address);

uint16_t FASTCALL Z80_ReadW_Bad(uint32_t address);
uint16_t FASTCALL Z80_ReadW_Ram(uint32_t address);
uint16_t FASTCALL Z80_ReadW_Bank(uint32_t address);
uint16_t FASTCALL Z80_ReadW_YM2612(uint32_t address);
uint16_t FASTCALL Z80_ReadW_PSG(uint32_t address);
uint16_t FASTCALL Z80_ReadW_68K_Ram(uint32_t address);

void FASTCALL Z80_WriteB_Bad(uint32_t address, uint8_t data);
void FASTCALL Z80_WriteB_Ram(uint32_t address, uint8_t data);
void FASTCALL Z80_WriteB_Bank(uint32_t address, uint8_t data);
void FASTCALL Z80_WriteB_YM2612(uint32_t address, uint8_t data);
void FASTCALL Z80_WriteB_PSG(uint32_t address, uint8_t data);
void FASTCALL Z80_WriteB_68K_Ram(uint32_t address, uint8_t data);

void FASTCALL Z80_WriteW_Bad(uint32_t address, uint16_t data);
void FASTCALL Z80_WriteW_Bank(uint32_t address, uint16_t data);
void FASTCALL Z80_WriteW_Ram(uint32_t address, uint16_t data);
void FASTCALL Z80_WriteW_YM2612(uint32_t address, uint16_t data);
void FASTCALL Z80_WriteW_PSG(uint32_t address, uint16_t data);
void FASTCALL Z80_WriteW_68K_Ram(uint32_t address, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MEM_Z80_H */
