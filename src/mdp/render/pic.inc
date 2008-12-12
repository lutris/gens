; Position-independent code macros.

%ifdef __PIC
	; Position-independent code.
	; These macros assist with PIC data location.
	
	%ifdef __OBJ_ELF
		%define	__GLOBAL_OFFSET_TABLE	_GLOBAL_OFFSET_TABLE
	%endif
	
	extern __GLOBAL_OFFSET_TABLE
	
	; Get the Global Offset Table.
	%macro get_GOT 0
		call	%%getgot
		%%getgot:
		pop	ebx
		add	ebx, __GLOBAL_OFFSET_TABLE + $$ - %%getgot wrt ..gotpc
	%endmacro
	
	; Get a local data variable.
	; Syntax: get_localvar destRegister, varName
	%macro get_localvar 2
		mov	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Get a local data variable. (mmx destination register)
	; Syntax: get_localvar_mmx destRegister, varName
	%macro get_localvar_mmx 2
		movq	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Write a local data variable.
	; Syntax: put_localvar varName, srcRegister
	%macro put_localvar 2
		mov	[ebx + %1 wrt ..gotoff], %2
	%endmacro
	
	; Write a local data variable. (mmx source register)
	; Syntax: put_localvar_mmx varName, srcRegister
	%macro put_localvar_mmx 2
		movq	[ebx + %1 wrt ..gotoff], %2
	%endmacro
	
	; Logical "or" an mmx register with a data variable.
	; Syntax: por_localvar_mmx destRegister, varName
	%macro por_localvar_mmx 2
		por	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
	
	; Logical "and" an mmx register with a data variable.
	; Syntax: pand_localvar_mmx destRegister, varName
	%macro pand_localvar_mmx 2
		pand	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
%else
	; Not PIC. Define dummy macros.
	%macro get_GOT 0
	%endmacro
	
	%macro get_localvar 2
		mov	%1, [%2]
	%endmacro
	
	%macro get_localvar_mmx 2
		movq	%1, [%2]
	%endmacro
	
	%macro put_localvar 2
		mov	[%1], %2
	%endmacro
	
	%macro put_localvar_mmx 2
		movq	[%1], %2
	%endmacro
	
	%macro por_localvar_mmx 2
		por	%1, [%2]
	%endmacro
	
	%macro pand_localvar_mmx 2
		pand	%1, [%2]
	%endmacro
%endif
