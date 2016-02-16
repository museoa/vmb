#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(uMPS, [1.23rc2], mmorsian@users.sourceforge.net)
AC_CANONICAL_SYSTEM
AC_C_BIGENDIAN
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/systembus.cc])
AC_CONFIG_HEADER([config.h])
AC_PREFIX_DEFAULT(/usr/local)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PROG_AS
AM_PROG_LD

# Adding X11 library path to $LIBS for libforms.
AC_PATH_XTRA
LIBS="${LIBS} ${X_LIBS}" 

# Do you have external libfors or libelf?
AC_ARG_WITH([forms],[  --with-forms[=PATH]       libforms-dev pathname if not in standard location.], [FORMSLIB="${withval}"], [FORMSLIB="no"])
AC_ARG_WITH([elf],[  --with-elf[=PATH]         libelf-dev patname if not in standard location.], [ELFLIB="${withval}"], [ELFLIB="no"])

FORMSLIB=`readlink -f $FORMSLIB`
ELFLIB=`readlink -f $ELFLIB`

# If we are using external libs we need some fixing before checks.
if test "x$FORMSLIB" != "xno"
then
	UMPSCPPFLAG="-I${FORMSLIB}"
	LIBS="${LIBS} -L${FORMSLIB}"
	AC_DEFINE([EXTFORMS], [externforms], [You are using external libforms.])
fi


if test "x$ELFLIB" != "xno"
then
	UMPSCPPFLAG="${UMPSCPPFLAG} -I${ELFLIB}"
	LIBS="${LIBS} -L${ELFLIB}"
	AC_DEFINE([__LIBELF_INTERNAL_], [0], [You are using external libelf.])
fi
# enable pthreads
LIBS="${LIBS} -lpthread"

# Checks for libraries.
AC_CHECK_LIB([Xpm], [XpmReadFileToPixmap], [LIBS="${LIBS} -lXpm"],
	     AC_MSG_ERROR([*** LibXpm not found.])
	     )

AC_CHECK_LIB([X11], [XSetInputFocus], [],
	     AC_MSG_ERROR([*** Libx11 not found.])
	     )
	     
AC_CHECK_LIB([m], [ceil], [],
	     AC_MSG_ERROR([*** Libmath not found.])
	     )

AC_CHECK_LIB([elf], [elf_version], [],
	     AC_MSG_ERROR([*** Libelf not found. Specify a different path or install it. --help for info.])
	     )

AC_CHECK_LIB([forms], [fl_initialize], [],
	     AC_MSG_ERROR([*** Libforms not found. Specify a different path or install it. --help for info.])
	     )

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h libelf.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h unistd.h])

