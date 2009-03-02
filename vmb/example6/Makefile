#   produce an object file from the assembler file
BIN=/home/ruckert/mmixmb/bitrange/install/bin
AFLAGS= -gdwarf2
AS=$(BIN)/mmixal
MAS=$(BIN)/mmix-as
MLD=$(BIN)/mmix-ld
MCC=$(BIN)/mmix-gcc
MLDFLAGS= -t -static 
MCFLAGS= -ggdb -Wall -mabi=mmixware -mno-base-addresses -O2 

all: hello.elf

%.mmo: %.mms
	$(AS) $<

%.img: %.mmo
	mmoboot $<

%.o: %.s
	$(MAS) -gdwarf2 -x $^ -o $@



%.o: %.c
	$(MCC) $(MCFLAGS) -c $^ -o $@

%.elf: %.c
	$(MCC) $(MCFLAGS) -melf $<  -o $@


%.elf: %.o
	$(MLD) $(MLDFLAGS) --oformat elf64-mmix $<  -o $@

%.mmo: %.o
	$(MLD) $(MLDFLAGS) --oformat mmo $<  -o $@



clean:
	rm -rf *.mmo *.img *~


mount:
	mount -o loop  disk.bin mnt

mkdisk:
	mkdosfs -C -F 32 disk.bin 128