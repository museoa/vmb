PROG= ram
UTILDIR = ../util
OPTDIR = ../opt
UTILDIR = ../util
CFLAGS= -g  -D_REENTRANT -Wall  -I$(UTILDIR) -I$(OPTDIR)
LDFLAGS= -L$(OPTDIR) -lvmbopt -L$(UTILDIR) -lvmb -lpthread
CC = gcc


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)
	$(MAKE) -C  $(OPTDIR)


$(PROG): $(PROG).c  main.c utilities
	$(CC) $(CFLAGS) $(PROG).c main.c $(LDFLAGS) -o $(PROG)

install: all
	install $(PROG) $(BINDIR)

clean:
	rm -f $(PROG) *.o *~

