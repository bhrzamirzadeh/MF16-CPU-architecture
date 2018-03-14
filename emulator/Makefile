#
# makefile for MF16 emulator
#

CC     = cc

CFLAGS = -std=c99 -Wall -Wextra -g

mf16emu: mf16emulib.o mf16emu.o
	$(CC) $(CFLAGS) -o mf16emu mf16emulib.o mf16emu.o

mf16emu.o: mf16emulib.c mf16emulib.h mf16emu.c
	$(CC) $(CFLAGS) -c mf16emu.c

mf16emulib.o: mf16emulib.c mf16emulib.h
	$(CC) $(CFLAGS) -c mf16emulib.c
	
clean:
	rm -f *.o mf16emu
