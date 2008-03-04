
CFLAGS= -O3 -ggdb -D_REENTRANT -Wall -pg
CC= gcc
AR = ar
ARFLAGS = cru
RANLIB = ranlib

VMBDEFAULTS = vmbpoweron.o vmbpoweroff.o vmbreset.o vmbinterrupt.o vmbgetpayload.o vmbputpayload.o vmbunknown.o

VMBOPTOBJECTS = param.o option.o version.o
VMBOBJECTS = cache.o vmb.o message.o bus-arith.o error.o $(VMBDEFAULTS)

.PHONY:  clean
     
all: libvmb.a libvmbopt.a
     
libvmb.a: $(VMBOBJECTS)
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(VMBOBJECTS)
	$(RANLIB) $@

libvmbopt.a: $(VMBOPTOBJECTS)
	rm -f $@
	$(AR) $(ARFLAGS) $@ $(VMBOPTOBJECTS)
	$(RANLIB) $@

install:
	echo nothing to do

devicetemplate: devicetemplate.o libvmb.a
	$(CC) $(CFLAGS) $< -L . -lvmb -lpthread -o $@

clean:
	rm -f *.o *.a *~ 
