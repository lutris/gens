#ifndef _BLIT_H_
#define _BLIT_H_

void Blit_1x(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_1x_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_1x_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2x(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_2x_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_2x_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2x_Int_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_2x_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2x_Scanline_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_2x_Scanline_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2x_Scanline_Int_16_asm(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_2x_Scanline_Int_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2x_Scanline_50_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_Scanline_50_Int_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2x_Scanline_25_16_asm_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_Scanline_25_Int_MMX(unsigned char *screen, int pitch, int x, int y, int offset);

void Blit_2xSAI_MMX(unsigned char *screen, int pitch, int x, int y, int offset);
void Blit_Scale2x(unsigned char *screen, int pitch, int x, int y, int offset);
void _Blit_HQ2x(unsigned char *screen, int pitch, int x, int y, int offset);
int Blit_HQ2x_InitLUTs(void);

#endif
