PROG= mother
UTILDIR = ../util
OPTDIR = ../opt
CFLAGS= -g  -D_REENTRANT -Wall  -I $(UTILDIR) -I $(OPTDIR)
LDFLAGS= -L $(OPTDIR) -lvmbopt -L $(UTILDIR) -lvmb -lpthread
CC = gcc


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)
	$(MAKE) -C  $(OPTDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) $(PROG).c $(LDFLAGS) -o $(PROG)

install: all
	install $(PROG) $(BINDIR)


TAGS:
	etags *.c *.h $(UTILDIR)/*.c $(UTILDIR)/*.h $(OPTDIR)/*.c $(OPTDIR)/*.h

clean:
	rm -f $(PROG) *.o *~

