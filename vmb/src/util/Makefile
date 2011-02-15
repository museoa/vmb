
CFLAGS= -ggdb -D_REENTRANT -Wall 
#CFLAGS= -O3 -D_REENTRANT -Wall 
CC= gcc
AR = ar
ARFLAGS = cru
RANLIB = ranlib

VMBDEFAULTS = vmbpoweron.o vmbpoweroff.o vmbreset.o vmbinterrupt.o vmbgetpayload.o vmbputpayload.o vmbunknown.o vmbdisconnected.o vmbterminate.o vmberror.o

VMBOBJECTS = cache.o vmb.o vmbtimewait.o message.o bus-arith.o error.o $(VMBDEFAULTS)

.PHONY:  clean
     
all: libvmb.a
     
libvmb.a: $(VMBOBJECTS)
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(VMBOBJECTS)
	$(RANLIB) $@

install:
	echo nothing to do

clean:
	rm -f *.o *.a *~ 
