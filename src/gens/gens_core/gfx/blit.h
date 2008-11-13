#ifndef GENS_BLIT_H
#define GENS_BLIT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GENS_X86_ASM
void Blit1x_16(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit1x_32(unsigned char *screen, int pitch, int x, int y, int offset);
#else
void Blit1x_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit1x_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit1x_32_asm(unsigned char *screen, int pitch, int x, int y, int offset);
#endif /* GENS_X86_ASM */

#ifndef GENS_X86_ASM
void Blit2x_16(unsigned char *screen, int pitch, int x, int y, int offset);
#else
void Blit2x_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit2x_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
#endif /* GENS_X86_ASM */
void Blit2x_32(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit2x_Int_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit2x_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

#ifndef GENS_X86_ASM
void Blit2x_Scanline_16(unsigned char *screen, int pitch, int x, int y, int offset);
#else
void Blit2x_Scanline_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit2x_Scanline_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
#endif
void Blit2x_Scanline_32(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit2x_Scanline_Int_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit2x_Scanline_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit2x_Scanline_50_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit2x_Scanline_50_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit2x_Scanline_25_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit2x_Scanline_25_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2xSAI_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_Scale2x_16(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_Scale2x_32(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_HQ2x_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);

int Blit_HQ2x_InitLUTs(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_BLIT_H */
