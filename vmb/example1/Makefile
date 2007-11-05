#   produce an object file from the assembler file
BIN=/home/ruckert/mmixmb/bitrange/install/bin
AFLAGS= -gdwarf2
AS=$(BIN)/mmix-as
LD=$(BIN)/mmix-ld
CC=$(BIN)/mmix-gcc
CFLAGS= -mabi=mmixware

LDFLAGS=--section-start .priv=0x8000000000000000

all: hello.mmo hello.elf

%.o: %.s
	$(AS) -gdwarf2 --linker-allocated-gregs $^ -o $@

%.mmo:%.c
	$(CC) $(CFLAGS) $< -o $@

%.elf:%.c
	$(CC) $(CFLAGS) -melf $< -o $@


%.s:%.c
	$(CC) -S $^

%.mmo: %.o
	$(LD) $^ -o $@


%.rom: %.o
	$(LD) $(LDFLAGS) --oformat binary $^ -o $@

%.elf: %.o
	$(LD) $(LDFLAGS) --oformat elf64-mmix $^ -o $@


show: hello.elf 
	$(BIN)/mmix-readelf -a hello.elf

clean:
	rm -rf *.mmo *.o *.elf *~ hello.s
