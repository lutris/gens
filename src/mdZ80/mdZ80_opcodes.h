/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80_opcodes.h: Z80 opcodes.                                           *
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

/**
 * Z80 opcodes: 1-byte opcodes.
 */
enum mdZ80_opcode
{
	/* 0x00 - 0x0F */
	Z80_OP_NOP		= 0x00,	Z80_OP_LD_BC_NN		= 0x01,
	Z80_OP_LD_mBC_A		= 0x02,	Z80_OP_INC_BC		= 0x03,
	Z80_OP_INC_B		= 0x04,	Z80_OP_DEC_B		= 0x05,
	Z80_OP_LD_B_N		= 0x06,	Z80_OP_RLCA		= 0x07,
	Z80_OP_EX_AF_AF2	= 0x08,	Z80_OP_ADD_HL_BC	= 0x09,
	Z80_OP_LD_A_mBC		= 0x0A,	Z80_OP_DEC_BC		= 0x0B,
	Z80_OP_INC_C		= 0x0C,	Z80_OP_DEC_C		= 0x0D,
	Z80_OP_LD_C_N		= 0x0E,	Z80_OP_RRCA		= 0x0F,
	
	/* 0x10 - 0x1F */
	Z80_OP_DJNZ		= 0x10,	Z80_OP_LD_DE_NN		= 0x11,
	Z80_OP_LD_mDE_A		= 0x12,	Z80_OP_INC_DE		= 0x13,
	Z80_OP_INC_D		= 0x14,	Z80_OP_DEC_D		= 0x15,
	Z80_OP_LD_D_N		= 0x16,	Z80_OP_RLA		= 0x17,
	Z80_OP_JR_N		= 0x18,	Z80_OP_ADD_HL_DE	= 0x19,
	Z80_OP_LD_A_mDE		= 0x1A,	Z80_OP_DEC_DE		= 0x1B,
	Z80_OP_INC_E		= 0x1C,	Z80_OP_DEC_E		= 0x1D,
	Z80_OP_LD_E_N		= 0x1E,	Z80_OP_RRA		= 0x1F,
	
	/* 0x20 - 0x2F */
	Z80_OP_JRNZ_N		= 0x20,	Z80_OP_LD_HL_NN		= 0x21,
	Z80_OP_LD_mNN_HL	= 0x22,	Z80_OP_INC_HL		= 0x23,
	Z80_OP_INC_H		= 0x24,	Z80_OP_DEC_H		= 0x25,
	Z80_OP_LD_H_N		= 0x26,	Z80_OP_DAA		= 0x27,
	Z80_OP_JRZ_N		= 0x28,	Z80_OP_ADD_HL_HL	= 0x29,
	Z80_OP_LD_HL_mNN	= 0x2A,	Z80_OP_DEC_HL		= 0x2B,
	Z80_OP_INC_L		= 0x2C,	Z80_OP_DEC_L		= 0x2D,
	Z80_OP_LD_L_N		= 0x2E,	Z80_OP_CPL		= 0x2F,
	
	/* 0x30 - 0x3F */
	Z80_OP_JRNC_N		= 0x30,	Z80_OP_LD_SP_NN		= 0x31,
	Z80_OP_LD_mNN_A		= 0x32,	Z80_OP_INC_SP		= 0x33,
	Z80_OP_INC_mHL		= 0x34,	Z80_OP_DEC_mHL		= 0x35,
	Z80_OP_LD_mHL_N		= 0x36,	Z80_OP_SCF		= 0x37,
	Z80_OP_JRC_N		= 0x38,	Z80_OP_ADD_HL_SP	= 0x39,
	Z80_OP_LD_A_mNN		= 0x3A,	Z80_OP_DEC_SP		= 0x3B,
	Z80_OP_INC_A		= 0x3C,	Z80_OP_DEC_A		= 0x3D,
	Z80_OP_LD_A_N		= 0x3E,	Z80_OP_CCF		= 0x3F,
	
	/* 0x40 - 0x4F */
	Z80_OP_LD_B_B		= 0x40,	Z80_OP_LD_B_C		= 0x41,
	Z80_OP_LD_B_D		= 0x42,	Z80_OP_LD_B_E		= 0x43,
	Z80_OP_LD_B_H		= 0x44,	Z80_OP_LD_B_L		= 0x45,
	Z80_OP_LD_B_mHL		= 0x46,	Z80_OP_LD_B_A		= 0x47,
	Z80_OP_LD_C_B		= 0x48,	Z80_OP_LD_C_C		= 0x49,
	Z80_OP_LD_C_D		= 0x4A,	Z80_OP_LD_C_E		= 0x4B,
	Z80_OP_LD_C_H		= 0x4C,	Z80_OP_LD_C_L		= 0x4D,
	Z80_OP_LD_C_mHL		= 0x4E,	Z80_OP_LD_C_A		= 0x4F,
	
