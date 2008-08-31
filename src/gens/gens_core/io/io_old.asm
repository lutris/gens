%include "nasmhead.inc"


%macro CALC_OFFSET 0
	mov ecx, ebx
	mov edx, ebx
	and ecx, 0x1
	and edx, 0xE
	xor ecx, 0x1
	shl edx, 3
	inc ebx
%endmacro


section .data align=64

	extern Controller_1_State
	extern Controller_1_COM
	extern Controller_1_Counter
	extern Controller_1_Delay
	extern Controller_1_Type
	extern Controller_1_Up
	extern Controller_1_Down
	extern Controller_1_Left
	extern Controller_1_Right
	extern Controller_1_Start
	extern Controller_1_Mode
	extern Controller_1_A
	extern Controller_1_B
	extern Controller_1_C
	extern Controller_1_X
	extern Controller_1_Y
	extern Controller_1_Z

	extern Controller_2_State
	extern Controller_2_COM
	extern Controller_2_Counter
	extern Controller_2_Delay
	extern Controller_2_Type
	extern Controller_2_Up
	extern Controller_2_Down
	extern Controller_2_Left
	extern Controller_2_Right
	extern Controller_2_Start
	extern Controller_2_Mode
	extern Controller_2_A
	extern Controller_2_B
	extern Controller_2_C
	extern Controller_2_X
	extern Controller_2_Y
	extern Controller_2_Z

	extern Controller_1B_Type
	extern Controller_1B_Up
	extern Controller_1B_Down
	extern Controller_1B_Left
	extern Controller_1B_Right
	extern Controller_1B_Start
	extern Controller_1B_Mode
	extern Controller_1B_A
	extern Controller_1B_B
	extern Controller_1B_C
	extern Controller_1B_X
	extern Controller_1B_Y
	extern Controller_1B_Z

	extern Controller_1C_Type
	extern Controller_1C_Up
	extern Controller_1C_Down
	extern Controller_1C_Left
	extern Controller_1C_Right
	extern Controller_1C_Start
	extern Controller_1C_Mode
	extern Controller_1C_A
	extern Controller_1C_B
	extern Controller_1C_C
	extern Controller_1C_X
	extern Controller_1C_Y
	extern Controller_1C_Z

	extern Controller_1D_Type
	extern Controller_1D_Up
	extern Controller_1D_Down
	extern Controller_1D_Left
	extern Controller_1D_Right
	extern Controller_1D_Start
	extern Controller_1D_Mode
	extern Controller_1D_A
	extern Controller_1D_B
	extern Controller_1D_C
	extern Controller_1D_X
	extern Controller_1D_Y
	extern Controller_1D_Z

	extern Controller_2B_Type
	extern Controller_2B_Up
	extern Controller_2B_Down
	extern Controller_2B_Left
	extern Controller_2B_Right
	extern Controller_2B_Start
	extern Controller_2B_Mode
	extern Controller_2B_A
	extern Controller_2B_B
	extern Controller_2B_C
	extern Controller_2B_X
	extern Controller_2B_Y
	extern Controller_2B_Z

	extern Controller_2C_Type
	extern Controller_2C_Up
	extern Controller_2C_Down
	extern Controller_2C_Left
	extern Controller_2C_Right
	extern Controller_2C_Start
	extern Controller_2C_Mode
	extern Controller_2C_A
	extern Controller_2C_B
	extern Controller_2C_C
	extern Controller_2C_X
	extern Controller_2C_Y
	extern Controller_2C_Z

	extern Controller_2D_Type
	extern Controller_2D_Up
	extern Controller_2D_Down
	extern Controller_2D_Left
	extern Controller_2D_Right
	extern Controller_2D_Start
	extern Controller_2D_Mode
	extern Controller_2D_A
	extern Controller_2D_B
	extern Controller_2D_C
	extern Controller_2D_X
	extern Controller_2D_Y
	extern Controller_2D_Z

	Table_TP1:
		dd RD_Controller_1_TP.TP_L0L, RD_Controller_1_TP.TP_L0H	; 00-01
		dd RD_Controller_1_TP.TP_H0L, RD_Controller_1_TP.TP_H0H	; 02-03
		dd RD_Controller_1_TP.TP_L1L, RD_Controller_1_TP.TP_L1H	; 04-05
		dd RD_Controller_1_TP.TP_H1L, RD_Controller_1_TP.TP_H1H	; 06-07
		dd RD_Controller_1_TP.TP_L2L, RD_Controller_1_TP.TP_L2H	; 08-09
		dd RD_Controller_1_TP.TP_H2L, RD_Controller_1_TP.TP_H2H	; 0A-0B
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.TP_L3H	; 0C-0D
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.TP_H3H	; 0E-0F

		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 10-11
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 12-13
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 14-15
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 16-17
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 18-19
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 1A-1B
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 1C-1D
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 1E-1F

		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 20-21
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 22-23
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 24-25
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 26-27
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 28-29
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 2A-2B
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 2C-2D
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 2E-2F

		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 30-31
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 32-33
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 34-35
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 36-37
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 38-39
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 3A-3B
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 3C-3D
		dd RD_Controller_1_TP.UNDEF,  RD_Controller_1_TP.UNDEF	; 3E-3F

	Table_TP2:
		dd RD_Controller_2_TP.TP_L0L, RD_Controller_2_TP.TP_L0H	; 00-01
		dd RD_Controller_2_TP.TP_H0L, RD_Controller_2_TP.TP_H0H	; 02-03
		dd RD_Controller_2_TP.TP_L1L, RD_Controller_2_TP.TP_L1H	; 04-05
		dd RD_Controller_2_TP.TP_H1L, RD_Controller_2_TP.TP_H1H	; 06-07
		dd RD_Controller_2_TP.TP_L2L, RD_Controller_2_TP.TP_L2H	; 08-09
		dd RD_Controller_2_TP.TP_H2L, RD_Controller_2_TP.TP_H2H	; 0A-0B
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.TP_L3H	; 0C-0D
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.TP_H3H	; 0E-0F

		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 10-11
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 12-13
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 14-15
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 16-17
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 18-19
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 1A-1B
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 1C-1D
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 1E-1F

		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 20-21
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 22-23
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 24-25
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 26-27
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 28-29
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 2A-2B
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 2C-2D
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 2E-2F

		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 30-31
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 32-33
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 34-35
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 36-37
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 38-39
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 3A-3B
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 3C-3D
		dd RD_Controller_2_TP.UNDEF,  RD_Controller_2_TP.UNDEF	; 3E-3F


