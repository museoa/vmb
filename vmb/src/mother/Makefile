PROG= mother
UTILDIR = ../util
OPTDIR = ../opt
INSTALLDIR=../bin

#CFLAGS= -g  -D_REENTRANT -Wall  -I$(UTILDIR) -I$(OPTDIR)
CFLAGS= -O3  -D_REENTRANT -Wall  -I$(UTILDIR) -I$(OPTDIR)
LDFLAGS= -L$(OPTDIR) -lvmbopt -L$(UTILDIR) -lvmb -lpthread
CC = gcc


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)
	$(MAKE) -C  $(OPTDIR)


$(PROG): $(PROG).c optiontab.c utilities
	$(CC) $(CFLAGS) $(PROG).c optiontab.c $(LDFLAGS) -o $(PROG)

install: all
	install $(PROG) $(INSTALLDIR)


TAGS:
	etags *.c *.h $(UTILDIR)/*.c $(UTILDIR)/*.h $(OPTDIR)/*.c $(OPTDIR)/*.h

clean:
	rm -f $(PROG) *.o *~

