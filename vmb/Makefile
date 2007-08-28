
SUBDIRS  = src doc
     

.PHONY: $(SUBDIRS) clean
     
$(SUBDIRS):
	$(MAKE) -C $@
     
clean:
	rm -f *.o *~ 
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

