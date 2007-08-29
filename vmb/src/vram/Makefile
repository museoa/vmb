PROG= vram
UTILDIR = ../util
CFLAGS= -g -Wall
CC = gcc
INCDIRS = -I /usr/X11R6/include -I $(UTILDIR)
LIBS    = -L/usr/X11R6/lib -lX11 -lpthread -L $(UTILDIR) -lvmb


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) $(INCDIRS) $(PROG).c  $(LIBS)  -o $(PROG)

clean:
	rm -f $(PROG) *.o *~



