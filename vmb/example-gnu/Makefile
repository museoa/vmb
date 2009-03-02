#   produce an object file from the assembler file
BIN=install/bin
MAS=$(BIN)/mmix-as
MLD=$(BIN)/mmix-ld
MCC=$(BIN)/mmix-gcc
MCFLAGS= -g -Wall
MLDFLAGS=--entry=_start.
#MLDFLAGS=--section-start .text=0x8000000000000000 


all: hello.elf name.elf


# to make mmo or elf files one can use the gnu binutils

%.o: %.s
	$(MAS) -g --linker-allocated-gregs $^ -o $@


%.elf: %.o
	$(MLD) -g --oformat elf64-mmix $^ -o $@


%.o: %.c
        $(MCC) -g -melf -mabi=mmixware -c $^ -o $@
        

%.elf: %.c
	$(MCC) -g -melf -mabi=mmixware  $^ -o $@

%.mmo: %.c
	$(MCC) -g -mabi=mmixware  $^ -o $@


BIOSFLAGS=  -t -T bios.x --section-start .BIOS=0x8000000000000000 --entry=0x8000000000000000

bios.elf: bios.o
	$(MLD) $(BIOSFLAGS) --oformat elf64-mmix bios.o -o bios.elf

bios.img: bios.o
	$(MLD)  $(BIOSFLAGS) --oformat binary $^ -o $@

devices:
	cp ../src/disk/disk disk
	cp ../src/ram/ram ram 
	cp ../src/rom/rom rom
	cp ../src/mmixgdb/mmix mmix 
	cp ../src/screen/screen screen 
	cp ../src/keyboard/keyboard keyboard
	cp ../src/mother/mother mother

clean:
	rm -f *.o  *~
