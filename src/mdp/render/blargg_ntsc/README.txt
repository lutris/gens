Blargg's NTSC Filter for the Sega Mega Drive.
Packaged for the MDP Plugin Interface v1.0.0.

Copyright (c) 2006 by Shay Green.
MDP version Copyright (c) 2008-2009 by David Korth.

Based on md_ntsc v0.1.2.

================================================================

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

================================================================

MDP Version History

v1.1.1: Minor bugfix release.

- The reimplementation of the double-scan algorithm in v1.1.0 missed
  the last scanline when interpolating, leading to a blank line that
  never gets updated. This fixes it so the last line is redrawn
  correctly.

v1.1.0: Bugfix release.

- Reimplemented the double-scan algorithm inside of the main blitter.
  This eliminates the need to read the already-written pixels from the
  destination buffer, which caused severe lag issues on Win32 when
  using DirectDraw if the backbuffer was set to use video memory.

v1.0.0: First public release.
