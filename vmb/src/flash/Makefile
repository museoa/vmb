PROG= flash
UTILDIR = ../util
CFLAGS= -g -D_REENTRANT -Wall
CC = gcc


all: $(PROG) TAGS

utilities:
	$(MAKE) -C  $(UTILDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) -I $(UTILDIR) $(PROG).c  -L $(UTILDIR) -lvmb  -lvmbopt -lpthread  -o $(PROG)
install:
	install $(PROG) $(BINDIR)

clean:
	rm -f $(PROG) *.o *~

TAGS: *.c *.h $(UTILDIR)/*.c $(UTILDIR)/*.h
	etags *.c *.h $(UTILDIR)/*.c $(UTILDIR)/*.h
