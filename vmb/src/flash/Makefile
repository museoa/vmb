PROG= flash
UTILDIR = ../util
CFLAGS= -g -Wall
CC = gcc


all: $(PROG)

utilities:
	$(MAKE) -C  $(UTILDIR)


$(PROG): $(PROG).c  utilities
	$(CC) $(CFLAGS) -I $(UTILDIR) $(PROG).c  -L $(UTILDIR) -lvmb  -o $(PROG)
install:
	install $(PROG) $(BINDIR)

clean:
	rm -f $(PROG) *.o *~

