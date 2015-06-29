# C compiler
CC=gcc
# C flags
CFLAGS=-Wall -lncurses
# ASM compiler
CC2=asm
# ASM flags
CFLAGS2=-f elf64

#Executable name
exec=-o space_invaders

build:
	#nasm -f elf64 -o main.o main.asm
	#gcc main.o main.c -o main
	$(CC) main.c $(exec) $(CFLAGS)
