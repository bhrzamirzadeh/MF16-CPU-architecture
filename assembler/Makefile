#
# makefile for MF16 assembler (mf16asm)
#

CC     = cc
CFLAGS = -std=c99 -Wall -Wextra -g

mf16asm: mf16asmlib.o mf16asm.o
	$(CC) $(CFLAGS) -o mf16asm mf16asmlib.o mf16asm.o

mf16asm.o: mf16asmlib.c mf16asmlib.h mf16asm.c
	$(CC) $(CFLAGS) -c mf16asm.c

mf16asmlib.o: mf16asmlib.c mf16asmlib.h
	$(CC) $(CFLAGS) -c mf16asmlib.c


clean:
	rm -f *.o mf16asm
