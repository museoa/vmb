bin_PROGRAMS = umps umps-elf2umps umps-objdump

AM_CPPFLAGS = @UMPSCPPFLAG@

umps_SOURCES = \
	omain.cc systembus.cc \
	processor.cc \
	setup.cc disassemble.cc \
	watch.cc xinterface.cc appforms.cc \
	utility.cc main.c \
	../../bus-util.c ../../message.c \
	../../bus-arith.c ../../error.c defaults.c \
	cache.cc

umps_elf2umps_SOURCES = \
	elf2umps.cc h/const.h h/types.h \
	h/blockdev.h h/processor.h h/aout.h

umps_objdump_SOURCES = \
	h/const.h h/types.h e/disassemble.e \
	h/blockdev.h h/aout.h disassemble.cc objdump.cc 