AC_CHECK_HEADER([forms.h],[],[AC_CHECK_HEADER([X11/forms.h],[UMPSCPPFLAG="${UMPSCPPFLAG} -I/usr/include/X11"])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# We can specify crosscompiler PATH or use the conventional /usr/local/bin
AC_ARG_WITH([cross],[  --with-cross[=PATH]       pathname of the crosscompiler set.], [CROSSBIN="${withval}"], [CROSSBIN="/usr/local/bin"])

CROSSBIN=`readlink -f $CROSSBIN`

# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([getcwd memset socket strchr strerror strstr strtoul])

# Setting the arch-dependent prefix and flags.
if test "x$ac_cv_c_bigendian" = "xyes"
then
	crosscompiler_prefix="mips-linux"
	LDCOREFLAGS="-T ../support/elf32btsmip.h.umpscore.x"
	LDAOUTFLAGS="-T ../support/elf32btsmip.h.umpsaout.x"
	LDCORESCRIPT="elf32btsmip.h.umpscore.x"
	LDAOUTSCRIPT="elf32btsmip.h.umpsaout.x"
else
	
	crosscompiler_prefix="mipsel-linux"
	LDCOREFLAGS="-T ../support/elf32ltsmip.h.umpscore.x"
	LDAOUTFLAGS="-T ../support/elf32ltsmip.h.umpsaout.x"
	LDCORESCRIPT="elf32ltsmip.h.umpscore.x"
	LDAOUTSCRIPT="elf32ltsmip.h.umpsaout.x"
fi

# Checking for gcc, ar, as and ld crosscompiling tools.
AC_CHECK_PROGS([GCCUMPS], ["${crosscompiler_prefix}"-gcc], ["no"], ["${CROSSBIN}"])
if test "x${GCCUMPS}" = "xno"
then
	AC_MSG_ERROR([*** ${crosscompiler_prefix}-gcc not found in ${CROSSBIN}. Specify a different path or install it.])
fi
AC_CHECK_PROGS([ASUMPS], ["${crosscompiler_prefix}"-as], ["no"], ["${CROSSBIN}"])
if test "x${ASUMPS}" = "xno"
then
	AC_MSG_ERROR([*** ${crosscompiler_prefix}-as not found in ${CROSSBIN}. Specify a different path or install it.])
fi
AC_CHECK_PROGS([ARUMPS], ["${crosscompiler_prefix}"-ar], ["no"], ["${CROSSBIN}"])
if test "x${ARUMPS}" = "xno"
then
	AC_MSG_ERROR([*** ${crosscompiler_prefix}-ar not found in ${CROSSBIN}. Specify a different path or install it.])
fi
AC_CHECK_PROGS([LDUMPS], ["${crosscompiler_prefix}"-ld], ["no"], ["${CROSSBIN}"])
if test "x${LDUMPS}" = "xno"
then
	AC_MSG_ERROR([*** ${crosscompiler_prefix}-ld not found in ${CROSSBIN}. Specify a different path or install it.])
fi

# If cross-tools path have to be in the form /dir/
if test `expr ${CROSSBIN} : '.*/$'` -eq "0"
then
	CROSSBIN=${CROSSBIN}/
fi

# Checking for hosttype.
case "${host}" in
	*-*-linux*) 
	AC_DEFINE([LINUX], [linux], [Linuxonly convention])
	;;
esac

# We have to make umpsrc prefix-dependent
if test -f support/umpsrc 
then
	rm -rf support/umpsrc
fi


if test "x${prefix}" = "xNONE"
then
	sed -e s%PROGPREFIX%${ac_default_prefix}%g -e s%PACKAGENAME%${PACKAGE_NAME}%g support/umpsrc.sample >> support/umpsrc
else
	sed -e s%PROGPREFIX%${prefix}%g -e s%PACKAGENAME%${PACKAGE_NAME}%g support/umpsrc.sample >> support/umpsrc
fi

# Adding cross-tools complete pathname.
GCCUMPS=${CROSSBIN}${GCCUMPS}
ASUMPS=${CROSSBIN}${ASUMPS}
ARUMPS=${CROSSBIN}$ARUMPS}
LDUMPS=${CROSSBIN}${LDUMPS}

# We have to setup example's Makefiles
for i in `find ./*example* -name Makefile.mod`
do
	rm -rf `dirname $i`/Makefile
	sed 	-e s%LD_AOUT_FLAGS%${LDAOUTSCRIPT}%g \
		-e s%LD_CORE_FLAGS%${LDCORESCRIPT}%g \
		-e s%GCC-UMPS%${GCCUMPS}%g \
		-e s%LD-UMPS%${LDUMPS}%g \
		-e s%AS-UMPS%${ASUMPS}%g \
		-e s%PACKAGENAME%${PACKAGE_NAME}%g \
		$i >> `dirname $i`/Makefile
done
	
# Setting variable for @@ substitution in Makefile.am
CCAS=${ASUMPS}
AC_SUBST(CCAS)
CCASFLAGS="-KPIC"
AC_SUBST(CCASFLAGS)
AC_SUBST(UMPSCPPFLAG)
AC_SUBST(LDCOREFLAGS)
AC_SUBST(LDAOUTFLAGS)
AC_SUBST(LDCORESCRIPT)
AC_SUBST(LDAOUTSCRIPT)

AC_CONFIG_FILES([Makefile]
		[src/Makefile]
		[support/Makefile])
				
AC_OUTPUT
