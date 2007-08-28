
CFLAGS= -g -Wall

GENERICS = bus-util.o main.o message.o  bus-arith.o param.o option.o error.o
SUBDIRS  = mmix screen mother keyboard ram rom disk mmixio
     

.PHONY: hardware $(SUBDIRS) clean
     
hardware: $(GENERICS) $(SUBDIRS)
     
$(SUBDIRS):
	$(MAKE) -C $@
     
clean:
	rm -f *.o *~ 
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

TAGS: *.c *.h */*.c */*.h
	etags *.c *.h */*.c */*.h
