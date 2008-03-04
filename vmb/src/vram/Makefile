PROG= vram
UTILDIR = ../util
CFLAGS= -g -D_REENTRANT -Wall
CC = gcc
INCDIRS = -I /usr/X11R6/include -I $(UTILDIR)
LIBS    = -L/usr/X11R6/lib -lX11 -lpthread -L $(UTILDIR) -lvmb  -lvmbopt -lpthread


all: $(PROG) TAGS

utilities:
	$(MAKE) -C  $(UTILDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) $(INCDIRS) $(PROG).c  $(LIBS)  -o $(PROG)

install:
	install $(PROG) $(BINDIR)


clean:
	rm -f $(PROG) *.o *~



TAGS: *.c  $(UTILDIR)/*.c $(UTILDIR)/*.h
	etags *.c  $(UTILDIR)/*.c $(UTILDIR)/*.h
