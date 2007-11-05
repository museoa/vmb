
CFLAGS= -O3 -ggdb -Wall -pg
CC= gcc
AR = ar
ARFLAGS = cru
RANLIB = ranlib


OBJECTS = main.o bus-util.o message.o  bus-arith.o param.o option.o error.o version.o

.PHONY:  clean
     
all: libvmb.a
     
libvmb.a: $(OBJECTS)
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
	$(RANLIB) $@

install:
	echo nothing to do

clean:
	rm -f *.o *~ 
