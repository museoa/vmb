#   produce an object file from the assembler file
BIN=.
AFLAGS= -gdwarf2
AS=$(BIN)/mmixal

all: lorentz.mmo bios.img

%.mmo: %.mms
	$(AS) $<

%.img: %.mmo
	mmoboot $<

clean:
	rm -rf *.mmo *.img *~

