Gens/GS r7

Copyright (c) 1999-2002 by Stéphane Dallongeville
Copyright (c) 2003-2004 by Stéphane Akhoun
Copyright (c) 2008-2009 by David Korth

This version of Gens is maintained by David Korth.
E-mail: gerbilsoft@verizon.net

For news on Gens/GS, visit Sonic Retro:
http://www.sonicretro.org

================================================================

1. What is Gens/GS?

Gens/GS r7 is David Korth's fork of Gens, initially based on
Gens for Linux v2.15.2. The number after the "r" indicates the
Gens/GS release. Releases are usually made after a significant new
feature has been added, with minor releases if a major bug is found.

================================================================

2. Short History of Gens/GS (mostly taken from the Gens for Linux readme)

Stephane Dallongeville is the author of Gens (Windows version).
Caz has made a BeOS port, using Allegro. But this port was command line
oriented. And though Allegro is a great library, its GUI part is ... well ... rather ugly.

Thus, I (Stephane Akhoun) wanted to use GTK+ for the GUI. But as mixing
Allegro and GTK+ events loop was a bit of challenge (though possible) I
decided to switch to SDL. Fortunately, it was very easy (only 2 function 
calls to change !).

Finally, in the port process, my main work is the GTK+ GUI, and some work
here and there. Caz ported the gens core : I wanted to thank him/her one
more time for his/her pieces of advice.

Thanks also for Lester Barrows (aryarya) and El Pelos (wah wah 69) for
their contribution, patches, joystick support, ...
For all users that helped and help us in the porting process : thank you very much !

Gens/GS picks up where Gens for Linux left off. Among other things, it
integrates the Windows version into the same codebase. See the ChangeLog
file for a comprehensive version history of Gens/GS.

================================================================

3. Minimum System Software Requirements

Linux: [runtime requirements]
- SDL 1.2 (SDL 1.1.3 has been reported to work previously.)
- GTK+ 2.4

Linux: [build requirements]
- gcc 2.95
  - gcc 4.x recommended [there may be some unknown 2.95/3.x compatibility issues]
- nasm 2.05.1
- autoconf-2.63
- automake-1.10

Windows: [runtime requirements]
- Windows 9x, Me, 2000 or later.
  - Tested on 95, 98SE, 2000, and XP.
- DirectX 3 or later for keyboard input and GDI rendering. (No audio)
- DirectX 6 or later for joystick input and DirectDraw 4 rendering, and sound.

Windows: [build requirements]
- MinGW and mSYS
- MinGW Runtime 3.14, 3.15.1, or later
  - MinGW Runtime 3.15 has a bug that causes command-line parsing to crash.
- DirectX headers (specifically, ddraw.h, dinput.h, and dsound.h)
- nasm 2.05.1
- autoconf-2.63
- automake-1.10

================================================================

3.1. Easy Windows Build Instructions

Since not everyone is a computer wiz, I created an easier build system for
Win32. It consists of four easy steps.

1. Copy all files from C:\WINDOWS into the Gens/GS source code directory.

2. Run "compile.hta". Internet Explorer will ask you to download and install
an unsigned ActiveX control. Click Yes. If your antivirus program warns you,
click "Ignore". "compile.hta" will generate a Visual Studio 2008 project,
"gensgs.sln".

3. Open "gensgs.sln" in Visual Studio 2008, Professional Edition or higher.
You must use a legally licensed copy of Visual Studio 2008; if your copy is
pirated, you will be redirected to newegg.com, where you may purchase a copy
of Visual Studio 2008, Professional Edition, for the low price of $649.99.

4. Compile the project using Visual Studio 2008. The final binary will be
called "gens.exe".

================================================================

3.2. Feature Requests

Want a new feature in Gens/GS? Here's a proven method for getting it done.

1. Contact GerbilSoft asking for a new feature.

2. Don't tell him what the new feature is, but claim that Gens/GS is
   incomplete without it.

3. ?????

4. PROFIT!

If it can work for administrators of popular websites, it can work for you!

================================================================

4. Where are gens.cfg and language.dat stored?

On Linux: ~/.gens/
On Windows: the same directory as gens.exe

================================================================

5. What is the syntax of gens.cfg ?

gens.cfg uses the standard INI format, which looks like this:

[section1]
key=value
...
...

[section2]
key=value
...

================================================================

6. How do I configure controller inputs without the GUI?

----------------------------------------------------------------

6.1. Old Gens joystick configuration format.

Older versions of Gens/GS (m6 and older), and the original Gens, used
a relatively limited format for storing controller configuration.

Older joystick input format: (16-bit binary)
0001 JJJJ TTTT WWWW

where:
- 0001: High nybble is always set to 1.
- JJJJ: Joystick number.
- TTTT: Type of input. (0 == axis; 1-7 == button; 8 == POV hat)
- WWWW: Which axis, button, or POV hat.

For axes, the following values are allowed:
- 1: Axis 1, negative. (Up) [TODO: Original documentation said Down, verify this!]
- 2: Axis 1, positive. (Down) [TODO: Original documentation said Up, verify this!]
- 3: Axis 0, negative. (Left)
- 4: Axis 0, positive. (Right)
- 5: Axis 3, negative.
- 6: Axis 3, positive.
- 7: Axis 2, negative.
- 8: Axis 2, positive.
- 9: Axis 5, negative.
- 10: Axis 5, positive.
- 11: Axis 4, negative.
- 12: Axis 4, positive.

Among the problems with this scheme are the "backwards" notation (axis 1 before 0)
and the 6 axis limitation. On Windows, the 6 axis limitation (actually 5, since
the DirectInput handler ignores the 6th axis) isn't much of a problem, since
most Windows joystick drivers map some axes to POV hats. On Linux, most
joystick drivers map all axes to axes instead of POV hats, which does create
a problem. Notably, the Xbox 360 controller's D-pad is mapped as axes 6 and 7,
which doesn't work with this scheme.

For buttons, TTTT WWWW is taken as a whole byte. The number 0x10 is subtracted
from the byte, resulting in the button value. For example, the first button,
button 0, would have a TTTT WWWW value of 0x10.

For POV hats, the following format is used: NNDD

where:
- NN: POV hat number.
- DD: Direction. (0 = up; 1 = right; 2 = down; 3 = left)

Thus, the "Up" direction on the first POV hat on the first joystick would be
indicated as 0x1080.

----------------------------------------------------------------

6.2. New Gens/GS joystick configuration format.

Starting with Gens/GS r7, a newer format is used for storing joystick
configuration. The new format allows up to 128 axes, 256 buttons, and
64 POV hats.

New joystick input format: (16-bit binary)
1TTT JJJJ WWWW WWWW

where:
- 1: High bit is always set to indicate new joystick input format.
- TTT: Type of input. (0 == axis; 1 == button; 2 == POV hat)
- JJJJ: Joystick number.
- WWWW WWWW: Which axis, button, or POV hat.

For axes, the following format is used: NNNN NNND

where:
- NNNN NNN: Axis number.
- D: Axis direction. (0 == negative; 1 == positive)

For buttons, the entire WWWW WWWW value is used as the button number.

For POV hats, the following format is used: NNNN NNDD

where:
- NNNN NN: POV hat number.
- DD: Direction. (0 = up; 1 = right; 2 = down; 3 = left)
