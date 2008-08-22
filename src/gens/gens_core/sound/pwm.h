#ifndef _PWM_H_
#define _PWM_H_

#ifndef __GNUC__

#define FASTCALL				__fastcall
#define DECL_FASTCALL(type, name)	type FASTCALL name

#else //__GNUC__

#define FASTCALL __attribute__ ((regparm(2)))
#define DECL_FASTCALL(type, name)	type name __attribute__ ((regparm(2)))

#endif //!__GNUC__

extern unsigned short PWM_FIFO_R[8];
extern unsigned short PWM_FIFO_L[8];
extern unsigned int PWM_RP_R;
extern unsigned int PWM_WP_R;
extern unsigned int PWM_RP_L;
extern unsigned int PWM_WP_L;
extern unsigned int PWM_Cycles;
extern unsigned int PWM_Cycle;
extern unsigned int PWM_Cycle_Cnt;
extern unsigned int PWM_Int;
extern unsigned int PWM_Int_Cnt;
extern unsigned int PWM_Mode;
extern unsigned int PWM_Enable;
extern unsigned int PWM_Out_R;
extern unsigned int PWM_Out_L;


void FASTCALL PWM_Init(void);
void FASTCALL PWM_Set_Cycle(unsigned int cycle);
void FASTCALL PWM_Set_Int(unsigned int int_time);
void FASTCALL PWM_Clear_Timer(void);
void FASTCALL PWM_Update_Timer(unsigned int cycle);
void FASTCALL PWM_Update(int **buf, int length);

#endif
