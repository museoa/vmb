

PROGS = talk serial utilities options
UTILDIR = ../util
OPTDIR = ../opt
INSTALLDIR=../bin

CFLAGS= -ggdb  -D_REENTRANT -Wall  -I$(UTILDIR) -I$(OPTDIR)
LDFLAGS= -L$(OPTDIR) -lvmbopt -L$(UTILDIR) -lvmb -lpthread
CC=gcc

all: $(PROGS)

serial: serial.o optiontab.o utilities options
	$(CC) $(CFLAGS)  serial.o optiontab.o  $(LDFLAGS) -o $@

utilities:
	$(MAKE) -C  $(UTILDIR)

options:
	$(MAKE) -C  $(OPTDIR)

install: serial
	install serial $(INSTALLDIR)


clean:
	rm -f *.o *~ $(PROGS)