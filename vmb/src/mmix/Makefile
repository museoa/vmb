#
#   Makefile for MMIXware
#

#   Be sure that CWEB version 3.0 or greater is installed before proceeding!
#   In fact, CWEB 3.61 is recommended for making hardcopy or PDF documentation.

#   If you prefer optimization to debugging, change -g to something like -O:

MMIXDIR=/usr/local/bin
AS=$(MMIXDIR)/mmix-as
LD=$(MMIXDIR)/mmix-ld
#CFLAGS = -ggdb -O0 -D_REENTRANT -Wall 
CFLAGS = -O3 -D_REENTRANT -Wall 

UTILDIR = ../util
OPTDIR = ../opt
INSTALLDIR=../bin

CC = gcc
INCDIRS = -I $(UTILDIR) -I $(OPTDIR)
LIBS    = -L $(UTILDIR) -lm -lvmb -lpthread -lrt


all: .depend mmixcpu mmixgdb mmixal mmoimg mmotype

.depend:
	gcc $(INCDIRS) -MM *.c > .depend

utilities:
	$(MAKE) -C  $(UTILDIR)

VMBOBJECTS=mmix-arith.o address.o mmix-bus.o breaks.o 
GDBOBJECTS=gdb.o remote-utils.o buffers.o

abstime.h: abstime
	./abstime > abstime.h

mmixcpu:  mmix-vmb.c abstime utilities  $(VMBOBJECTS)
	./abstime > abstime.h
	$(CC) $(CFLAGS) $(INCDIRS) mmix-vmb.c $(VMBOBJECTS) $(LIBS)  -o mmixcpu
	rm abstime.h

mmixgdb:  mmix-gdb.c abstime utilities $(VMBOBJECTS) $(GDBOBJECTS)
	./abstime > abstime.h
	$(CC) $(CFLAGS) $(INCDIRS) mmix-gdb.c $(VMBOBJECTS) $(GDBOBJECTS) $(LIBS)  -o mmixgdb
	rm abstime.h


#   Uncomment the second line if you use pdftex to bypass .dvi files:
PDFTEX = dvipdfm
#PDFTEX = pdftex

.SUFFIXES: .dvi .tex .w .ps .pdf

.tex.dvi:
	tex $*.tex

.dvi.ps:
	dvips $* -o $*.ps

mmix-vmb.c: mmix-sim.w vmb.ch
	ctangle mmix-sim.w vmb.ch mmix-vmb.c

mmix-gdb.c: mmix-sim.w vmb.ch gdb.ch
	ctie -c vmb-gdb.ch mmix-sim.w vmb.ch gdb.ch
	ctangle mmix-sim.w vmb-gdb.ch mmix-gdb.c


%.tex: %.w %.ch
	if test -r $*.ch; then cweave $*.w $*.ch; else cweave $*.w; fi

%.c: %.w %.ch
	if test -r $*.ch; then ctangle $*.w $*.ch; else ctangle $*.w; fi


%.o: %.c
	$(CC) $(CFLAGS) $(INCDIRS) -c $< -o $@


.w.dvi:
	make $*.tex
	make $*.dvi

.w.ps:
	make $*.dvi
	make $*.ps

.w.pdf:
	make $*.tex
	case "$(PDFTEX)" in \
	 dvipdfm ) tex "\let\pdf+ \input $*"; dvipdfm $* ;; \
	 pdftex ) pdftex $* ;; \
	esac

%.o: %.mms
	$(AS) -gdwarf2 --linker-allocated-gregs $^ -o $@

%.elf: %.o
	$(LD)  --oformat elf64-mmix $^ -o $@

%.mmo: %.o
	$(LD) --oformat mmo $^ -o $@






GDBFILES = remote-utils.o gdb.o

WEBFILES = abstime.w boilerplate.w mmix-arith.w mmix-config.w mmix-doc.w \
	mmix-mem.w mmix-pipe.w mmix-sim.w mmixal.w mmmix.w mmotype.w
CHANGEFILES =
TESTFILES = *.mms silly.run silly.out *.mmconfig *.mmix
MISCFILES = Makefile makefile.dos README mmix.mp mmix.1
ALL = $(WEBFILES) $(TESTFILES) $(MISCFILES)


doc:    mmix-doc.ps mmixal.dvi mmix-sim.dvi
	dvips -n13 mmixal.dvi -o mmixal-intro.ps
	dvips -n8 mmix-sim.dvi -o mmix-sim-intro.ps

install: all
	install mmixcpu mmixgdb mmoimg $(INSTALLDIR)


clean:
	rm -f *~ *.o *.tex *.log *.dvi *.toc *.idx *.scn *.ncb *.ps core 
	rm -f mmix-arith.c mmix-sim.c mmixal.c mmoimg.c mmotype.c 
	rm -f mmix-vmb.w mmix-gdb.w mmix-vmb.c mmix-gdb.c vmb-gdb.ch
	rm -f abstime mmixcpu mmixgdb mmixal mmotype mmoimg

mmixal: mmix-arith.o mmixal.c
	$(CC) $(CFLAGS) mmixal.c mmix-arith.o -o mmixal


mmoimg.c: mmotype.w mmoimg.ch
	ctangle mmotype.w mmoimg.ch mmoimg.c

mmoimg.tex: mmotype.w mmoimg.ch
	cweave mmotype.w mmoimg.ch mmoimg.tex

TAGS:
	etags *.c *.h ../util/*.c ../util/*.h


include .depend