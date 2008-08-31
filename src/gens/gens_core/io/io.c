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

// Controller bitfield format:
// 0000 0000 0000 0000 0000 ZYXM CBAS RLDU

static unsigned char RD_Controller(unsigned int state,
				   unsigned int type,
				   unsigned int counter,
				   unsigned int buttons[4]);

unsigned char RD_Controller_1(void)
{
	// Read controller 1.
	
	// Create the bitfields.
	unsigned int buttons[4] = {0, 0, 0, 0};
	
	// Player A
	buttons[0] |= CONTROLLER_UP	* Controller_1_Up;
	buttons[0] |= CONTROLLER_DOWN	* Controller_1_Down;
	buttons[0] |= CONTROLLER_LEFT	* Controller_1_Left;
	buttons[0] |= CONTROLLER_RIGHT	* Controller_1_Right;
	buttons[0] |= CONTROLLER_START	* Controller_1_Start;
	buttons[0] |= CONTROLLER_A	* Controller_1_A;
	buttons[0] |= CONTROLLER_B	* Controller_1_B;
	buttons[0] |= CONTROLLER_C	* Controller_1_C;
	buttons[0] |= CONTROLLER_MODE	* Controller_1_Mode;
	buttons[0] |= CONTROLLER_X	* Controller_1_X;
	buttons[0] |= CONTROLLER_Y	* Controller_1_Y;
	buttons[0] |= CONTROLLER_Z	* Controller_1_Z;
	
	// Player B
	buttons[1] |= CONTROLLER_UP	* Controller_1B_Up;
	buttons[1] |= CONTROLLER_DOWN	* Controller_1B_Down;
	buttons[1] |= CONTROLLER_LEFT	* Controller_1B_Left;
	buttons[1] |= CONTROLLER_RIGHT	* Controller_1B_Right;
	buttons[1] |= CONTROLLER_START	* Controller_1B_Start;
	buttons[1] |= CONTROLLER_A	* Controller_1B_A;
	buttons[1] |= CONTROLLER_B	* Controller_1B_B;
	buttons[1] |= CONTROLLER_C	* Controller_1B_C;
	buttons[1] |= CONTROLLER_MODE	* Controller_1B_Mode;
	buttons[1] |= CONTROLLER_X	* Controller_1B_X;
	buttons[1] |= CONTROLLER_Y	* Controller_1B_Y;
	buttons[1] |= CONTROLLER_Z	* Controller_1B_Z;
	
	// Player C
	buttons[2] |= CONTROLLER_UP	* Controller_1C_Up;
	buttons[2] |= CONTROLLER_DOWN	* Controller_1C_Down;
	buttons[2] |= CONTROLLER_LEFT	* Controller_1C_Left;
	buttons[2] |= CONTROLLER_RIGHT	* Controller_1C_Right;
	buttons[2] |= CONTROLLER_START	* Controller_1C_Start;
	buttons[2] |= CONTROLLER_A	* Controller_1C_A;
	buttons[2] |= CONTROLLER_B	* Controller_1C_B;
	buttons[2] |= CONTROLLER_C	* Controller_1C_C;
	buttons[2] |= CONTROLLER_MODE	* Controller_1C_Mode;
	buttons[2] |= CONTROLLER_X	* Controller_1C_X;
	buttons[2] |= CONTROLLER_Y	* Controller_1C_Y;
	buttons[2] |= CONTROLLER_Z	* Controller_1C_Z;
	
	// Player D
	buttons[3] |= CONTROLLER_UP	* Controller_1D_Up;
	buttons[3] |= CONTROLLER_DOWN	* Controller_1D_Down;
	buttons[3] |= CONTROLLER_LEFT	* Controller_1D_Left;
	buttons[3] |= CONTROLLER_RIGHT	* Controller_1D_Right;
	buttons[3] |= CONTROLLER_START	* Controller_1D_Start;
	buttons[3] |= CONTROLLER_A	* Controller_1D_A;
	buttons[3] |= CONTROLLER_B	* Controller_1D_B;
	buttons[3] |= CONTROLLER_C	* Controller_1D_C;
	buttons[3] |= CONTROLLER_MODE	* Controller_1D_Mode;
	buttons[3] |= CONTROLLER_X	* Controller_1D_X;
	buttons[3] |= CONTROLLER_Y	* Controller_1D_Y;
	buttons[3] |= CONTROLLER_Z	* Controller_1D_Z;
	
	// Read the controller data.
	return RD_Controller(Controller_1_State, Controller_1_Type,
			     Controller_1_Counter, buttons);
}


