===[CONTENTS]==================================================================

1 - ABOUT
2 - LICENSE/DISCLAIMER
3 - FEEDBACK
4 - (RE)COMPILING

===[1 - ABOUT]=================================================================

savof v1.0 (December 2022)
Copyright (C) 2016-2022 Norbert de Jonge <nlmdejonge@gmail.com>

Create, view and modify Prince of Persia for DOS save (PRINCE.SAV) and hall of fame (PRINCE.HOF) files.
The savof website can be found at [ https://github.com/EndeavourAccuracy/savof ].

===[2 - LICENSE/DISCLAIMER]====================================================

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see [ www.gnu.org/licenses/ ].

===[3 - FEEDBACK]==============================================================

If savof crashes, gets compilation errors or crashes while building, send an e-mail to [ nlmdejonge@gmail.com ]. Make sure to describe exactly what actions triggered the bug and the precise symptoms of the bug. If possible, also include: 'uname -a', 'gcc -v', 'sdl2-config --version', and 'savof --version'.

===[4 - (RE)COMPILING]=========================================================

GNU/Linux
=========

$ make

You will need libsdl2-dev, libsdl2-image-dev and libsdl2-ttf-dev.

Windows (32-bit)
================

1) Set up Dev-C++:

1.1 Download

https://sourceforge.net/projects/orwelldevcpp/files/Setup%20Releases/
or
https://downloads.sourceforge.net/project/orwelldevcpp/Setup%20Releases/
  Dev-Cpp%205.11%20TDM-GCC%204.9.2%20Setup.exe

1.2 Install

Simply run the executable.

2) Install SDL2's MinGW libraries:

2.1 Download

https://libsdl.org/release/
  SDL2-devel-2.0.7-mingw.tar.gz
https://libsdl.org/projects/SDL_ttf/release/
  SDL2_ttf-devel-2.0.14-mingw.tar.gz
https://libsdl.org/projects/SDL_image/release/
  SDL2_image-devel-2.0.2-mingw.tar.gz

2.2 Install

Unpack the packages.

For all three packages, copy the i686-w64-mingw32/ directories into the Dev-Cpp/MinGW64/ directory. Make sure to use the zlib1.dll from SDL2_image (and NOT from SDL2_ttf).

GNU/Linux users who use Wine can find this directory at:
~/.wine/drive_c/Program Files (x86)/Dev-Cpp/MinGW64/

Copy the Dev-Cpp/MinGW64/i686-w64-mingw32/bin/*.dll files to the savof directory.
(You do not need libjpeg-9.dll, libtiff-5.dll and libwebp-7.dll.)

3) Compile

Start Dev-C++.

Go to: File->New->Project...
Basic->Console Application
C Project
Name: savof
Save savof.dev to the savof directory.

Go to: Project->Remove From Project...
Select main.c and press Delete.

Project->Add To Project...
savof.c

Go to: Project->Project Options...->Compiler
Set "Base compiler set:" to "TDM-GCC 4.9.2 32-bit Release".
(Discard customizations if necessary.)

Go to: Project->Project Options...->Parameters
In the C compiler field, add:
-m32 -O2 -Wno-unused-result -std=c99 -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -D_REENTRANT -lm
In the Linker field, add:
-l"mingw32"
-l"SDL2main"
-l"SDL2.dll"
-l"SDL2_image.dll"
-l"SDL2_ttf.dll"

Go to: Project->Project Options...->Directories
Select the tab: Library Directories
Add: C:\Program Files (x86)\Dev-Cpp\MinGW64\i686-w64-mingw32\lib
Select the tab: Include Directories
Add: C:\Program Files (x86)\Dev-Cpp\MinGW64\i686-w64-mingw32\include\SDL2

Go to: Tools->Compiler Options...->Directories
Select the tab: Binaries
Add: C:\Program Files (x86)\Dev-Cpp\MinGW64\i686-w64-mingw32\bin
Select the tab: Libraries
Add: C:\Program Files (x86)\Dev-Cpp\MinGW64\i686-w64-mingw32\lib

Go to: Project->Project Options...->General
Browse and add: png/savof_icon.ico

Select: Execute->Compile (or press F9).
