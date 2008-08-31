 /**
  * Gens: Controller I/O
  */

#include <stdio.h>

#include "io.h"

#define CREATE_CONTROLLER_VARIABLES(player)			\
	unsigned int Controller_ ## player ## _Type	= 0;	\
	unsigned int Controller_ ## player ## _Up	= 1;	\
	unsigned int Controller_ ## player ## _Down	= 1;	\
	unsigned int Controller_ ## player ## _Left	= 1;	\
	unsigned int Controller_ ## player ## _Right	= 1;	\
	unsigned int Controller_ ## player ## _Start	= 1;	\
	unsigned int Controller_ ## player ## _Mode	= 1;	\
	unsigned int Controller_ ## player ## _A	= 1;	\
	unsigned int Controller_ ## player ## _B	= 1;	\
	unsigned int Controller_ ## player ## _C	= 1;	\
	unsigned int Controller_ ## player ## _X	= 1;	\
	unsigned int Controller_ ## player ## _Y	= 1;	\
	unsigned int Controller_ ## player ## _Z	= 1;	\

unsigned int Controller_1_State		= 0x40;
unsigned int Controller_1_COM		= 0;
unsigned int Controller_1_Counter	= 0;
unsigned int Controller_1_Delay		= 0;

unsigned int Controller_2_State		= 0x40;
unsigned int Controller_2_COM		= 0;
unsigned int Controller_2_Counter	= 0;
unsigned int Controller_2_Delay		= 0;

CREATE_CONTROLLER_VARIABLES(1);
CREATE_CONTROLLER_VARIABLES(1B);
CREATE_CONTROLLER_VARIABLES(1C);
CREATE_CONTROLLER_VARIABLES(1D);

CREATE_CONTROLLER_VARIABLES(2);
CREATE_CONTROLLER_VARIABLES(2B);
CREATE_CONTROLLER_VARIABLES(2C);
CREATE_CONTROLLER_VARIABLES(2D);

// Select Line status
enum SelectLine
{
	First_Low	= 0x00,
	First_High	= 0x01,
	Second_Low	= 0x02,
	Second_High	= 0x03,
	Third_Low	= 0x04,
	Third_High	= 0x05,
	Fourth_Low	= 0x06,
	Fourth_High	= 0x07,
};

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


static unsigned char RD_Controller(unsigned int state,
				   unsigned int type,
				   unsigned int counter,
				   unsigned int buttons[4]);


#define CREATE_CONTROLLER_BITFIELD(bitfield, player)				\
{										\
	bitfield |= CONTROLLER_UP	* Controller_ ## player ## _Up;		\
	bitfield |= CONTROLLER_DOWN	* Controller_ ## player ## _Down;	\
	bitfield |= CONTROLLER_LEFT	* Controller_ ## player ## _Left;	\
	bitfield |= CONTROLLER_RIGHT	* Controller_ ## player ## _Right;	\
	bitfield |= CONTROLLER_START	* Controller_ ## player ## _Start;	\
	bitfield |= CONTROLLER_A	* Controller_ ## player ## _A;		\
	bitfield |= CONTROLLER_B	* Controller_ ## player ## _B;		\
	bitfield |= CONTROLLER_C	* Controller_ ## player ## _C;		\
	bitfield |= CONTROLLER_MODE	* Controller_ ## player ## _Mode;	\
	bitfield |= CONTROLLER_X	* Controller_ ## player ## _X;		\
	bitfield |= CONTROLLER_Y	* Controller_ ## player ## _Y;		\
	bitfield |= CONTROLLER_Z	* Controller_ ## player ## _Z;		\
}


unsigned char RD_Controller_1(void)
{
	// Read controller 1.
	
	if (Controller_1_Type & 0x10)
	{
		// TeamPlayer.
		//return RD_Controller_TP1();
	}
	
	// Create the bitfields.
	// TODO: This will be unnecessary when controllers are converted to bitfields.
	unsigned int buttons[4] = {0, 0, 0, 0};
	CREATE_CONTROLLER_BITFIELD(buttons[0], 1);
	CREATE_CONTROLLER_BITFIELD(buttons[1], 1B);
	CREATE_CONTROLLER_BITFIELD(buttons[2], 1C);
	CREATE_CONTROLLER_BITFIELD(buttons[3], 1D);
	
	// Read the controller data.
	return RD_Controller(Controller_1_State, Controller_1_Type,
			     Controller_1_Counter, buttons);
}


