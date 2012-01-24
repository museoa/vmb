PROG= hostdisk
UTILDIR = ../util
OPTDIR = ../opt
INSTALLDIR=../bin

CFLAGS= -g  -D_REENTRANT -Wall  -I$(UTILDIR) -I$(OPTDIR)
LDFLAGS= -L$(OPTDIR) -lvmbopt -L$(UTILDIR) -lvmb -lpthread 
CC = gcc

all: $(PROG) TAGS

utilities:
	$(MAKE) -C  $(UTILDIR)
	$(MAKE) -C  $(OPTDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) $(PROG).c $(LDFLAGS) -o $(PROG)


TAGS: *.c *.h $(UTILDIR)/*.c $(UTILDIR)/*.h $(OPTDIR)/*.c $(OPTDIR)/*.h
	etags *.c *.h $(UTILDIR)/*.c $(UTILDIR)/*.h

install: $(PROG)
	install $(PROG) $(INSTALLDIR)

clean:
	rm -f $(PROG) *.o *~


