; Position-independent code macros.

%ifdef __PIC
	; Position-independent code.
	; These macros assist with PIC data location.
	
	%ifdef __OBJ_ELF
		%define	__GLOBAL_OFFSET_TABLE	_GLOBAL_OFFSET_TABLE
	%endif
	
	extern __GLOBAL_OFFSET_TABLE
	
	; Get the Global Offset Table.
	%macro get_GOT
		call	%%getgot
		%%getgot:
		pop	ebx
		add	ebx, __GLOBAL_OFFSET_TABLE + $$ - %%getgot wrt ..gotpc
	%endmacro
	
	; Get a local data variable.
	; Syntax: get_var destRegister, varName
	%macro get_localvar 2
		mov	%1, [ebx + %2 wrt ..gotoff]
	%endmacro
%else
	; Not PIC. Define dummy macros.
	%macro get_GOT
	%endmacro
	
	%macro get_localvar 2
		mov	%1, [%2]
	%endmacro
%endif