unsigned char RD_Controller_2(void)
{
	// Read controller 1.
	
	// Create the bitfields.
	unsigned int buttons[4] = {0, 0, 0, 0};
	
	// Player A
	buttons[0] |= CONTROLLER_UP	* Controller_2_Up;
	buttons[0] |= CONTROLLER_DOWN	* Controller_2_Down;
	buttons[0] |= CONTROLLER_LEFT	* Controller_2_Left;
	buttons[0] |= CONTROLLER_RIGHT	* Controller_2_Right;
	buttons[0] |= CONTROLLER_START	* Controller_2_Start;
	buttons[0] |= CONTROLLER_A	* Controller_2_A;
	buttons[0] |= CONTROLLER_B	* Controller_2_B;
	buttons[0] |= CONTROLLER_C	* Controller_2_C;
	buttons[0] |= CONTROLLER_MODE	* Controller_2_Mode;
	buttons[0] |= CONTROLLER_X	* Controller_2_X;
	buttons[0] |= CONTROLLER_Y	* Controller_2_Y;
	buttons[0] |= CONTROLLER_Z	* Controller_2_Z;
	
	// Player B
	buttons[1] |= CONTROLLER_UP	* Controller_2B_Up;
	buttons[1] |= CONTROLLER_DOWN	* Controller_2B_Down;
	buttons[1] |= CONTROLLER_LEFT	* Controller_2B_Left;
	buttons[1] |= CONTROLLER_RIGHT	* Controller_2B_Right;
	buttons[1] |= CONTROLLER_START	* Controller_2B_Start;
	buttons[1] |= CONTROLLER_A	* Controller_2B_A;
	buttons[1] |= CONTROLLER_B	* Controller_2B_B;
	buttons[1] |= CONTROLLER_C	* Controller_2B_C;
	buttons[1] |= CONTROLLER_MODE	* Controller_2B_Mode;
	buttons[1] |= CONTROLLER_X	* Controller_2B_X;
	buttons[1] |= CONTROLLER_Y	* Controller_2B_Y;
	buttons[1] |= CONTROLLER_Z	* Controller_2B_Z;
	
	// Player C
	buttons[2] |= CONTROLLER_UP	* Controller_2C_Up;
	buttons[2] |= CONTROLLER_DOWN	* Controller_2C_Down;
	buttons[2] |= CONTROLLER_LEFT	* Controller_2C_Left;
	buttons[2] |= CONTROLLER_RIGHT	* Controller_2C_Right;
	buttons[2] |= CONTROLLER_START	* Controller_2C_Start;
	buttons[2] |= CONTROLLER_A	* Controller_2C_A;
	buttons[2] |= CONTROLLER_B	* Controller_2C_B;
	buttons[2] |= CONTROLLER_C	* Controller_2C_C;
	buttons[2] |= CONTROLLER_MODE	* Controller_2C_Mode;
	buttons[2] |= CONTROLLER_X	* Controller_2C_X;
	buttons[2] |= CONTROLLER_Y	* Controller_2C_Y;
	buttons[2] |= CONTROLLER_Z	* Controller_2C_Z;
	
	// Player D
	buttons[3] |= CONTROLLER_UP	* Controller_2D_Up;
	buttons[3] |= CONTROLLER_DOWN	* Controller_2D_Down;
	buttons[3] |= CONTROLLER_LEFT	* Controller_2D_Left;
	buttons[3] |= CONTROLLER_RIGHT	* Controller_2D_Right;
	buttons[3] |= CONTROLLER_START	* Controller_2D_Start;
	buttons[3] |= CONTROLLER_A	* Controller_2D_A;
	buttons[3] |= CONTROLLER_B	* Controller_2D_B;
	buttons[3] |= CONTROLLER_C	* Controller_2D_C;
	buttons[3] |= CONTROLLER_MODE	* Controller_2D_Mode;
	buttons[3] |= CONTROLLER_X	* Controller_2D_X;
	buttons[3] |= CONTROLLER_Y	* Controller_2D_Y;
	buttons[3] |= CONTROLLER_Z	* Controller_2D_Z;
	
	// Read the controller data.
	return RD_Controller(Controller_2_State, Controller_2_Type,
			     Controller_2_Counter, buttons);
}


