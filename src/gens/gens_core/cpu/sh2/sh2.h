/**********************************************************/
/*                                                        */
/* SH2 emulator 1.40 (Hearder part)                       */
/* Copyright 2002 Stéphane Dallongeville                  */
/* Used for the 32X emulation in Gens                     */
/*                                                        */
/**********************************************************/

/* 20021106 sthief: slightly changed for GCC (FASTCALL, DECL_FASTCALL) */

#ifndef GENS_SH2_H
#define GENS_SH2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* The SH2 emulator uses FASTCALL. */
#include "libgsft/gsft_fastcall.h"


/****************************/
/* Structures & definitions */
/****************************/

struct __status_reg
{
	uint8_t T;
	uint8_t S;
	uint8_t IMask;
	uint8_t MQ;
};

typedef struct __status_reg STATREG;

struct __interrupt_struct
{
	uint8_t Vect;
	uint8_t Prio;
	uint8_t res1;
	uint8_t res2;
};

typedef struct __interrupt_struct INTSTR;


struct __fetch_region
{
	uint32_t Low_Adr;
	uint32_t High_Adr;
	uint16_t *Fetch_Reg;
};

typedef struct __fetch_region FETCHREG;


typedef uint8_t FASTCALL SH2_RB(uint32_t adr);
typedef uint16_t FASTCALL SH2_RW(uint32_t adr);
typedef uint32_t FASTCALL SH2_RL(uint32_t adr);
typedef void FASTCALL SH2_WB(uint32_t adr, uint8_t data);
typedef void FASTCALL SH2_WW(uint32_t adr, uint16_t data);
typedef void FASTCALL SH2_WL(uint32_t adr, uint32_t data);


struct __SH2_context
{
	uint8_t      Cache[0x1000];

	uint32_t     R[0x10];

	STATREG    SR;
	INTSTR     INT;
	uint32_t     GBR;
	uint32_t     VBR;

	uint8_t      INT_QUEUE[0x20];

	uint32_t     MACH;
	uint32_t     MACL;
	uint32_t     PR;
	uint32_t     PC;

	uint32_t     Status;
	uint32_t     Base_PC;
	uint32_t     Fetch_Start;
	uint32_t     Fetch_End;

	uint32_t     DS_Inst;
	uint32_t     DS_PC;
	uint32_t     Unused1;
	uint32_t     Unused2;

	uint32_t     Odometer;
	uint32_t     Cycle_TD;
	uint32_t     Cycle_IO;
	uint32_t     Cycle_Sup;
	
	SH2_RB     *Read_Byte[0x100];
	SH2_RW     *Read_Word[0x100];
	SH2_RL     *Read_Long[0x100];

	SH2_WB     *Write_Byte[0x100];
	SH2_WW     *Write_Word[0x100];
	SH2_WL     *Write_Long[0x100];

	FETCHREG   Fetch_Region[0x100];

	uint8_t      IO_Reg[0x200];

	uint32_t     DVCR;
	uint32_t     DVSR;
	uint32_t     DVDNTH;
	uint32_t     DVDNTL;

	uint8_t      DRCR0;
	uint8_t      DRCR1;
	uint8_t      DREQ0;
	uint8_t      DREQ1;

	uint32_t     DMAOR;

	uint32_t     SAR0;
	uint32_t     DAR0;
	uint32_t     TCR0;
	uint32_t     CHCR0;

	uint32_t     SAR1;
	uint32_t     DAR1;
	uint32_t     TCR1;
	uint32_t     CHCR1;

	uint32_t     VCRDIV;
	uint32_t     VCRDMA0;
	uint32_t     VCRDMA1;
	uint32_t     VCRWDT;

	uint32_t     IPDIV;
	uint32_t     IPDMA;
	uint32_t     IPWDT;
	uint32_t     IPBSC;

	uint32_t     BARA;
	uint32_t     BAMRA;

	uint8_t      WDT_Tab[8];
	uint32_t     WDTCNT;
	uint8_t      WDT_Sft;
	uint8_t      WDTSR;
	uint8_t      WDTRST;
	uint8_t      Unused3;

	uint8_t      FRT_Tab[4];
	uint32_t     FRTCNT;
	uint32_t     FRTOCRA;
	uint32_t     FRTOCRB;

	uint8_t      FRTTIER;
	uint8_t      FRTCSR;
	uint8_t      FRTTCR;
	uint8_t      FRTTOCR;
	uint32_t     FRTICR;
	uint32_t     FRT_Sft;
	uint32_t     BCR1;
};

