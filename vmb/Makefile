
SUBDIRS  = src doc util


.PHONY: $(SUBDIRS) clean install

$(SUBDIRS):
	$(MAKE) -C $@


export BINDIR = $(CURDIR)/bin

install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done



www:
	rm -f htdocs/*~
	rsync -aivz --exclude CVS --exclude .cvsignore htdocs/ ruckert,vmb@web.sourceforge.net:htdocs/


helpfiles := $(wildcard src/*/help.html)
helpdirs := $(patsubst src/%/help.html,%,$(helpfiles))

wwwdoc: util $(helpfiles)
	for dir in $(helpdirs); do \
	  cp src/$$dir/help.html htdocs/"$$dir"_help.html ; \
	  for file in `util/imglist src/$$dir/ < src/$$dir/help.html`; do\
	    cp $$file htdocs;  \
	  done; \
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