	/* 0x50 - 0x5F */
	Z80_OP_LD_D_B		= 0x50,	Z80_OP_LD_D_C		= 0x51,
	Z80_OP_LD_D_D		= 0x52,	Z80_OP_LD_D_E		= 0x53,
	Z80_OP_LD_D_H		= 0x54,	Z80_OP_LD_D_L		= 0x55,
	Z80_OP_LD_D_mHL		= 0x56,	Z80_OP_LD_D_A		= 0x57,
	Z80_OP_LD_E_B		= 0x58,	Z80_OP_LD_E_C		= 0x59,
	Z80_OP_LD_E_D		= 0x5A,	Z80_OP_LD_E_E		= 0x5B,
	Z80_OP_LD_E_H		= 0x5C,	Z80_OP_LD_E_L		= 0x5D,
	Z80_OP_LD_E_mHL		= 0x5E,	Z80_OP_LD_E_A		= 0x5F,
	
	/* 0x60 - 0x6F */
	Z80_OP_LD_H_B		= 0x60,	Z80_OP_LD_H_C		= 0x61,
	Z80_OP_LD_H_D		= 0x62,	Z80_OP_LD_H_E		= 0x63,
	Z80_OP_LD_H_H		= 0x64,	Z80_OP_LD_H_L		= 0x65,
	Z80_OP_LD_H_mHL		= 0x66,	Z80_OP_LD_H_A		= 0x67,
	Z80_OP_LD_L_B		= 0x68,	Z80_OP_LD_L_C		= 0x69,
	Z80_OP_LD_L_D		= 0x6A,	Z80_OP_LD_L_E		= 0x6B,
	Z80_OP_LD_L_H		= 0x6C,	Z80_OP_LD_L_L		= 0x6D,
	Z80_OP_LD_L_mHL		= 0x6E,	Z80_OP_LD_L_A		= 0x6F,
	
	/* 0x70 - 0x7F */
	Z80_OP_LD_mHL_B		= 0x70,	Z80_OP_LD_mHL_C		= 0x71,
	Z80_OP_LD_mHL_D		= 0x72,	Z80_OP_LD_mHL_E		= 0x73,
	Z80_OP_LD_mHL_H		= 0x74,	Z80_OP_LD_mHL_L		= 0x75,
	Z80_OP_LD_mHL_mHL	= 0x76,	Z80_OP_LD_mHL_A		= 0x77,
	Z80_OP_LD_A_B		= 0x78,	Z80_OP_LD_A_C		= 0x79,
	Z80_OP_LD_A_D		= 0x7A,	Z80_OP_LD_A_E		= 0x7B,
	Z80_OP_LD_A_H		= 0x7C,	Z80_OP_LD_A_L		= 0x7D,
	Z80_OP_LD_A_mHL		= 0x7E,	Z80_OP_LD_A_A		= 0x7F,
	
	/* 0x80 - 0x8F */
	Z80_OP_ADD_A_B		= 0x80,	Z80_OP_ADD_A_C		= 0x81,
	Z80_OP_ADD_A_D		= 0x82,	Z80_OP_ADD_A_E		= 0x83,
	Z80_OP_ADD_A_H		= 0x84,	Z80_OP_ADD_A_L		= 0x85,
	Z80_OP_ADD_A_mHL	= 0x86,	Z80_OP_ADD_A_A		= 0x87,
	Z80_OP_ADC_A_B		= 0x88,	Z80_OP_ADC_A_C		= 0x89,
	Z80_OP_ADC_A_D		= 0x8A,	Z80_OP_ADC_A_E		= 0x8B,
	Z80_OP_ADC_A_H		= 0x8C,	Z80_OP_ADC_A_L		= 0x8D,
	Z80_OP_ADC_A_mHL	= 0x8E,	Z80_OP_ADC_A_A		= 0x8F,
	
	/* 0x90 - 0x9F */
	Z80_OP_SUB_B		= 0x90,	Z80_OP_SUB_C		= 0x91,
	Z80_OP_SUB_D		= 0x92,	Z80_OP_SUB_E		= 0x93,
	Z80_OP_SUB_H		= 0x94,	Z80_OP_SUB_L		= 0x95,
	Z80_OP_SUB_mHL		= 0x96,	Z80_OP_SUB_A		= 0x97,
	Z80_OP_SBC_B		= 0x98,	Z80_OP_SBC_C		= 0x99,
	Z80_OP_SBC_D		= 0x9A,	Z80_OP_SBC_E		= 0x9B,
	Z80_OP_SBC_H		= 0x9C,	Z80_OP_SBC_L		= 0x9D,
	Z80_OP_SBC_mHL		= 0x9E,	Z80_OP_SBC_A		= 0x9F,
	
	/* 0xA0 - 0xAF */
	Z80_OP_AND_B		= 0xA0,	Z80_OP_AND_C		= 0xA1,
	Z80_OP_AND_D		= 0xA2,	Z80_OP_AND_E		= 0xA3,
	Z80_OP_AND_H		= 0xA4,	Z80_OP_AND_L		= 0xA5,
	Z80_OP_AND_mHL		= 0xA6,	Z80_OP_AND_A		= 0xA7,
	Z80_OP_XOR_B		= 0xA8,	Z80_OP_XOR_C		= 0xA9,
	Z80_OP_XOR_D		= 0xAA,	Z80_OP_XOR_E		= 0xAB,
	Z80_OP_XOR_H		= 0xAC,	Z80_OP_XOR_L		= 0xAD,
	Z80_OP_XOR_mHL		= 0xAE,	Z80_OP_XOR_A		= 0xAF,
	
