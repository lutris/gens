 /**
  * Gens: Controller I/O
  */

#include <stdio.h>

#include "io.h"

unsigned int Controller_1_Type;
unsigned int Controller_1_State;
unsigned int Controller_1_COM;
unsigned int Controller_1_Counter;
unsigned int Controller_1_Delay;
unsigned int Controller_1_Up;
unsigned int Controller_1_Down;
unsigned int Controller_1_Left;
unsigned int Controller_1_Right;
unsigned int Controller_1_Start;
unsigned int Controller_1_Mode;
unsigned int Controller_1_A;
unsigned int Controller_1_B;
unsigned int Controller_1_C;
unsigned int Controller_1_X;
unsigned int Controller_1_Y;
unsigned int Controller_1_Z;

unsigned int Controller_1B_Type;
unsigned int Controller_1B_State;
unsigned int Controller_1B_COM;
unsigned int Controller_1B_Counter;
unsigned int Controller_1B_Delay;
unsigned int Controller_1B_Up;
unsigned int Controller_1B_Down;
unsigned int Controller_1B_Left;
unsigned int Controller_1B_Right;
unsigned int Controller_1B_Start;
unsigned int Controller_1B_Mode;
unsigned int Controller_1B_A;
unsigned int Controller_1B_B;
unsigned int Controller_1B_C;
unsigned int Controller_1B_X;
unsigned int Controller_1B_Y;
unsigned int Controller_1B_Z;

unsigned int Controller_1C_Type;
unsigned int Controller_1C_State;
unsigned int Controller_1C_COM;
unsigned int Controller_1C_Counter;
unsigned int Controller_1C_Delay;
unsigned int Controller_1C_Up;
unsigned int Controller_1C_Down;
unsigned int Controller_1C_Left;
unsigned int Controller_1C_Right;
unsigned int Controller_1C_Start;
unsigned int Controller_1C_Mode;
unsigned int Controller_1C_A;
unsigned int Controller_1C_B;
unsigned int Controller_1C_C;
unsigned int Controller_1C_X;
unsigned int Controller_1C_Y;
unsigned int Controller_1C_Z;

unsigned int Controller_1D_Type;
unsigned int Controller_1D_State;
unsigned int Controller_1D_COM;
unsigned int Controller_1D_Counter;
unsigned int Controller_1D_Delay;
unsigned int Controller_1D_Up;
unsigned int Controller_1D_Down;
unsigned int Controller_1D_Left;
unsigned int Controller_1D_Right;
unsigned int Controller_1D_Start;
unsigned int Controller_1D_Mode;
unsigned int Controller_1D_A;
unsigned int Controller_1D_B;
unsigned int Controller_1D_C;
unsigned int Controller_1D_X;
unsigned int Controller_1D_Y;
unsigned int Controller_1D_Z;

unsigned int Controller_2_Type;
unsigned int Controller_2_State;
unsigned int Controller_2_COM;
unsigned int Controller_2_Counter;
unsigned int Controller_2_Delay;
unsigned int Controller_2_Up;
unsigned int Controller_2_Down;
unsigned int Controller_2_Left;
unsigned int Controller_2_Right;
unsigned int Controller_2_Start;
unsigned int Controller_2_Mode;
unsigned int Controller_2_A;
unsigned int Controller_2_B;
unsigned int Controller_2_C;
unsigned int Controller_2_X;
unsigned int Controller_2_Y;
unsigned int Controller_2_Z;

unsigned int Controller_2B_Type;
unsigned int Controller_2B_State;
unsigned int Controller_2B_COM;
unsigned int Controller_2B_Counter;
unsigned int Controller_2B_Delay;
unsigned int Controller_2B_Up;
unsigned int Controller_2B_Down;
unsigned int Controller_2B_Left;
unsigned int Controller_2B_Right;
unsigned int Controller_2B_Start;
unsigned int Controller_2B_Mode;
unsigned int Controller_2B_A;
unsigned int Controller_2B_B;
unsigned int Controller_2B_C;
unsigned int Controller_2B_X;
unsigned int Controller_2B_Y;
unsigned int Controller_2B_Z;

