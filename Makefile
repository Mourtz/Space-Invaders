# C compiler
CC=gcc
# C flags
CFLAGS=-lncurses
# ASM compiler
CC2=nasm
# ASM flags
CFLAGS2=-f elf64

#Executable name
exec=-o space_invaders

build:
	$(CC2) $(CFLAGS2) -o main.o main.asm
	$(CC) main.o main.c $(exec) $(CFLAGS)