typedef struct __SH2_context SH2_CONTEXT;


/*************************/
/* Publics SH2 variables */
/*************************/

// 2 SH2 contexts offered :)

extern SH2_CONTEXT M_SH2;
extern SH2_CONTEXT S_SH2;


/*************************/
/* Publics SH2 functions */
/*************************/

// C functions

void SH2_Init(SH2_CONTEXT *SH2, uint32_t slave);

// VERY IMPORTANT : Fetch have to be a DWORD aligned pointer !

void SH2_Set_Fetch_Reg(SH2_CONTEXT *SH2, uint32_t reg, uint32_t low_adr, uint32_t high_adr, uint16_t *Fetch);
void SH2_Add_Fetch(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, uint16_t *Fetch);

void SH2_Add_ReadB(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, SH2_RB *Func);
void SH2_Add_ReadW(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, SH2_RW *Func);
void SH2_Add_ReadL(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, SH2_RL *Func);
void SH2_Add_WriteB(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, SH2_WB *Func);
void SH2_Add_WriteW(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, SH2_WW *Func);
void SH2_Add_WriteL(SH2_CONTEXT *SH2, uint32_t low_adr, uint32_t high_adr, SH2_WL *Func);

void SH2_Map_Cache_Trough(SH2_CONTEXT *SH2);

uint8_t  FASTCALL Def_READB(uint32_t);
uint16_t FASTCALL Def_READW(uint32_t);
uint32_t FASTCALL Def_READL(uint32_t);

void FASTCALL Def_WRITEB(uint32_t, uint8_t);
void FASTCALL Def_WRITEW(uint32_t, uint16_t);
void FASTCALL Def_WRITEL(uint32_t, uint32_t);

// ASM emulation functions

