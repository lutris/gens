;
; Gens: VDP Data Misc. (x86 asm)
;
; Copyright (c) 1999-2002 by Stéphane Dallongeville
; Copyright (c) 2003-2004 by Stéphane Akhoun
; Copyright (c) 2008-2010 by David Korth
;
; This program is free software; you can redistribute it and/or modify it
; under the terms of the GNU General Public License as published by the
; Free Software Foundation; either version 2 of the License, or (at your
; option) any later version.
;
; This program is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
;
	
struc VDP_Data_Misc_t
	.Pattern_Adr:	resd 1
	.Line_7:	resd 1
	.X:		resd 1
	.Cell:		resd 1
	.Start_A:	resd 1
	.Length_A:	resd 1
	.Start_W:	resd 1
	.Length_W:	resd 1
	.Mask:		resd 1
	.Spr_End:	resd 1
	.Next_Cell:	resd 1
	.Palette:	resd 1
	.Borne:		resd 1
endstruc
