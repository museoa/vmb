datadir = ${prefix}/share/${PACKAGE_NAME}
includedir = ${prefix}/include/${PACKAGE_NAME}
pkgdatadir = ${prefix}/lib/${PACKAGE_NAME}
sysconfdir = /etc

CLEANFILES = exec coreboot tapeboot *.o *.rom.umps 

sysconf_DATA = umpsrc
data_DATA = exec.rom.umps coreboot.rom.umps tapeboot.rom.umps @LDCORESCRIPT@ @LDAOUTSCRIPT@
pkgdata_DATA = libumps.o crtso.o crti.o
include_HEADERS = e/libumps.e h/types.h

all: exec.rom.umps coreboot.rom.umps tapeboot.rom.umps libumps.o crtso.o crti.template.o crti.o

# generic aout startup function 

.s.o:
	$(CCAS) $(CCASFLAGS) $< -o $@
	

exec.rom.umps: exec
	./umps-elf2umps -v -b $<

exec: exec.s
	$(CCAS) $(CCASFLAGS) $< -o $@

coreboot: coreboot.s
	$(CCAS) $(CCASFLAGS) $< -o $@

tapeboot: tapeboot.s
	$(CCAS) $(CCASFLAGS) $< -o $@
	
# tapeboot target
tapeboot.rom.umps: tapeboot
	./umps-elf2umps -v -b $<
	
# coreboot target
coreboot.rom.umps: coreboot
	./umps-elf2umps -v -b $<
