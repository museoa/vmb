
CFLAGS= -ggdb -Wall -melf -mabi=mmixware -mno-base-addresses -O1
CC= mmix-gcc
AR = mmix-ar
ARFLAGS = cru
RANLIB = mmix-ranlib


OBJECTS = FAT32_Access.o  FAT32_FileLib.o \
          FAT32_Table.o FAT32_Base.o \
          FAT32_FileString.o  FAT32_Write.o \
          FAT32_Disk.o    FAT32_Misc.o \


all: libfat32.a


libfat32.a: $(OBJECTS)
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	$(RANLIB) $@

install:
	echo nothing to do

clean:
	rm -f *.o *.a *~ 


.depend:
	gcc -MM -MG *.c > .depend 

include .depend