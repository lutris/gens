#ifndef GENS_IO_H
#define GENS_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "io_old.h"

#define CONTROLLER_UP		0x00000001
#define CONTROLLER_DOWN		0x00000002
#define CONTROLLER_LEFT		0x00000004
#define CONTROLLER_RIGHT	0x00000008
#define CONTROLLER_START	0x00000010
#define CONTROLLER_A		0x00000020
#define CONTROLLER_B		0x00000040
#define CONTROLLER_C		0x00000080
#define CONTROLLER_MODE		0x00000100
#define CONTROLLER_X		0x00000200
#define CONTROLLER_Y		0x00000400
#define CONTROLLER_Z		0x00000800

extern unsigned int Controller_1_State;
extern unsigned int Controller_1_COM;
extern unsigned int Controller_1_Counter;
extern unsigned int Controller_1_Delay;

extern unsigned int Controller_1_Buttons;
extern unsigned int Controller_1B_Buttons;
extern unsigned int Controller_1C_Buttons;
extern unsigned int Controller_1D_Buttons;

extern unsigned int Controller_2_Buttons;
extern unsigned int Controller_2B_Buttons;
extern unsigned int Controller_2C_Buttons;
extern unsigned int Controller_2D_Buttons;

extern unsigned int Controller_2_State;
extern unsigned int Controller_2_COM;
extern unsigned int Controller_2_Counter;
extern unsigned int Controller_2_Delay;

extern unsigned int Controller_1_Type;
extern unsigned int Controller_1B_Type;
extern unsigned int Controller_1C_Type;
extern unsigned int Controller_1D_Type;

extern unsigned int Controller_2_Type;
extern unsigned int Controller_2B_Type;
extern unsigned int Controller_2C_Type;
extern unsigned int Controller_2D_Type;

unsigned char RD_Controller_1(void);
unsigned char RD_Controller_2(void);
unsigned char WR_Controller_1(unsigned char data);
unsigned char WR_Controller_2(unsigned char data);
void Fix_Controllers(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_IO_H */
