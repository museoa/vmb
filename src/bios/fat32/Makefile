# making the rom bios program cross-compiling on the host with target = mmix

# change this to the directory where mmix-gcc is installed
#BIN=/c/Program\ Files/mmix/bin/
#BIN=/opt/mmix/bin/
# or leave empty if mmix-gcc and friends is on your search path
BIN=

AS=$(BIN)mmix-as
LD=$(BIN)mmix-ld
CC=$(BIN)mmix-gcc
OCP=$(BIN)mmix-objcopy

#CFLAGS= -ggdb  -O0 -Wall -melf -mabi=mmixware -mno-base-addresses 
CFLAGS=         -O3 -Wall -melf -mabi=mmixware -mno-base-addresses
#LDFLAGS=--entry=_start.
LDFLAGS=  -static \
          --section-start .text=0x8000000000000000 \
          --section-start .bss=0x8000000100000000
# add -t and -M to the loadflags to get more output

ASFLAGS= -gdwarf2 -x 

# some more linker optione
# -lc link in libc
# -static do not link to dynamic libraries
# -nostdlib do not link in standard libraries

OBJECTS= bios.o tables.o dtraps.o ftraps.o terminal.o diskio.o
LIBS=libfat32/libfat32.a
SUBDIRS = libfat32
LFLAGS= -L libfat32 -lfat32  -lc

all:  bios.img bios.elf

pagetab.s: mkpagetab.sh
	mkpagetab.sh > pagetab.s

bios.o: bios.s pagetab.s
	$(AS) $(ASFLAGS) bios.s -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $^ -o $@


tables.o: tables.s
	$(AS) $(ASFLAGS) -no-expand $^ -o $@



%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@


bios.img: bios.elf
	$(OCP) -O binary bios.elf bios.img

bios.elf: $(OBJECTS) $(LIBS)
	$(LD) $(LDFLAGS) --oformat elf64-mmix $(OBJECTS) $(LFLAGS)  -o $@
	$(OCP) -O binary bios.elf bios.img



.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@


$(LIBS): $(SUBDIRS)

clean:
	rm -f *.o *~ *.img *.elf
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done





