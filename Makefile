# Makefile for savof 0.9 by Norbert de Jonge
#
# cppcheck --language=c --std=c99 --verbose savof.c
# Also try the line below with clang instead of gcc.
#
all:
	gcc -O2 -Wno-unused-result -std=c99 -g -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes savof.c -o savof `sdl2-config --cflags --libs` -I/usr/include/SDL2 -D_REENTRANT -lSDL2_ttf -lSDL2_image -lm
