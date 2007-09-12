
CFLAGS= -g -Wall

SUBDIRS  = util mother mmix screen keyboard ram rom flash disk hostdisk vram
     

.PHONY: hardware $(SUBDIRS) clean
     
hardware: $(GENERICS) $(SUBDIRS)
     
$(SUBDIRS):
	$(MAKE) -C $@


install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done



     
clean:
	rm -f *.o *~ 
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

TAGS:
	etags *.c *.h */*.c */*.h
