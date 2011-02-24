
SUBDIRS  = src doc
     

.PHONY: $(SUBDIRS) clean install
     
$(SUBDIRS):
	$(MAKE) -C $@


export BINDIR = $(CURDIR)/bin

install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done

www: htdocs
	rsync -aivzC htdocs/ ruckert,vmb@web.sourceforge.net:htdocs/


helpfiles := $(wildcard src/*/help.html)
helpdirs := $(patsubst src/%/help.html,%,$(helpfiles))

htdocs:
	for f in $(helpdirs); do \
	  echo copying src/$$f/help.html to htdocs/"$$f"_help.html; \
	  cp src/$$f/help.html htdocs/"$$f"_help.html ; \
	done

     
clean:
	rm -f *.o *~ 
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

ssh:
	ssh ruckert,vmb@shell.sourceforge.net



TAGS:
	etags */*.c */*.h */*.w

.PHONY : www
.PHONY : ssh
.PHONY : clean
.PHONY : htdocs