
CFLAGS= -g -Wall

SUBDIRS  = util device mother\
           mmixcpu \
	   mmixgdb \
           screen keyboard vram \
           ram rom flash \
           disk hostdisk led timer button serial

.PHONY: hardware $(SUBDIRS) clean

hardware: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@


install:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir install; \
	done


source.tgz:
	(cd /tmp; rm -rf vmb; cvs -d :ext:ruckert@vmb.cvs.sourceforge.net:/cvsroot/vmb co -P vmb/src)
	tar -czvf source.tgz -C /tmp --exclude-vcs  vmb/src
	rm -rf /tmp/vmb



clean:
	rm -f *.o *~ TAGS
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done

TAGS:
	etags  */*.c */*.h mmix/*.w mmix/*.ch
