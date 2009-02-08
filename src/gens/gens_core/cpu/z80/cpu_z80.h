#ifndef GENS_CPU_Z80_H
#define GENS_CPU_Z80_H

#ifdef __cplusplus
extern "C" {
#endif

int Z80_Init(void);
void Z80_Reset(void);

/** Stub functions for SegaCD debugging. **/
void Write_To_Bank(int val);
void Read_To_68K_Space(int adr);
void Write_To_68K_Space(int adr, int data);

#ifdef __cplusplus
}
#endif

#endif /* GENS_CPU_Z80_H */
