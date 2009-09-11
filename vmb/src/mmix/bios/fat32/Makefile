# making the rom bios program cross-compiling on the host with target = mmix

BIN=/usr/local/bin
MAS=$(BIN)/mmix-as
MLD=$(BIN)/mmix-ld
MCC=$(BIN)/mmix-gcc
MCFLAGS= -ggdb -Wall -melf -mabi=mmixware -mno-base-addresses -O2 
#MLDFLAGS=--entry=_start.
MLDFLAGS=  -static \
          --section-start .text=0x8000000000000000 \
          --section-start .bss=0x8000000100000000
# add -t and -M to the loadflags to get more output

MASFLAGS= -gdwarf2 -x 

# some more linker optione
# -lc link in libc
# -static do not link to dynamic libraries
# -nostdlib do not link in standard libraries

OBJECTS= bios.o dtraps.o ftraps.o terminal.o diskio.o
LIBS=libfat32/libfat32.a
SUBDIRS = libfat32

all:  bios.img bios.elf


%.o: %.s
	$(MAS) $(MASFLAGS) -no-expand $^ -o $@


tables.o: tables.s
	$(MAS) $(MASFLAGS) -no-expand $^ -o $@



%.o: %.c
	$(MCC) $(MCFLAGS) -c $^ -o $@


bios.img: $(OBJECTS) $(LIBS)
	$(MLD)  $(MLDFLAGS) --oformat binary $(OBJECTS)    -L libfat32 -lfat32  -lc -o $@



bios.elf: $(OBJECTS) $(LIBS)
	$(MLD) $(MLDFLAGS) --oformat elf64-mmix $(OBJECTS) -L libfat32 -lfat32 -lc  -o $@



.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@


$(LIBS): $(SUBDIRS)

clean:
	rm -f *.o *~ *.img *.elf
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done