static unsigned char RD_Controller(unsigned int state,
				   unsigned int type,
				   unsigned int counter,
				   unsigned int buttons[4])
{
	// Read controller 1.
	int eax, ebx;
	int jmp;
	
	eax = state;
	ebx = type;
	
	if (type & 0x10)
	{
		// TODO: Teamplayer
		return 0x00;
	}
	
	eax >>= 4;
	ebx = counter;
	ebx &= 0x03;
	eax &= 0x04;
	
	if (!(type & 0x01))
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
			eax = (buttons[0] & CONTROLLER_C ? 1 : 0);
			ebx = (buttons[0] & CONTROLLER_B ? 1 : 0);
			eax *= 4;
			ebx *= 4;
			eax |= (buttons[0] & CONTROLLER_RIGHT ? 1 : 0);
			ebx |= (buttons[0] & CONTROLLER_LEFT ? 1 : 0);
			eax *= 4;
			ebx *= 4;
			eax |= (buttons[0] & CONTROLLER_DOWN ? 1 : 0);
			ebx |= (buttons[0] & CONTROLLER_UP ? 1 : 0);
			eax = (eax * 2) + ebx + 0x40;
			return (unsigned char)eax;
		
		case First_Low:
		case Second_Low:
			eax = (buttons[0] & CONTROLLER_START ? 1 : 0);
			ebx = (buttons[0] & CONTROLLER_A ? 1 : 0);
			eax <<= 4;
			ebx <<= 4;
			eax |= (buttons[0] & CONTROLLER_DOWN ? 1 : 0);
			ebx |= (buttons[0] & CONTROLLER_UP ? 1 : 0);
			eax = (eax * 2) + ebx;
			return (unsigned char)eax;
		
		case Third_Low:
			eax = (buttons[0] & CONTROLLER_START ? 1 : 0);
			ebx = (buttons[0] & CONTROLLER_A ? 1 : 0);
			eax = (eax * 2) + ebx;
			eax <<= 4;
			return (unsigned char)eax;
		
		case Fourth_High:
			eax = (buttons[0] & CONTROLLER_C ? 1 : 0);
			ebx = (buttons[0] & CONTROLLER_B ? 1 : 0);
			eax *= 4;
			ebx *= 4;
			eax |= (buttons[0] & CONTROLLER_MODE ? 1 : 0);
			ebx |= (buttons[0] & CONTROLLER_X ? 1 : 0);
			eax *= 4;
			ebx *= 4;
			eax |= (buttons[0] & CONTROLLER_Y ? 1 : 0);
			ebx |= (buttons[0] & CONTROLLER_Z ? 1 : 0);
			eax = (eax * 2) + ebx + 0x40;
			return (unsigned char)eax;
		
		case Fourth_Low:
			eax = (buttons[0] & CONTROLLER_START ? 1 : 0);
			ebx = (buttons[0] & CONTROLLER_A ? 1 : 0);
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
