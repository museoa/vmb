
CFLAGS= -g -Wall

SUBDIRS  = util opt device mother\
           mmix mmixgdb \
           screen keyboard vram \
           ram rom flash \
           disk hostdisk   

.PHONY: hardware $(SUBDIRS) clean
     
hardware: $(GENERICS) $(SUBDIRS)
     
$(SUBDIRS):
	$(MAKE) -C $@


install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done



     
clean:
	rm -f *.o *~ TAGS
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

TAGS:
	etags  */*.c */*.h mmix/*.w mmix/*.ch