unsigned int Controller_2C_Type;
unsigned int Controller_2C_State;
unsigned int Controller_2C_COM;
unsigned int Controller_2C_Counter;
unsigned int Controller_2C_Delay;
unsigned int Controller_2C_Up;
unsigned int Controller_2C_Down;
unsigned int Controller_2C_Left;
unsigned int Controller_2C_Right;
unsigned int Controller_2C_Start;
unsigned int Controller_2C_Mode;
unsigned int Controller_2C_A;
unsigned int Controller_2C_B;
unsigned int Controller_2C_C;
unsigned int Controller_2C_X;
unsigned int Controller_2C_Y;
unsigned int Controller_2C_Z;

unsigned int Controller_2D_Type;
unsigned int Controller_2D_State;
unsigned int Controller_2D_COM;
unsigned int Controller_2D_Counter;
unsigned int Controller_2D_Delay;
unsigned int Controller_2D_Up;
unsigned int Controller_2D_Down;
unsigned int Controller_2D_Left;
unsigned int Controller_2D_Right;
unsigned int Controller_2D_Start;
unsigned int Controller_2D_Mode;
unsigned int Controller_2D_A;
unsigned int Controller_2D_B;
unsigned int Controller_2D_C;
unsigned int Controller_2D_X;
unsigned int Controller_2D_Y;
unsigned int Controller_2D_Z;

// Jump Table: Table_Cont (Controller status)
#define First_Low	0x00
#define First_High	0x04
#define Second_Low	0x08
#define Second_High	0x0C
#define Third_Low	0x10
#define Third_High	0x14
#define Fourth_Low	0x18
#define Fourth_High	0x1C


unsigned char RD_Controller_1(void)
{
	// Read controller 1.
	int eax, ebx;
	int jmp;
	
	eax = Controller_1_State;
	ebx = Controller_1_Counter;
	
	if (Controller_1_Type & 0x10)
	{
		// TODO: Teamplayer
		return 0x00;
	}
	
	eax >>= 4;
	ebx = Controller_1_Counter;
	ebx &= 0x03;
	eax &= 0x04;
	
	if (!(Controller_1_Type & 0x01))
	{
		// 3-button controller.
		ebx = 0;
	}
	
	jmp = (ebx * 8) + eax;
	switch (jmp)
	{
		case First_High:
		case Second_High:
		case Third_High:
			eax = Controller_1_C;
			ebx = Controller_1_B;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_1_Right;
			ebx |= Controller_1_Left;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_1_Down;
			ebx |= Controller_1_Up;
			eax = (eax * 2) + ebx + 0x40;
			return (unsigned char)eax;
		
		case First_Low:
		case Second_Low:
			eax = Controller_1_Start;
			ebx = Controller_1_A;
			eax <<= 4;
			ebx <<= 4;
			eax |= Controller_1_Down;
			ebx |= Controller_1_Up;
			eax = (eax * 2) + ebx;
			return (unsigned char)eax;
		
		case Third_Low:
			eax = Controller_1_Start;
			ebx = Controller_1_A;
			eax = (eax * 2) + ebx;
			eax <<= 4;
			return (unsigned char)eax;
		
		case Fourth_High:
			eax = Controller_1_C;
			ebx = Controller_1_B;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_1_Mode;
			ebx |= Controller_1_X;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_1_Y;
			ebx |= Controller_1_Z;
			eax = (eax * 2) + ebx + 0x40;
			return (unsigned char)eax;
		
		case Fourth_Low:
			eax = Controller_1_Start;
			ebx = Controller_1_A;
			eax = (eax * 2) + eax;
			eax <<= 4;
			eax |= 0x0F;
			return (unsigned char)eax;
	}
	
	// Shouldn't happen...
	printf("%s: Invalid jump table entry: %d\n", __func__, jmp);
}


