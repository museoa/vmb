#   produce an object file from the assembler file

all: hello.mmo bios.img

%.o: %.mms
	mmix-as hello.mms -o hello.o
	
%.mmo: %.o
	mmix-ld --oformat mmo hello.o -o hello.mmo

%.mmo: %.mms
	mmixal $<

%.img: %.mmo
	mmoimg $<

clean:
	rm -rf *.mmo *.img *~

