Gens/GS v2.15.5/GS-m6 (Gens/GS)

Copyright (c) 1999-2002 by Stéphane Dallongeville
Copyright (c) 2003-2004 by Stéphane Akhoun
Copyright (c) 2008 by David Korth

This version of Gens is maintained by David Korth.
E-mail: gerbilsoft@verizon.net

For news on Gens/GS, visit Sonic Retro:
http://www.sonicretro.org

================================================================

1. What is Gens/GS?

Gens v2.15.5/GS-m6 is David Korth's fork of Gens, initially based
on Gens for Linux v2.15.5. The number after the "m" indicates the
milestone. Releases are usually made after a significant new
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
- nasm 0.98.38

Windows: [runtime requirements]
- Windows 9x, Me, 2000 or later.
  - Tested on 95, 98SE, 2000, and XP.
- DirectX 6

Windows: [build requirements]
- MinGW and mSYS
- MinGW Runtime 3.14, 3.15.1, or later
  - MinGW Runtime 3.15 has a bug that causes command-line parsing to crash.
- Windows platform SDK (or headers from the Wine project)
- nasm 0.98.38

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

If you must edit gens.cfg by hand to configure your joystick:

Down  : 4096 + 256*which + 1
Up    : 4096 + 256*which + 2
Left  : 4096 + 256*which + 3
Right : 4096 + 256*which + 4

Buttons : 4112 + 256*which + x 

where which = 0 for first joystick,
            = 1 for second joystick,
      x     = the number of the button
		  
Here how a typical gens.cfg with joystick would look like (modify for your own needs)

P1.Up=4098
P1.Down=4097
P1.Left=4099
P1.Right=4100
P1.Start=4112
P1.A=4113
...
