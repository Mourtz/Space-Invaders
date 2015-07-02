# C compiler
CC=gcc-4.9
# C flags
CFLAGS=-lncursesw -lpthread -std=c11 -Wall
#Emscripten Compiler
EMCC=emcc
# ASM compiler
CC2=nasm
# ASM flags
CFLAGS2=-f elf64

#Executable name
exec=-o space_invaders
exec2=-o space_invaders.html

build:
	$(CC2) $(CFLAGS2) -o main.o main.asm
	$(CC) main.o main.c $(exec) $(CFLAGS)
	# $(CC) main.c $(exec) $(CFLAGS)

# not functional
emsc:
	$(CC2) $(CFLAGS2) -o main.o main.asm
	$(EMCC) main.o main.c $(exec2) $(CFLAGS)
