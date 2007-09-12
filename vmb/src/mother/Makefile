PROG= mother
UTILDIR = ../util
CFLAGS= -g -Wall
CC = gcc


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) -I $(UTILDIR) $(PROG).c  -L $(UTILDIR) -lvmb  -o $(PROG)

install: all
	install $(PROG) $(BINDIR)


TAGS:
	etags *.c *.h ../util/*.c ../util/*.h

clean:
	rm -f $(PROG) *.o *~

