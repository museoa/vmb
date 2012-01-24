
SUBDIRS  = src doc util


.PHONY: $(SUBDIRS) clean install

$(SUBDIRS):
	$(MAKE) -C $@


export BINDIR = $(CURDIR)/bin

install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done

source:
	tar -cvzf src_1_2.tgz src/*/Makefile src/*/*.ico src/*/*.bmp src/*/*.c src/*/*.h src/*/*.vcproj src/*/*.sln src/*/*.rc


www: wwwdoc
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
	cp src/VMBLogo/vmb128.jpg src/VMBLogo/vmb200.jpg src/VMBLogo/vmb64.gif htdocs

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