unsigned char RD_Controller_2(void)
{
	// Read controller 2.
	
	// Create the bitfields.
	// TODO: This will be unnecessary when controllers are converted to bitfields.
	unsigned int buttons[4] = {0, 0, 0, 0};
	CREATE_CONTROLLER_BITFIELD(buttons[0], 2);
	CREATE_CONTROLLER_BITFIELD(buttons[1], 2B);
	CREATE_CONTROLLER_BITFIELD(buttons[2], 2C);
	CREATE_CONTROLLER_BITFIELD(buttons[3], 2D);
	
	// Read the controller data.
	return RD_Controller(Controller_2_State, Controller_2_Type,
			     Controller_2_Counter, buttons);
}


static unsigned char RD_Controller(unsigned int state,
				   unsigned int type,
				   unsigned int counter,
				   unsigned int buttons[4])
{
	// Read the specified controller.
	
	int SelectLine_State = ((state >> 6) & 0x01);
	if (type & 0x01)
	{
		// 6-button controller. Add the counter value.
		SelectLine_State += ((counter & 0x03) * 2);
	}
	
	unsigned char out = 0;
	switch (SelectLine_State)
	{
		case First_High:
		case Second_High:
		case Third_High:
			// Format: 01CBRLDU
			if (buttons[0] & CONTROLLER_UP)
				out |= 0x01;
			if (buttons[0] & CONTROLLER_DOWN)
				out |= 0x02;
			if (buttons[0] & CONTROLLER_LEFT)
				out |= 0x04;
			if (buttons[0] & CONTROLLER_RIGHT)
				out |= 0x08;
			if (buttons[0] & CONTROLLER_B)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_C)
				out |= 0x20;
			
			return (out | 0x40);
		
		case First_Low:
		case Second_Low:
			// Format: 00SA00DU
			if (buttons[0] & CONTROLLER_UP)
				out |= 0x01;
			if (buttons[0] & CONTROLLER_DOWN)
				out |= 0x02;
			if (buttons[0] & CONTROLLER_A)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_START)
				out |= 0x20;
			
			return out;
		
		case Third_Low:
			// Format: 00SA0000
			if (buttons[0] & CONTROLLER_A)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_START)
				out |= 0x20;
			
			return out;
		
		case Fourth_High:
			// Format: 01CBMXYZ
			if (buttons[0] & CONTROLLER_Z)
				out |= 0x01;
			if (buttons[0] & CONTROLLER_Y)
				out |= 0x02;
			if (buttons[0] & CONTROLLER_X)
				out |= 0x04;
			if (buttons[0] & CONTROLLER_MODE)
				out |= 0x08;
			if (buttons[0] & CONTROLLER_B)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_C)
				out |= 0x20;
			
			return (out | 0x40);
		
		case Fourth_Low:
			// Format: 00SA1111
			if (buttons[0] & CONTROLLER_A)
				out |= 0x10;
			if (buttons[0] & CONTROLLER_START)
				out |= 0x20;
			
			return (out | 0x0F);
	}
	
	// Shouldn't happen...
	printf("%s: Invalid Select Line State: %d\n", __func__, SelectLine_State);
	return 0x00;
}


unsigned char WR_Controller_1(unsigned char data)
{
	if (!((Controller_1_State & 0x40) && (!(data & 0x40))))
		Controller_1_Counter++;
	
	Controller_1_Delay = 0;
	
	if (!(Controller_1_Type & 0x10))
	{
		// Not Team Player, so skip the rest of the function.
		Controller_1_State = data;
		return data;
	}
	
	// Team Player adapter
	if (!((Controller_1_State & 0x20) && (!(data & 0x20))))
		Controller_1_Counter += (1 << 16);
	
	Controller_1_State = data;
	return data;
}


unsigned char WR_Controller_2(unsigned char data)
{
	if (!((Controller_2_State & 0x40) && (!(data & 0x40))))
		Controller_2_Counter++;
	
	// Team Player on Port 1
	// TODO: Why is this here?
	if ((Controller_1_Type & 0x10) & (data & 0x0C))
		Controller_1_Counter = 0;
	
	Controller_2_Delay = 0;
	
	if (!(Controller_2_Type & 0x10))
	{
		// Not Team Player, so skip the rest of the function.
		Controller_2_State = data;
		return data;
	}
	
	// Team Player adapter
	if (!((Controller_2_State & 0x20) && (!(data & 0x20))))
	{
		Controller_2_Counter += (1 << 16);
	}
	
	Controller_2_State = data;
	return data;
}


void Fix_Controllers(void)
{
	if (Controller_1_Delay <= 25)
		Controller_1_Delay++;
	else
	{
		//Controller_1_Delay = 0; // commented out in asm
		Controller_1_Counter = 0;
	}
	
	if (Controller_2_Delay <= 25)
		Controller_2_Delay++;
	else
	{
		//Controller_2_Delay = 0; // commented out in asm
		Controller_2_Counter = 0;
	}
}
