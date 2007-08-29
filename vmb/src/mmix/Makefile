#
#   Makefile for MMIXware
#

#   Be sure that CWEB version 3.0 or greater is installed before proceeding!
#   In fact, CWEB 3.61 is recommended for making hardcopy or PDF documentation.

#   If you prefer optimization to debugging, change -g to something like -O:

MMIXDIR=/usr/local/bin
AS=$(MMIXDIR)/mmix-as
LD=$(MMIXDIR)/mmix-ld
CFLAGS = -ggdb -Wall -I..
PROG= mmix
UTILDIR = ../util
CFLAGS= -g -Wall
CC = gcc
INCDIRS = -I $(UTILDIR)
LIBS    = -L $(UTILDIR) -lm -lvmb


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)

OBJECTS=mmix-arith.o mmix-io.o mmix-sim.o address.o cache.o gdb.o remote-utils.o breaks.o

abstime.h: abstime
	./abstime > abstime.h

$(PROG):  utilities  abstime.h $(OBJECTS)
	$(CC) $(CFLAGS) $(INCDIRS)  $(OBJECTS) $(LIBS)  -o $(PROG)
	rm abstime.h




#   Uncomment the second line if you use pdftex to bypass .dvi files:
PDFTEX = dvipdfm
#PDFTEX = pdftex

.SUFFIXES: .dvi .tex .w .ps .pdf

.tex.dvi:
	tex $*.tex

.dvi.ps:
	dvips $* -o $*.ps

%.c: %.w %.ch
	if test -r $*.ch; then ctangle $*.w $*.ch; else ctangle $*.w; fi

%.tex: %.w %.ch
	if test -r $*.ch; then cweave $*.w $*.ch; else cweave $*.w; fi


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


clean:
	rm -f *~ *.o *.tex *.log *.dvi *.toc *.idx *.scn *.ps core $(PROG)


mmixal: mmix-arith.o mmixal.c
	$(CC) $(CFLAGS) mmixal.c mmix-arith.o -o mmixal


