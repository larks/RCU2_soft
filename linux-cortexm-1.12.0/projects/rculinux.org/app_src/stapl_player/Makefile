#
#	Module:			makefile
#
#					Copyright (C) Altera Corporation 1997-1999
#
#	Description:	Makefile for JAM Interpreter
#
#
#	Actel version 1.1             May 2003
#

# Compiler flags
CFLAGS		:= -Os -mcpu=cortex-m3 -mthumb
LDFLAGS		:= -mcpu=cortex-m3 -mthumb
CC			= $(CROSS_COMPILE_APPS)gcc

EXE = jam

OBJS = \
	jamstub.o \
	jamexec.o \
	jamnote.o \
	jamcrc.o \
	jamsym.o \
	jamstack.o \
	jamheap.o \
	jamarray.o \
	jamcomp.o \
	fs2jtag.o \
	jamutil.o \
	jamexp.o \
	jamgpio.o

#.c.o :
#	cl /W4 /c /ML /DWINNT /DFS2 $<

# LINK: add appropriate linker command here

$(EXE) : $(OBJS)
	$(CC) $(LDFLAGS) -o jam $(OBJS)

# Dependencies:

jamstub.o : \
	jamstub.c \
	jamport.h \
	jamexprt.h \
	jamgpio.c \
	jamgpio.h
	$(CC) -c jamstub.c $(CFLAGS)

jamexec.o : \
	jamexec.c \
	jamport.h \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamutil.h \
	jamexp.h \
	jamsym.h \
	jamstack.h \
	jamheap.h \
	jamarray.h \
	jamjtag.h
	$(CC) -c jamexec.c $(CFLAGS)

jamnote.o : \
	jamnote.c \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamutil.h
	$(CC) -c jamnote.c $(CFLAGS)

jamcrc.o : \
	jamcrc.c \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamutil.h
	$(CC) -c jamcrc.c $(CFLAGS)

jamsym.o : \
	jamsym.c \
	jamexprt.h \
	jamdefs.h \
	jamsym.h \
	jamheap.h \
	jamutil.h
	$(CC) -c jamsym.c $(CFLAGS)

jamstack.o : \
	jamstack.c \
	jamexprt.h \
	jamdefs.h \
	jamutil.h \
	jamsym.h \
	jamstack.h
	$(CC) -c jamstack.c $(CFLAGS)

jamheap.o : \
	jamheap.c \
	jamport.h \
	jamexprt.h \
	jamdefs.h \
	jamsym.h \
	jamstack.h \
	jamheap.h \
	jamutil.h
	$(CC) -c jamheap.c $(CFLAGS)

jamarray.o : \
	jamarray.c \
	jamexprt.h \
	jamdefs.h \
	jamexec.h \
	jamexp.h \
	jamsym.h \
	jamstack.h \
	jamheap.h \
	jamutil.h \
	jamcomp.h \
	jamarray.h
	$(CC) -c jamarray.c $(CFLAGS)

jamcomp.o : \
	jamcomp.c \
	jamdefs.h \
	jamcomp.h
	$(CC) -c jamcomp.c $(CFLAGS)

fs2jtag.o : \
	fs2jtag.c \
	jamexprt.h \
	jamdefs.h \
	jamsym.h \
	jamutil.h \
	jamjtag.h
	$(CC) -c fs2jtag.c $(CFLAGS)

jamutil.o : \
	jamutil.c \
	jamutil.h
	$(CC) -c jamutil.c $(CFLAGS)

jamexp.o : \
	jamexp.c \
	jamexprt.h \
	jamdefs.h \
	jamexp.h \
	jamsym.h \
	jamheap.h \
	jamarray.h \
	jamutil.h \
	jamytab.h
	$(CC) -c jamexp.c $(CFLAGS)
	
all	: jam
clean	:
	-rm -f jam *.o *.gdb