uint32_t FASTCALL SH2_Reset(SH2_CONTEXT*, uint32_t);		/* SH2, manual */
void FASTCALL SH2_Enable(SH2_CONTEXT*);				/* SH2 */
void FASTCALL SH2_Disable(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Exec(SH2_CONTEXT*, uint32_t);		/* SH2, odo */
void FASTCALL SH2_Interrupt(SH2_CONTEXT*, uint32_t);		/* SH2, level */
void FASTCALL SH2_Interrupt_Internal(SH2_CONTEXT*, uint32_t);	/* SH2, level_vector */
void FASTCALL SH2_NMI(SH2_CONTEXT*);				/* SH2 */

uint32_t FASTCALL SH2_Read_Odo(SH2_CONTEXT*);			/* SH2 */
void FASTCALL SH2_Write_Odo(SH2_CONTEXT*, uint32_t);		/* SH2 */
void FASTCALL SH2_Clear_Odo(SH2_CONTEXT*);			/* SH2 */
void FASTCALL SH2_Add_Cycles(SH2_CONTEXT*, uint32_t);		/* SH2, cycles */

void FASTCALL SH2_DMA0_Request(SH2_CONTEXT*, uint8_t);		/* SH2, state */
void FASTCALL SH2_DMA1_Request(SH2_CONTEXT*, uint8_t);		/* SH2, state */
void FASTCALL SH2_FRT_Signal(SH2_CONTEXT*);			/* SH2 */

uint32_t FASTCALL SH2_Get_R(SH2_CONTEXT*, uint32_t);		/* SH2, num */
uint32_t FASTCALL SH2_Get_PC(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Get_SR(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Get_GBR(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Get_VBR(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Get_PR(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Get_MACH(SH2_CONTEXT*);			/* SH2 */
uint32_t FASTCALL SH2_Get_MACL(SH2_CONTEXT*);			/* SH2 */

void FASTCALL SH2_Set_PC(SH2_CONTEXT *, uint32_t);		/* SH2, val */
void FASTCALL SH2_Set_SR(SH2_CONTEXT *, uint32_t);		/* SH2, val */
void FASTCALL SH2_Set_GBR(SH2_CONTEXT *, uint32_t);		/* SH2, val */
void FASTCALL SH2_Set_VBR(SH2_CONTEXT *, uint32_t);		/* SH2, val */
void FASTCALL SH2_Set_PR(SH2_CONTEXT *, uint32_t);		/* SH2, val */
void FASTCALL SH2_Set_MACH(SH2_CONTEXT *, uint32_t);		/* SH2, val */
void FASTCALL SH2_Set_MACL(SH2_CONTEXT *, uint32_t);		/* SH2, val */

uint8_t  SH2_Read_Byte(SH2_CONTEXT *SH2, uint32_t adr);
uint16_t SH2_Read_Word(SH2_CONTEXT *SH2, uint32_t adr);
uint32_t SH2_Read_Long(SH2_CONTEXT *SH2, uint32_t adr);

void SH2_Write_Byte(SH2_CONTEXT *SH2, uint32_t adr, uint8_t data);
void SH2_Write_Word(SH2_CONTEXT *SH2, uint32_t adr, uint16_t data);
void SH2_Write_Long(SH2_CONTEXT *SH2, uint32_t adr, uint32_t data);


/****************************/
/* No-Publics SH2 variables */
/****************************/

extern void *OP_Table[0x10000];
extern void *OPDS_Table[0x10000];
extern uint32_t Set_SR_Table[0x400];


/****************************/
/* No-Publics SH2 functions */
/****************************/

// ASM Memories handlers

uint8_t  FASTCALL SH2_Read_Byte_C0(uint32_t);
uint16_t FASTCALL SH2_Read_Word_C0(uint32_t);
uint32_t FASTCALL SH2_Read_Long_C0(uint32_t);
uint8_t  FASTCALL SH2_Read_Byte_FF(uint32_t);
uint16_t FASTCALL SH2_Read_Word_FF(uint32_t);
uint32_t FASTCALL SH2_Read_Long_FF(uint32_t);

void FASTCALL SH2_Write_Byte_C0(uint32_t, uint8_t);
void FASTCALL SH2_Write_Word_C0(uint32_t, uint16_t);
void FASTCALL SH2_Write_Long_C0(uint32_t, uint32_t);
void FASTCALL SH2_Write_Byte_FF(uint32_t, uint8_t);
void FASTCALL SH2_Write_Word_FF(uint32_t, uint16_t);
void FASTCALL SH2_Write_Long_FF(uint32_t, uint32_t);


// Instruction function definition

void SH2I_ADD(void);
void SH2I_ADDI(void);
void SH2I_ADDC(void);
void SH2I_ADDV(void);
void SH2I_AND(void);
void SH2I_ANDI(void);
void SH2I_ANDM(void);
void SH2I_BF(void);
void SH2I_BFfast(void);
void SH2I_BFS(void);
void SH2I_BFSfast(void);
void SH2I_BRA(void);
void SH2I_BRAfast1(void);
void SH2I_BRAfast2(void);
void SH2I_BRAF(void);
void SH2I_BSR(void);
void SH2I_BSRF(void);
void SH2I_BT(void);
void SH2I_BTS(void);
void SH2I_CLRMAC(void);
void SH2I_CLRT(void);
void SH2I_CMPEQ(void);
void SH2I_CMPGE(void);
void SH2I_CMPGT(void);
void SH2I_CMPHI(void);
void SH2I_CMPHS(void);
void SH2I_CMPPL(void);
void SH2I_CMPPZ(void);
void SH2I_CMPSTR(void);
void SH2I_CMPIM(void);
void SH2I_DIV0S(void);
void SH2I_DIV0U(void);
void SH2I_DIV1(void);
void SH2I_DMULS(void);
void SH2I_DMULU(void);
void SH2I_DT(void);
void SH2I_EXTSB(void);
void SH2I_EXTSW(void);
void SH2I_EXTUB(void);
void SH2I_EXTUW(void);
void SH2I_ILLEGAL(void);
void SH2I_JMP(void);
void SH2I_JSR(void);
void SH2I_LDCSR(void);
void SH2I_LDCGBR(void);
void SH2I_LDCVBR(void);
void SH2I_LDCMSR(void);
void SH2I_LDCMGBR(void);
void SH2I_LDCMVBR(void);
void SH2I_LDSMACH(void);
void SH2I_LDSMACL(void);
void SH2I_LDSPR(void);
void SH2I_LDSMMACH(void);
void SH2I_LDSMMACL(void);
void SH2I_LDSMPR(void);
void SH2I_MACL(void);
void SH2I_MACW(void);
void SH2I_MOV(void);
void SH2I_MOVBS(void);
void SH2I_MOVWS(void);
void SH2I_MOVLS(void);
void SH2I_MOVBL(void);
void SH2I_MOVWL(void);
void SH2I_MOVLL(void);
void SH2I_MOVBM(void);
void SH2I_MOVWM(void);
void SH2I_MOVLM(void);
void SH2I_MOVBP(void);
void SH2I_MOVWP(void);
void SH2I_MOVLP(void);
void SH2I_MOVBS0(void);
void SH2I_MOVWS0(void);
void SH2I_MOVLS0(void);
void SH2I_MOVBL0(void);
void SH2I_MOVWL0(void);
void SH2I_MOVLL0(void);
void SH2I_MOVI(void);
void SH2I_MOVWI(void);
void SH2I_MOVLI(void);
void SH2I_MOVBLG(void);
void SH2I_MOVWLG(void);
void SH2I_MOVLLG(void);
void SH2I_MOVBSG(void);
void SH2I_MOVWSG(void);
void SH2I_MOVLSG(void);
void SH2I_MOVBS4(void);
void SH2I_MOVWS4(void);
void SH2I_MOVLS4(void);
void SH2I_MOVBL4(void);
void SH2I_MOVWL4(void);
void SH2I_MOVLL4(void);
void SH2I_MOVA(void);
void SH2I_MOVT(void);
void SH2I_MULL(void);
void SH2I_MULS(void);
void SH2I_MULU(void);
void SH2I_NEG(void);
void SH2I_NEGC(void);
void SH2I_NOP(void);
void SH2I_NOT(void);
void SH2I_OR(void);
void SH2I_ORI(void);
void SH2I_ORM(void);
void SH2I_ROTCL(void);
void SH2I_ROTCR(void);
void SH2I_ROTL(void);
void SH2I_ROTR(void);
void SH2I_RTE(void);
void SH2I_RTS(void);
void SH2I_SETT(void);
void SH2I_SHAL(void);
void SH2I_SHAR(void);
void SH2I_SHLL(void);
void SH2I_SHLL2(void);
void SH2I_SHLL8(void);
void SH2I_SHLL16(void);
void SH2I_SHLR(void);
void SH2I_SHLR2(void);
void SH2I_SHLR8(void);
void SH2I_SHLR16(void);
void SH2I_SLEEP(void);
void SH2I_STCSR(void);
void SH2I_STCGBR(void);
void SH2I_STCVBR(void);
void SH2I_STCMSR(void);
void SH2I_STCMGBR(void);
void SH2I_STCMVBR(void);
void SH2I_STSMACH(void);
void SH2I_STSMACL(void);
void SH2I_STSPR(void);
void SH2I_STSMMACH(void);
void SH2I_STSMMACL(void);
void SH2I_STSMPR(void);
void SH2I_SUB(void);
void SH2I_SUBC(void);
void SH2I_SUBV(void);
void SH2I_SWAPB(void);
void SH2I_SWAPW(void);
void SH2I_TAS(void);
void SH2I_TRAPA(void);
void SH2I_TST(void);
void SH2I_TSTI(void);
void SH2I_TSTM(void);
void SH2I_XOR(void);
void SH2I_XORI(void);
void SH2I_XORM(void);
void SH2I_XTRCT(void);


// Delay-slot instruction function definition

void SH2I_ADD_DS(void);
void SH2I_ADDI_DS(void);
void SH2I_ADDC_DS(void);
void SH2I_ADDV_DS(void);
void SH2I_AND_DS(void);
void SH2I_ANDI_DS(void);
void SH2I_ANDM_DS(void);
void SH2I_CLRMAC_DS(void);
void SH2I_CLRT_DS(void);
void SH2I_CMPEQ_DS(void);
void SH2I_CMPGE_DS(void);
void SH2I_CMPGT_DS(void);
void SH2I_CMPHI_DS(void);
void SH2I_CMPHS_DS(void);
void SH2I_CMPPL_DS(void);
void SH2I_CMPPZ_DS(void);
void SH2I_CMPSTR_DS(void);
void SH2I_CMPIM_DS(void);
void SH2I_DIV0S_DS(void);
void SH2I_DIV0U_DS(void);
void SH2I_DIV1_DS(void);
void SH2I_DMULS_DS(void);
void SH2I_DMULU_DS(void);
void SH2I_DT_DS(void);
void SH2I_EXTSB_DS(void);
void SH2I_EXTSW_DS(void);
void SH2I_EXTUB_DS(void);
void SH2I_EXTUW_DS(void);
void SH2I_ILLEGAL_DS(void);
void SH2I_LDCSR_DS(void);
void SH2I_LDCGBR_DS(void);
void SH2I_LDCVBR_DS(void);
void SH2I_LDCMSR_DS(void);
void SH2I_LDCMGBR_DS(void);
void SH2I_LDCMVBR_DS(void);
void SH2I_LDSMACH_DS(void);
void SH2I_LDSMACL_DS(void);
void SH2I_LDSPR_DS(void);
void SH2I_LDSMMACH_DS(void);
void SH2I_LDSMMACL_DS(void);
void SH2I_LDSMPR_DS(void);
void SH2I_MACL_DS(void);
void SH2I_MACW_DS(void);
void SH2I_MOV_DS(void);
void SH2I_MOVBS_DS(void);
void SH2I_MOVWS_DS(void);
void SH2I_MOVLS_DS(void);
void SH2I_MOVBL_DS(void);
void SH2I_MOVWL_DS(void);
void SH2I_MOVLL_DS(void);
void SH2I_MOVBM_DS(void);
void SH2I_MOVWM_DS(void);
void SH2I_MOVLM_DS(void);
void SH2I_MOVBP_DS(void);
void SH2I_MOVWP_DS(void);
void SH2I_MOVLP_DS(void);
void SH2I_MOVBS0_DS(void);
void SH2I_MOVWS0_DS(void);
void SH2I_MOVLS0_DS(void);
void SH2I_MOVBL0_DS(void);
void SH2I_MOVWL0_DS(void);
void SH2I_MOVLL0_DS(void);
void SH2I_MOVI_DS(void);
void SH2I_MOVWI_DS(void);
void SH2I_MOVLI_DS(void);
void SH2I_MOVBLG_DS(void);
void SH2I_MOVWLG_DS(void);
void SH2I_MOVLLG_DS(void);
void SH2I_MOVBSG_DS(void);
void SH2I_MOVWSG_DS(void);
void SH2I_MOVLSG_DS(void);
void SH2I_MOVBS4_DS(void);
void SH2I_MOVWS4_DS(void);
void SH2I_MOVLS4_DS(void);
void SH2I_MOVBL4_DS(void);
void SH2I_MOVWL4_DS(void);
void SH2I_MOVLL4_DS(void);
void SH2I_MOVA_DS(void);
void SH2I_MOVT_DS(void);
void SH2I_MULL_DS(void);
void SH2I_MULS_DS(void);
void SH2I_MULU_DS(void);
void SH2I_NEG_DS(void);
void SH2I_NEGC_DS(void);
void SH2I_NOP_DS(void);
void SH2I_NOT_DS(void);
void SH2I_OR_DS(void);
void SH2I_ORI_DS(void);
void SH2I_ORM_DS(void);
void SH2I_ROTCL_DS(void);
void SH2I_ROTCR_DS(void);
void SH2I_ROTL_DS(void);
void SH2I_ROTR_DS(void);
void SH2I_SETT_DS(void);
void SH2I_SHAL_DS(void);
void SH2I_SHAR_DS(void);
void SH2I_SHLL_DS(void);
void SH2I_SHLL2_DS(void);
void SH2I_SHLL8_DS(void);
void SH2I_SHLL16_DS(void);
void SH2I_SHLR_DS(void);
void SH2I_SHLR2_DS(void);
void SH2I_SHLR8_DS(void);
void SH2I_SHLR16_DS(void);
void SH2I_SLEEP_DS(void);
void SH2I_STCSR_DS(void);
void SH2I_STCGBR_DS(void);
void SH2I_STCVBR_DS(void);
void SH2I_STCMSR_DS(void);
void SH2I_STCMGBR_DS(void);
void SH2I_STCMVBR_DS(void);
void SH2I_STSMACH_DS(void);
void SH2I_STSMACL_DS(void);
void SH2I_STSPR_DS(void);
void SH2I_STSMMACH_DS(void);
void SH2I_STSMMACL_DS(void);
void SH2I_STSMPR_DS(void);
void SH2I_SUB_DS(void);
void SH2I_SUBC_DS(void);
void SH2I_SUBV_DS(void);
void SH2I_SWAPB_DS(void);
void SH2I_SWAPW_DS(void);
void SH2I_TAS_DS(void);
void SH2I_TST_DS(void);
void SH2I_TSTI_DS(void);
void SH2I_TSTM_DS(void);
void SH2I_XOR_DS(void);
void SH2I_XORI_DS(void);
void SH2I_XORM_DS(void);
void SH2I_XTRCT_DS(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_SH2_H */