	/* 0xB0 - 0xBF */
	Z80_OP_OR_B		= 0xB0,	Z80_OP_OR_C		= 0xB1,
	Z80_OP_OR_D		= 0xB2,	Z80_OP_OR_E		= 0xB3,
	Z80_OP_OR_H		= 0xB4,	Z80_OP_OR_L		= 0xB5,
	Z80_OP_OR_mHL		= 0xB6,	Z80_OP_OR_A		= 0xB7,
	Z80_OP_CP_B		= 0xB8,	Z80_OP_CP_C		= 0xB9,
	Z80_OP_CP_D		= 0xBA,	Z80_OP_CP_E		= 0xBB,
	Z80_OP_CP_H		= 0xBC,	Z80_OP_CP_L		= 0xBD,
	Z80_OP_CP_mHL		= 0xBE,	Z80_OP_CP_A		= 0xBF,
	
	/* 0xC0 - 0xCF */
	Z80_OP_RET_NZ		= 0xC0,	Z80_OP_POP_BC		= 0xC1,
	Z80_OP_JP_NZ_NN		= 0xC2,	Z80_OP_JP_NN		= 0xC3,
	Z80_OP_CALL_NZ_NN	= 0xC4,	Z80_OP_PUSH_BC		= 0xC5,
	Z80_OP_ADD_A_N		= 0xC6,	Z80_OP_RST_00H		= 0xC7,
	Z80_OP_RET_Z		= 0xC8,	Z80_OP_RET		= 0xC9,
	Z80_OP_JP_Z_NN		= 0xCA,	Z80_PREFIX_CB		= 0xCB,
	Z80_OP_CALL_Z_NN	= 0xCC,	Z80_OP_CALL_NN		= 0xCD,
	Z80_OP_ADC_A_N		= 0xCE,	Z80_OP_RST_08H		= 0xCF,
	
	/* 0xD0 - 0xDF */
	Z80_OP_RET_NC		= 0xD0,	Z80_OP_POP_DE		= 0xD1,
	Z80_OP_JP_NC_NN		= 0xD2,	Z80_OP_OUT_mN		= 0xD3,
	Z80_OP_CALL_NC_NN	= 0xD4,	Z80_OP_PUSH_DE		= 0xD5,
	Z80_OP_SUB_N		= 0xD6,	Z80_OP_RST_10H		= 0xD7,
	Z80_OP_RET_C		= 0xD8,	Z80_OP_EXX		= 0xD9,
	Z80_OP_JP_C_NN		= 0xDA,	Z80_IN_mN		= 0xDB,
	Z80_OP_CALL_C_NN	= 0xDC,	Z80_PREFIX_DD		= 0xDD,
	Z80_OP_SBC_N		= 0xDE,	Z80_OP_RST_18H		= 0xDF,
	
	/* 0xE0 - 0xEF */
	Z80_OP_RET_NP		= 0xE0,	Z80_OP_POP_HL		= 0xE1,
	Z80_OP_JP_NP_NN		= 0xE2,	Z80_OP_EX_mSP_HL	= 0xE3,
	Z80_OP_CALL_NP_NN	= 0xE4,	Z80_OP_PUSH_HL		= 0xE5,
	Z80_OP_AND_N		= 0xE6,	Z80_OP_RST_20H		= 0xE7,
	Z80_OP_RET_P		= 0xE8,	Z80_OP_JP_HL		= 0xE9,
	Z80_OP_JP_P_NN		= 0xEA,	Z80_EX_DE_HL		= 0xEB,
	Z80_OP_CALL_P_NN	= 0xEC,	Z80_PREFIX_ED		= 0xED,
	Z80_OP_XOR_N		= 0xEE,	Z80_OP_RST_28H		= 0xEF,
	
	/* 0xF0 - 0xFF */
	Z80_OP_RET_NS		= 0xF0,	Z80_OP_POP_AF		= 0xF1,
	Z80_OP_JP_NS_NN		= 0xF2,	Z80_OP_DI		= 0xF3,
	Z80_OP_CALL_NS_NN	= 0xF4,	Z80_OP_PUSH_AF		= 0xF5,
	Z80_OP_OR_N		= 0xF6,	Z80_OP_RST_30H		= 0xF7,
	Z80_OP_RET_S		= 0xF8,	Z80_OP_LD_SP_HL		= 0xF9,
	Z80_OP_JP_S_NN		= 0xFA,	Z80_EX_EI		= 0xFB,
	Z80_OP_CALL_S_NN	= 0xFC,	Z80_PREFIX_FD		= 0xFD,
	Z80_OP_CP_N		= 0xFE,	Z80_OP_RST_38H		= 0xFF,
};
