#   produce an object file from the assembler file

all: hello.mmo bios.img

%.mmo: %.mms
	mmixal $<

%.img: %.mmo
	mmoimg $<

clean:
	rm -rf *.mmo *.img *~

