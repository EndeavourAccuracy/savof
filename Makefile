# SPDX-License-Identifier: GPL-3.0-or-later
# Makefile for savof 1.0 by Norbert de Jonge
#
# cppcheck --language=c --std=c99 --verbose savof.c
# Also try the line below with clang instead of gcc.
#
all:
	gcc -O2 -Wno-unused-result -std=c99 -g -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes savof.c -o savof `sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image -lm -Wno-format-truncation
