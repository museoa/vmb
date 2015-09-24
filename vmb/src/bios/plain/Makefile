
all: bios.img

# making the rom bios program cross-compiling on the host with target = mmix

BIN=../..
MAS=$(BIN)/mmixal
MLD=$(BIN)/mmoboot

%.mmo: %.mms
	$(MAS) $<

%.img: %.mmo
	$(MLD) $< 

clean:
	rm -f *.mmo *~ *.img 





