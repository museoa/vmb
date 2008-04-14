PROG= vram
UTILDIR = ../util
OPTDIR = ../opt
UTILDIR = ../util
CFLAGS= -g  -D_REENTRANT -Wall  -I$(UTILDIR) -I$(OPTDIR) -I/usr/X11R6/include
LDFLAGS= -L/usr/X11R6/lib -lX11 -L$(OPTDIR) -lvmbopt -L$(UTILDIR) -lvmb -lpthread
CC = gcc


all: $(PROG) TAGS

utilities:
	$(MAKE) -C  $(UTILDIR)
	$(MAKE) -C  $(OPTDIR)


$(PROG): $(PROG).c utilities
	$(CC) $(CFLAGS) $(PROG).c $(LDFLAGS) -o $(PROG)

install: all
	install $(PROG) $(BINDIR)


TAGS: *.c $(UTILDIR)/*.c $(UTILDIR)/*.h $(OPTDIR)/*.c $(OPTDIR)/*.h
	etags *.c $(UTILDIR)/*.c $(UTILDIR)/*.h

clean:
	rm -f $(PROG) *.o *~ TAGS


