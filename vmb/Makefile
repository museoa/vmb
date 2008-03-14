
SUBDIRS  = src doc
     

.PHONY: $(SUBDIRS) clean install
     
$(SUBDIRS):
	$(MAKE) -C $@


export BINDIR = $(CURDIR)/bin

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
	etags */*.c */*.h */*.w