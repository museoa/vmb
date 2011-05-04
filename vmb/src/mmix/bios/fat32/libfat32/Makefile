
BIN=/opt/mmix/bin
CC=$(BIN)/mmix-gcc
AR=$(BIN)/mmix-ar
RANLIB = $(BIN)/mmix-ranlib

CFLAGS= -ggdb -Wall -melf -mabi=mmixware -mno-base-addresses -O1
ARFLAGS = cru


OBJECTS = FAT32_Cache.o FAT32_Access.o  FAT32_FileLib.o \
          FAT32_Table.o \
          FAT32_Disk.o  FAT32_Name.o \


all: .depend libfat32.a TAGS


libfat32.a: $(OBJECTS)
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	$(RANLIB) $@

install:
	echo nothing to do

TAGS: *.c *.h
	etags *.c *.h

clean:
	rm -f *.o *.a *.elf *~ TAGS .depend


.depend:
	gcc -MM -MG *.c > .depend 

include .depend