unsigned char RD_Controller_2(void)
{
	// Read controller 2.
	int eax, ebx;
	int jmp;
	
	eax = Controller_2_State;
	eax >>= 4;
	ebx = Controller_2_Counter;
	
	if (Controller_2_Type & 0x10)
	{
		// TODO: Teamplayer
		return;
	}
	
	ebx = Controller_2_Counter;
	ebx &= 0x03;
	eax &= 0x04;
	
	if (!(Controller_2_Type & 0x01))
	{
		// 3-button controller.
		ebx = 0;
	}
	
	jmp = (ebx * 8) + eax;
	switch (jmp)
	{
		case First_High:
		case Second_High:
		case Third_High:
			eax = Controller_2_C;
			ebx = Controller_2_B;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_2_Right;
			ebx |= Controller_2_Left;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_2_Down;
			ebx |= Controller_2_Up;
			eax = (eax * 2) + ebx + 0x40;
			return (unsigned char)eax;
		
		case First_Low:
		case Second_Low:
			eax = Controller_2_Start;
			ebx = Controller_2_A;
			eax <<= 4;
			ebx <<= 4;
			eax |= Controller_2_Down;
			ebx |= Controller_2_Up;
			eax = (eax * 2) + ebx;
			return (unsigned char)eax;
		
		case Third_Low:
			eax = Controller_2_Start;
			ebx = Controller_2_A;
			eax = (eax * 2) + ebx;
			eax <<= 4;
			return (unsigned char)eax;
		
		case Fourth_High:
			eax = Controller_2_C;
			ebx = Controller_2_B;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_2_Mode;
			ebx |= Controller_2_X;
			eax *= 4;
			ebx *= 4;
			eax |= Controller_2_Y;
			ebx |= Controller_2_Z;
			eax = (eax * 2) + ebx + 0x40;
			return (unsigned char)eax;
		
		case Fourth_Low:
			eax = Controller_2_Start;
			ebx = Controller_2_A;
			eax = (eax * 2) + eax;
			eax <<= 4;
			eax |= 0x0F;
			return (unsigned char)eax;
	}
	
	// Shouldn't happen...
	printf("%s: Invalid jump table entry: %d\n", __func__, jmp);
}


unsigned char WR_Controller_1(unsigned char data)
{
	unsigned short al = data;
	if (Controller_1_State & 0x40)
		goto Line1_Down;
	
	if (!(al & 0x40))
		goto Line1_Down;
	
	Controller_1_Counter++;
	
Line1_Down:
	Controller_1_Delay = 0;
	if (Controller_1_Type & 0x10)
	{
		// TODO: Teamplayer support.
	}
	Controller_1_State = al;
	return (unsigned char)al;
}


unsigned char WR_Controller_2(unsigned char data)
{
	unsigned short al = data;
	if (Controller_2_State & 0x40)
		goto Line1_Down;
	
	if (!(al & 0x40))
		goto Line1_Down;
	
	Controller_2_Counter++;
	
Line1_Down:
	Controller_2_Delay = 0;
	if (Controller_2_Type & 0x10)
	{
		// TODO: Teamplayer support.
	}
	Controller_2_State = al;
	return (unsigned char)al;
}


void Fix_Controllers(void)
{
	if (Controller_1_Delay > 25)
		goto Delay_1_Expired;
	Controller_1_Delay++;
	
	if (Controller_2_Delay > 25)
		goto Delay_2_Expired_1;
	Controller_2_Delay++;
	return;
	
Delay_2_Expired_1:
	//Controller_2_Delay = 0;
	Controller_2_Counter = 0;
	return;
	
Delay_1_Expired:
	//Controller_1_Delay = 0;
	Controller_1_Counter = 0;
	if (Controller_2_Delay > 25)
		goto Delay_2_Expired_2;
	Controller_2_Delay++;
	return;
	
Delay_2_Expired_2:
	//Controller_2_Delay = 0;
	Controller_2_Counter = 0;
	return;
}


void Make_IO_Table(void)
{
	// No, we don't need an I/O table for the C version.
	return;
}