section .text align=64

	ALIGN64

	;unsigned char RD_Controller_1_TP(void)		ATTENTION, ebx est modifié et non sauvegardé ici !!!
	DECL RD_Controller_1_TP
		mov eax, [Controller_1_State]
		mov ebx, [Controller_1_Counter]
		test byte [Controller_2_State], 0xC
		jnz near .Team_Player_2
		shr eax, 3
		and ebx, 0x0F0000
		and eax, 0x0C
		shr ebx, 12
		add eax, ebx
		mov ebx, eax
		test eax, 0x4
		jz short .TP_Low
		or ebx, 0x2
	.TP_Low
		shl ebx, 3
		and ebx, 0x70
		jmp [Table_TP1 + eax]

	ALIGN4

	.TP_H0H
		mov eax, 0x73
		ret

	ALIGN4

	.TP_L0H
		mov eax, 0x3F
		ret

	ALIGN4

	.TP_H0L
	.TP_L0L
	.TP_H1H
	.TP_L1H
		mov eax, ebx
		ret

	ALIGN4

	.TP_H1L
	.TP_L1L
		mov ax, [Controller_1_Type]
		and ax, 1
		or ax, bx
		ret

	ALIGN4

	.TP_H2H
	.TP_L2H
		mov ax, [Controller_1B_Type]
		or ax, bx
		ret

	ALIGN4

	.TP_H2L
	.TP_L2L
		mov ax, [Controller_1C_Type]
		or ax, bx
		ret

	ALIGN4

	.TP_H3H
	.TP_L3H
		mov ax, [Controller_1D_Type]
		or ax, bx
		ret

	ALIGN4

	.TP_PA_DIR
		mov eax, [Controller_1_Right]
		add eax, eax
		add eax, [Controller_1_Left]
		add eax, eax
		add eax, [Controller_1_Down]
		add eax, eax
		add eax, [Controller_1_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PA_ABC
		mov eax, [Controller_1_Start]
		add eax, eax
		add eax, [Controller_1_A]
		add eax, eax
		add eax, [Controller_1_C]
		add eax, eax
		add eax, [Controller_1_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PA_XYZ
		mov eax, [Controller_1_Mode]
		add eax, eax
		add eax, [Controller_1_X]
		add eax, eax
		add eax, [Controller_1_Y]
		add eax, eax
		add eax, [Controller_1_Z]
		add eax, ebx
		ret

	ALIGN4

	.TP_PB_DIR
		mov eax, [Controller_1B_Right]
		add eax, eax
		add eax, [Controller_1B_Left]
		add eax, eax
		add eax, [Controller_1B_Down]
		add eax, eax
		add eax, [Controller_1B_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PB_ABC
		mov eax, [Controller_1B_Start]
		add eax, eax
		add eax, [Controller_1B_A]
		add eax, eax
		add eax, [Controller_1B_C]
		add eax, eax
		add eax, [Controller_1B_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PB_XYZ
		mov eax, [Controller_1B_Mode]
		add eax, eax
		add eax, [Controller_1B_X]
		add eax, eax
		add eax, [Controller_1B_Y]
		add eax, eax
		add eax, [Controller_1B_Z]
		add eax, ebx
		ret

	ALIGN4

	.TP_PC_DIR
		mov eax, [Controller_1C_Right]
		add eax, eax
		add eax, [Controller_1C_Left]
		add eax, eax
		add eax, [Controller_1C_Down]
		add eax, eax
		add eax, [Controller_1C_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PC_ABC
		mov eax, [Controller_1C_Start]
		add eax, eax
		add eax, [Controller_1C_A]
		add eax, eax
		add eax, [Controller_1C_C]
		add eax, eax
		add eax, [Controller_1C_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PC_XYZ
		mov eax, [Controller_1C_Mode]
		add eax, eax
		add eax, [Controller_1C_X]
		add eax, eax
		add eax, [Controller_1C_Y]
		add eax, eax
		add eax, [Controller_1C_Z]
		add eax, ebx
		ret

	ALIGN4

	.TP_PD_DIR
		mov eax, [Controller_1D_Right]
		add eax, eax
		add eax, [Controller_1D_Left]
		add eax, eax
		add eax, [Controller_1D_Down]
		add eax, eax
		add eax, [Controller_1D_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PD_ABC
		mov eax, [Controller_1D_Start]
		add eax, eax
		add eax, [Controller_1D_A]
		add eax, eax
		add eax, [Controller_1D_C]
		add eax, eax
		add eax, [Controller_1D_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PD_XYZ
		mov eax, [Controller_1D_Mode]
		add eax, eax
		add eax, [Controller_1D_X]
		add eax, eax
		add eax, [Controller_1D_Y]
		add eax, eax
		add eax, [Controller_1D_Z]
		add eax, ebx
		ret

	ALIGN4

	.UNDEF
		mov eax, 0xF
		or eax, ebx
		ret

	ALIGN4
	
	.Table_Get_Cont
		dd Controller_1_Type
		dd Controller_1B_Type
		dd Controller_1C_Type
		dd Controller_1D_Type
	
	ALIGN32

	.Team_Player_2
		push edx
		mov edx, [Controller_2_State]
		test edx, 0x40
		jnz short .TP2_Spec

		and edx, 0x30
		shr edx, 2
		mov edx, [.Table_Get_Cont + edx]

;		dec ebx
		shr eax, 4
		and ebx, 0x03
		and eax, 0x04
		test byte [edx + 0], 1				; 6 buttons controller
		jnz short .TP2_Six_Buttons

		xor ebx, ebx

	.TP2_Six_Buttons
		jmp [.TP2_Table_Cont + ebx * 8 + eax]

	ALIGN4
	
	.TP2_Table_Cont:

		dd .TP2_First_Low, .TP2_First_High
		dd .TP2_Second_Low, .TP2_Second_High
		dd .TP2_Third_Low, .TP2_Third_High
		dd .TP2_Fourth_Low, .TP2_Fourth_High

	ALIGN4

	.TP2_Spec
		mov eax, 0x70
		pop edx
		ret

	ALIGN32

	.TP2_First_High
	.TP2_Second_High
	.TP2_Third_High
		mov eax, [edx + 36]
		mov ebx, [edx + 32]
		lea eax, [eax * 4]
		lea ebx, [ebx * 4]
		or eax, [edx + 16]
		or ebx, [edx + 12]
		lea eax, [eax * 4]
		lea ebx, [ebx * 4]
		or eax, [edx + 8]
		or ebx, [edx + 4]
		lea eax, [eax * 2 + ebx + 0x40]
		pop edx
		ret

	ALIGN32

	.TP2_First_Low
	.TP2_Second_Low
  		mov eax, [edx + 20]
		mov ebx, [edx + 28]
		shl eax, 4
		shl ebx, 4
		or eax, [edx + 8]
		or ebx, [edx + 4]
		lea eax, [eax * 2 + ebx]
		pop edx
		ret

	ALIGN32

	.TP2_Third_Low
  		mov eax, [Controller_1_Start]
		mov ebx, [Controller_1_A]
		lea eax, [eax * 2 + ebx]
		shl eax, 4
		pop edx
		ret

	ALIGN32

	.TP2_Fourth_High
		mov eax, [edx + 36]
		mov ebx, [edx + 32]
		lea eax, [eax * 4]
		lea ebx, [ebx * 4]
		or eax, [edx + 24]
		or ebx, [edx + 40]
		lea eax, [eax * 4]
		lea ebx, [ebx * 4]
		or eax, [edx + 44]
		or ebx, [edx + 48]
		lea eax, [eax * 2 + ebx + 0x40]
		pop edx
		ret

	ALIGN32

	.TP2_Fourth_Low
  		mov eax, [edx + 20]
		mov ebx, [edx + 28]
		lea eax, [eax * 2 + ebx]
		shl eax, 4
		or eax, 0xF
		pop edx
		ret


	ALIGN32

	;unsigned char RD_Controller_2_TP(void) 	ATTENTION, ebx est modifié et non sauvegardé ici !!!
	DECL RD_Controller_2_TP

		mov eax, [Controller_2_State]
		mov ebx, [Controller_2_Counter]
		shr eax, 3
		and ebx, 0x0F0000
		and eax, 0x0C
		shr ebx, 12
		add eax, ebx
		mov ebx, eax
		test eax, 0x4
		jz short .TP_Low
		or ebx, 0x2
	.TP_Low
		shl ebx, 3
		and ebx, 0x70
		jmp [Table_TP1 + eax]		; TODO: Should this be TP1 or TP2?

	ALIGN4

	.TP_H0H
		mov eax, 0x73
		ret

	ALIGN4

	.TP_L0H
		mov eax, 0x3F
		ret

	ALIGN4

	.TP_H0L
	.TP_L0L
	.TP_H1H
	.TP_L1H
		mov eax, ebx
		ret

	ALIGN4

	.TP_H1L
	.TP_L1L
		mov ax, [Controller_2_Type]
		and ax, 1
		or ax, bx
		ret

	ALIGN4

	.TP_H2H
	.TP_L2H
		mov ax, [Controller_2B_Type]
		or ax, bx
		ret

	ALIGN4

	.TP_H2L
	.TP_L2L
		mov ax, [Controller_2C_Type]
		or ax, bx
		ret

	ALIGN4

	.TP_H3H
	.TP_L3H
		mov ax, [Controller_2D_Type]
		or ax, bx
		ret

	ALIGN4

	.TP_PA_DIR
		mov eax, [Controller_2_Right]
		add eax, eax
		add eax, [Controller_2_Left]
		add eax, eax
		add eax, [Controller_2_Down]
		add eax, eax
		add eax, [Controller_2_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PA_ABC
		mov eax, [Controller_2_Start]
		add eax, eax
		add eax, [Controller_2_A]
		add eax, eax
		add eax, [Controller_2_C]
		add eax, eax
		add eax, [Controller_2_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PA_XYZ
		mov eax, [Controller_2_Mode]
		add eax, eax
		add eax, [Controller_2_X]
		add eax, eax
		add eax, [Controller_2_Y]
		add eax, eax
		add eax, [Controller_2_Z]
		add eax, ebx
		ret

	ALIGN4

	.TP_PB_DIR
		mov eax, [Controller_2B_Right]
		add eax, eax
		add eax, [Controller_2B_Left]
		add eax, eax
		add eax, [Controller_2B_Down]
		add eax, eax
		add eax, [Controller_2B_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PB_ABC
		mov eax, [Controller_2B_Start]
		add eax, eax
		add eax, [Controller_2B_A]
		add eax, eax
		add eax, [Controller_2B_C]
		add eax, eax
		add eax, [Controller_2B_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PB_XYZ
		mov eax, [Controller_2B_Mode]
		add eax, eax
		add eax, [Controller_2B_X]
		add eax, eax
		add eax, [Controller_2B_Y]
		add eax, eax
		add eax, [Controller_2B_Z]
		add eax, ebx
		ret

	ALIGN4

	.TP_PC_DIR
		mov eax, [Controller_2C_Right]
		add eax, eax
		add eax, [Controller_2C_Left]
		add eax, eax
		add eax, [Controller_2C_Down]
		add eax, eax
		add eax, [Controller_2C_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PC_ABC
		mov eax, [Controller_2C_Start]
		add eax, eax
		add eax, [Controller_2C_A]
		add eax, eax
		add eax, [Controller_2C_C]
		add eax, eax
		add eax, [Controller_2C_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PC_XYZ
		mov eax, [Controller_2C_Mode]
		add eax, eax
		add eax, [Controller_2C_X]
		add eax, eax
		add eax, [Controller_2C_Y]
		add eax, eax
		add eax, [Controller_2C_Z]
		add eax, ebx
		ret

	ALIGN4

	.TP_PD_DIR
		mov eax, [Controller_2D_Right]
		add eax, eax
		add eax, [Controller_2D_Left]
		add eax, eax
		add eax, [Controller_2D_Down]
		add eax, eax
		add eax, [Controller_2D_Up]
		add eax, ebx
		ret

	ALIGN4

	.TP_PD_ABC
		mov eax, [Controller_2D_Start]
		add eax, eax
		add eax, [Controller_2D_A]
		add eax, eax
		add eax, [Controller_2D_C]
		add eax, eax
		add eax, [Controller_2D_B]
		add eax, ebx
		ret

	ALIGN4

	.TP_PD_XYZ
		mov eax, [Controller_2D_Mode]
		add eax, eax
		add eax, [Controller_2D_X]
		add eax, eax
		add eax, [Controller_2D_Y]
		add eax, eax
		add eax, [Controller_2D_Z]
		add eax, ebx
		ret

	ALIGN4

	.UNDEF
		mov eax, 0xF
		or eax, ebx
		ret


	ALIGN32


	;void Make_IO_Table(void)
	DECL Make_IO_Table
		push eax
		push ebx
		push ecx
		push edx

	.P1A
		mov ebx, 1
		test byte [Controller_1_Type], 1
		jnz short .P1A_6

	.P1A_3
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PA_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PA_ABC
		jmp short .P1B

	ALIGN4
	
	.P1A_6
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PA_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PA_ABC
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PA_XYZ
		jmp short .P1B

	ALIGN4

	.P1B
		test byte [Controller_1B_Type], 1
		jnz short .P1B_6

	.P1B_3
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PB_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PB_ABC
		jmp short .P1C

	ALIGN4
	
	.P1B_6
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PB_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PB_ABC
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PB_XYZ
		jmp short .P1C

	ALIGN4

	.P1C
		test byte [Controller_1C_Type], 1
		jnz short .P1C_6

	.P1C_3
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PC_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PC_ABC
		jmp short .P1D

	ALIGN4
	
	.P1C_6
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PC_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PC_ABC
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PC_XYZ
		jmp short .P1D

	ALIGN4

	.P1D
		test byte [Controller_1D_Type], 1
		jnz short .P1D_6

	.P1D_3
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PD_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PD_ABC
		jmp short .P2A

	ALIGN4
	
	.P1D_6
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PD_DIR
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PD_ABC
		CALC_OFFSET
		mov dword [Table_TP1 + 0x30 + edx + ecx * 4], RD_Controller_1_TP.TP_PD_XYZ
		jmp short .P2A

	ALIGN4

	.P2A
		mov ebx, 1
		test byte [Controller_2_Type], 1
		jnz short .P2A_6

	.P2A_3
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PA_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PA_ABC
		jmp short .P2B

	ALIGN4
	
	.P2A_6
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PA_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PA_ABC
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PA_XYZ
		jmp short .P2B

	ALIGN4

	.P2B
		test byte [Controller_2B_Type], 1
		jnz short .P2B_6

	.P2B_3
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PB_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PB_ABC
		jmp short .P2C

	ALIGN4
	
	.P2B_6
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PB_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PB_ABC
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PB_XYZ
		jmp short .P2C

	ALIGN4

	.P2C
		test byte [Controller_2C_Type], 1
		jnz short .P2C_6

	.P2C_3
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PC_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PC_ABC
		jmp short .P2D

	ALIGN4
	
	.P2C_6
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PC_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PC_ABC
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PC_XYZ
		jmp short .P2D

	ALIGN4

	.P2D
		test byte [Controller_2D_Type], 1
		jnz short .P2D_6

	.P2D_3
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PD_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PD_ABC
		jmp short .End

	ALIGN4
	
	.P2D_6
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PD_DIR
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PD_ABC
		CALC_OFFSET
		mov dword [Table_TP2 + 0x30 + edx + ecx * 4], RD_Controller_2_TP.TP_PD_XYZ
		jmp short .End

	ALIGN4

	.End
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
