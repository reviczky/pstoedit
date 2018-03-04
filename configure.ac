dnl Configure a Makefile for pstoedit
dnl initial version by Scott Pakin <scott+ps2ed_AT_pakin.org>

dnl Adapting automake
dnl By Masatake YAMATO<jet _AT_ gyve.org>

dnl ------------------------------------------------------------------------
dnl NOTE: I have tested this only on Linux/g++, Solaris/g++, and Solaris/CC,
dnl       as those are all the systems I have access to.
dnl ------------------------------------------------------------------------

dnl Initialize Autoconf. 
AC_INIT(pstoedit,3.50,wglunz35_AT_pstoedit.net)
AC_LANG(C++)
dnl AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(pstoedit,3.50)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/pstoedit.h])
AC_CONFIG_SUBDIRS(libEMF-1.0)

dnl We need only C++ and no other language
dnl seems to be broken : AC_LIBTOOL_TAGS([CXX])

dnl Check for various programs the Makefile needs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_YACC
AC_PROG_INSTALL
dnl there is not need to build the static libs if shared libs are available
AC_DISABLE_STATIC

dnl yes, we are sure to have clean DLLs under cygwin
AC_LIBTOOL_WIN32_DLL 

AC_PROG_LIBTOOL
AC_PROG_MAKE_SET

AC_CONFIG_HEADER(src/pstoedit_config.h)

dnl Check file command
dnl FILECMD=""
dnl AC_PATH_PROG(FILECMD,file)
dnl if test -z "${FILECMD}"; then
dnl    AC_MSG_ERROR([** Cannot find file command which is needed for libtool. Please install. ])
dnl    exit 1
dnl fi

dnl Check Ghostscript
GS=""
AC_PATH_PROG(GS,gs)
if test -z "${GS}"; then
   AC_MSG_ERROR([** Cannot find ghostscript. If it is already installed, Check PATH.])
else
dnl Strip off // to / that causes troubles.
   GS=`echo $GS | sed -e 's/\/\//\//g'`	
fi

PKG_PROG_PKG_CONFIG


AC_C_BIGENDIAN([CPPFLAGS="$CPPFLAGS -D_BIG_ENDIAN"], [CPPFLAGS="$CPPFLAGS -D_LITTLE_ENDIAN"])

dnl check for directory handling
AC_HEADER_DIRENT

dnl 
dnl Look for various optional libraries.
dnl
dnl TODO: These should be replaced by ltdl.
HAVE_LD=no
LIBLD_LDFLAGS=
AC_CHECK_LIB(dl,main,[LIBLD_LDFLAGS=-ldl])
if test -n "${LIBLD_LDFLAGS}"; then
   HAVE_LD=yes
fi
AC_SUBST(LIBLD_LDFLAGS)

AC_CHECK_LIB([gd],[gdImageCreateFromPng])

dnl some optional packages - default is to check automatically
dnl but the check can be disabled using --without-PACKAGE
dnl legal PACKAGE names are:
dnl emf, magick, libplot, swf


AC_ARG_WITH(emf,
[  --with-emf  enable/disable emf interface (default: enabled - use --without-emf to disable)],
with_emf=$withval, with_emf=yes)
AC_ARG_WITH(magick,
[  --with-magick  enable/disable magick++ interface (default: enabled - use --without-magick to disable)],
with_magick=$withval, with_magick=yes)
AC_ARG_WITH(libplot,
[  --with-libplot  enable/disable libplot interface (default: enabled - use --without-libplot to disable)],
with_libplot=$withval, with_libplot=yes)
AC_ARG_WITH(swf,
[  --with-swf  enable/disable swf interface (default: enabled - use --without-swf to disable)],
with_swf=$withval, with_swf=yes)


OLD_CPPFLAGS=${CPPFLAGS}
CPPFLAGS="$CPPFLAGS -DX_DISPLAY_MISSING"

if test -f src/drvwwmf.cpp ; then
	HAVE_PLUGINSRC=true;
	AM_CONDITIONAL(HAVE_PLUGINSRC, true)
else
	HAVE_PLUGINSRC=false;
	AM_CONDITIONAL(HAVE_PLUGINSRC, false)
fi

if test "${with_libplot}" = "yes" ; then
dnl
dnl Driver::libplotter
dnl
HAVE_LIBPLOTTER=no
LIBPLOTTER_LDFLAGS=
LIBPLOTTER_CFLAGS=
dnl AC_CHECK_LIB(plotter,main,
dnl	[LIBPLOTTER_LDFLAGS=-lplotter],,-lstdc++)
AC_CHECK_LIB(plotter,main,
	[LIBPLOTTER_LDFLAGS=-lplotter])
AC_CHECK_HEADER(plotter.h,
        [LIBPLOTTER_CFLAGS="-DHAVE_LIBPLOTTER"])

if test -n "${LIBPLOTTER_LDFLAGS}" && test -n "${LIBPLOTTER_CFLAGS}"; then
   HAVE_LIBPLOTTER=yes
   AC_MSG_CHECKING(for version of "lib plot" )   
   dnl See if we have a 2.3 or newer lib Plot version
   AC_TRY_COMPILE( [
		#include <plotter.h>
	     	typedef int Point; 
		],[ ],
		    AC_MSG_RESULT(you seem to have a compatible version ),
		    [LIBPLOTTER_CFLAGS="$LIBPLOTTER_CFLAGS -DOLD_LIBPLOTTER";   
		    AC_MSG_RESULT(you seem to have an older version - need at least version 2.3)])
else
   LIBPLOTTER_LDFLAGS=
   LIBPLOTTER_CFLAGS=
   AC_MSG_WARN([** libplotter driver is disabled.])
fi
AC_SUBST(LIBPLOTTER_LDFLAGS)
AC_SUBST(LIBPLOTTER_CFLAGS)
AM_CONDITIONAL(HAVE_LIBPLOTTER, test "$HAVE_LIBPLOTTER" = yes)
else
   AC_MSG_WARN([libplot support disabled by configure argument .])
   AM_CONDITIONAL(HAVE_LIBPLOTTER, false )
fi

CPPFLAGS=${OLD_CPPFLAGS}





AC_MSG_CHECKING(for mkstemp)
AC_TRY_COMPILE( [#include <stdlib.h>
		 char buffer[] = "aaaXXXXXX";
	 	 int i = mkstemp(buffer); ],[ ],
               [HAVE_MKSTEMP=yes;
                MKSTEMP_CFLAGS="-DHAVE_MKSTEMP"; 
                AC_MSG_RESULT(you seem to have mkstemp )], 
               [AC_MSG_RESULT(you do not have mkstemp - so will try using tempnam instead)])

AC_SUBST(MKSTEMP_CFLAGS)



if test "${with_swf}" = "yes" ; then
dnl
dnl Driver::libming
dnl
HAVE_LIBMING=no
LIBMING_LDFLAGS=
LIBMING_CFLAGS=

dnl If mingpp.h is found, use it.
AC_CHECK_LIB(ming,main,
	[LIBMING_LDFLAGS=-lming],,-lm)
AC_CHECK_HEADER(mingpp.h,
	[LIBMING_CFLAGS="-DHAVE_LIBMING"])
if test -n "${LIBMING_LDFLAGS}" && test -n "${LIBMING_CFLAGS}"; then
   HAVE_LIBMING=yes
   AC_CHECK_HEADER(util/png2dbl.c,[LIBMING_CFLAGS="$LIBMING_CFLAGS -DHAVE_LIBMINGUTIL"])
else
   LIBMING_LDFLAGS=
   LIBMING_CFLAGS=
   AC_MSG_WARN([** libming driver is disabled.])
fi
AC_SUBST(LIBMING_LDFLAGS)
AC_SUBST(LIBMING_CFLAGS)
AM_CONDITIONAL(HAVE_LIBMING, test "$HAVE_LIBMING" = yes)
else
   AC_MSG_WARN([swf support disabled by configure argument .])
   AM_CONDITIONAL(HAVE_LIBMING, false )
fi






if test "${with_magick}" = "yes" ; then
dnl
dnl Driver::libmagick++
dnl If Magick++.h is found, use it.
dnl
HAVE_LIBMAGICK=no
LIBMAGICK_CFLAGS=
LIBMAGICK_LDFLAGS=
PSTOEDIT_GCC_OK=yes
if test "x${CC}" = xgcc; then 
   AC_MSG_CHECKING(for gcc version)
   GCC_VERSION=`${CC} --version`
   changequote(,)dnl
   GCC_MAJOR_VERSION=`echo $GCC_VERSION \
   | sed -e "s/^\([0-9]*\)\.[0-9]*/\1/"`
   GCC_MINOR_VERSION=`echo $GCC_VERSION \
   | sed -e "s/^[0-9]*\.\([0-9]*\)/\1/"`
   changequote([,])dnl
   AC_MSG_RESULT([${GCC_MAJOR_VERSION}.${GCC_MINOR_VERSION}])
   if test "x${GCC_MAJOR_VERSION}" = x2; then
      PSTOEDIT_GCC_OK=no
   fi
fi

if test "x${PSTOEDIT_GCC_OK}" = xno ; then
      AC_MSG_WARN([** gcc version is too old to use Magick++ in pstoedit]) 
      AC_MSG_WARN([** gcc version >= 3 is required.]) 
      AC_MSG_WARN([** libMagick++ driver is disabled.])
else
      if test -n "$PKG_CONFIG"; then
        AC_MSG_CHECKING(ImageMagick flags through pkg-config)
	PKG_CHECK_MODULES(LIBMAGICK, ImageMagick++, 
		HAVE_LIBMAGICK=yes 
		LIBMAGICK_CFLAGS="-DHAVE_MAGIC $LIBMAGICK_CFLAGS" 
		LIBMAGICK_LDFLAGS="$LIBMAGICK_LIBS" , 
		echo "trying other methods for finding ImageMagick")
      else
	AC_PATH_PROG(MAGICK_CONFIG, Magick++-config)
	if test -n "${MAGICK_CONFIG}"; then
	   AC_MSG_CHECKING(for version of "GraphicsMagick++" )   
	   dnl See if we have a 1.0.6 or newer version
	   OLD_CPPFLAGS=${CPPFLAGS}
	   CPPFLAGS="$CPPFLAGS `Magick++-config --cppflags`"
	   AC_TRY_COMPILE( [#include <Magick++.h>
	            #if MagickLibVersion >= 0x000008
	 	    int correct;
	  	    #else
	  	    old version of GraphicsMagick;
	  	    #endif ],[ ],
	  	    MAGICKCORRECTVERSION="YES";AC_MSG_RESULT(you seem to have a compatible version ),
	  	    [AC_MSG_RESULT(you seem to have an older version - need at least version 1.0.6)])
	   CPPFLAGS=$OLD_CPPFLAGS
	   if test -n "${MAGICKCORRECTVERSION}"; then
	      HAVE_LIBMAGICK=yes
	      LIBMAGICK_CFLAGS="-DHAVE_MAGIC `Magick++-config --cppflags`"
	      LIBMAGICK_LDFLAGS="`Magick++-config --libs` `Magick++-config --ldflags`"
	   else
	      AC_MSG_WARN([** libMagick++ driver is disabled because of too old version of GraphicsMagick (need at least 1.0.6).])
	   fi
	else
	      AC_MSG_WARN([** libMagick++ driver is disabled.])
	fi
      fi
fi
AC_SUBST(LIBMAGICK_CFLAGS)
AC_SUBST(LIBMAGICK_LDFLAGS)
AM_CONDITIONAL(HAVE_LIBMAGICK, test "$HAVE_LIBMAGICK" = yes)
else
   AC_MSG_WARN([magick support disabled by configure argument .])
   AM_CONDITIONAL(HAVE_LIBMAGICK, false )
fi





if test "${with_emf}" = "yes" ; then
dnl
dnl Driver::libEMF
dnl Check for EMF support library
dnl
HAVE_LIBEMF=no
LIBEMF_CFLAGS=
LIBEMF_LDFLAGS=

OLD_CPPFLAGS=
OLD_LIBS=


AC_ARG_WITH(libemf-src,
[  --with-libemf-src=DIR           look for libEMF src in dir (e.g. /usr/local/lib)],
libemf_lib_src=$withval, libemf_lib_src=)

if test -n "${libemf_lib_src}"; then
dnl use libemf source directly
HAVE_LIBEMF=yes
AM_CONDITIONAL(USE_EMFBYSOURCE, true )
AM_CONDITIONAL(HAVE_LIBEMF, true )
LIBEMF_CFLAGS="-DHAVE_LIBEMF -I${libemf_lib_src}/include -I${libemf_lib_src}/libemf "
AC_SUBST(LIBEMF_CFLAGS)

else
AM_CONDITIONAL(USE_EMFBYSOURCE, false )

AC_ARG_WITH(libemf-lib,
[  --with-libemf-lib=DIR           look for libEMF library in dir (e.g. /usr/local/lib)],
libemf_lib_dir=$withval, libemf_lib_dir=)

AC_ARG_WITH(libemf-include,
[  --with-libemf-include=DIR       look for libEMF header in dir (e.g. /usr/local/include/libEMF)],
libemf_include_dir=$withval, libemf_include_dir=)

OLD_LIBS=${LIBS}
if test -n "${libemf_lib_dir}"; then
   LIBS="-L${libemf_lib_dir} ${LIBS}"
fi
AC_CHECK_LIB(EMF,main, [LIBEMF_LDFLAGS="-lEMF -lm"],,-lm)
if test -n "${LIBEMF_LDFLAGS}"; then
  if test -n "${libemf_lib_dir}"; then
     LIBEMF_LDFLAGS="-L${libemf_lib_dir} ${LIBEMF_LDFLAGS}"
  fi   
else
      AC_MSG_WARN([** Cannot find libEMF. libEMF supports are disabled.])
fi
LIBS=${OLD_LIBS}

OLD_CPPFLAGS=${CPPFLAGS}
if test -n "${libemf_include_dir}"; then
   CPPFLAGS="-I${libemf_include_dir} ${CPPFLAGS}"
fi
AC_CHECK_HEADER(emf.h,
		[LIBEMF_CFLAGS="-DHAVE_LIBEMF -DOLDLIBEMF"])
AC_CHECK_HEADER(libEMF/emf.h,
		[LIBEMF_CFLAGS="-DHAVE_LIBEMF"])
if test -n "${LIBEMF_CFLAGS}"; then
   if test -n "${libemf_include_dir}"; then
	 LIBEMF_CFLAGS="-I${libemf_include_dir} ${LIBEMF_CFLAGS}"
   fi
else
   AC_MSG_WARN([** Cannot find emf.h. libEMF supports are disabled.])
fi
CPPFLAGS=${OLD_CPPFLAGS}

if test -n "${LIBEMF_LDFLAGS}" && test -n "${LIBEMF_CFLAGS}"; then
   HAVE_LIBEMF=yes
else
   LIBEMF_CFLAGS=
   LIBEMF_LDFLAGS=
fi

AC_SUBST(LIBEMF_LDFLAGS)
AC_SUBST(LIBEMF_CFLAGS)
AM_CONDITIONAL(HAVE_LIBEMF, test "$HAVE_LIBEMF" = yes)
fi
else
   AC_MSG_WARN([EMF support disabled by configure argument .])
   AM_CONDITIONAL(USE_EMFBYSOURCE, false )
   AM_CONDITIONAL(HAVE_LIBEMF, false )
fi





dnl
dnl PNG and Zlib
dnl
HAVE_LIBPNG=no
LIBPNG_CFLAGS=
LIBPNG_LDFLAGS=
AC_MSG_CHECKING(See if we have libpng and zlib)
AC_TRY_COMPILE( [#include <png.h>
	 	 int i = PNG_TRANSFORM_IDENTITY; ],[ ],
               [HAVE_LIBPNG=yes;
                LIBPNG_CFLAGS="-DHAVE_LIBPNG"; 
                LIBPNG_LDFLAGS="-lpng -lz ";
                AC_MSG_RESULT(you seem to have compatible version of libpng)], 
               [AC_MSG_RESULT(you either have no libpng or an old version - 1.0.8 should be ok.)])
AC_SUBST(LIBPNG_LDFLAGS)
AC_SUBST(LIBPNG_CFLAGS)
AM_CONDITIONAL(HAVE_LIBPNG, test "$HAVE_LIBPNG" = yes)

dnl
dnl C++ compiler related checking
dnl

dnl See if we need to define a "bool" type.
AC_MSG_CHECKING(for built-in "bool" datatype)
AC_TRY_COMPILE(,[bool testing_bool; testing_bool=true;],
               [CXXFLAGS="$CXXFLAGS -DINTERNALBOOL"; AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(no)])

dnl Add extra, platform-specific, C++ compiler options.
AC_MSG_CHECKING(C++ compiler model)
if test "$GXX" = yes; then
  if test "$host_os" = riscos; then
    AC_MSG_RESULT([GNU on RiscOS])
    SPECIAL_CXXFLAGS="-Wall -Dunix -Ilibgpp: -Ilibstdc: -Ilibio:"
    LDLIBS="-rescan libgpp:o.libg++ libstdc:o.libstdc++ libio:o.libiostrea gcc:o.libgcc Unix:o.UnixLib"
    LINK=link
    AC_SUBST(LINK)
    AC_SUBST(LDLIBS)
  else
    AC_MSG_RESULT([GNU])
#   need to disable long long warning here because of ImageMagick using it
    SPECIAL_CXXFLAGS="-DBUGGYGPP -pedantic -Wall -Wwrite-strings -Wcast-qual -Wpointer-arith -Wno-long-long "
  fi
  CXXFLAGS="-g" 
elif test `echo "$CXX" | egrep -c 'xlC$'` -eq 1; then
  AC_MSG_RESULT([AIX xlC])
  CXXFLAGS="-D__unix__ -g -+ -I/usr/lpp/xlC/src/nihC/aixinclude"  
elif test `echo "$CXX" | egrep -c 'cxx$'` -eq 1; then
  AC_MSG_RESULT([OSF/1 cxx])
  SPECIAL_CXXFLAGS=-w
  CXXFLAGS=-define_templates
elif test `echo "$CXX" | egrep -c 'CC$'` -eq 1; then
  if test "$host_os" = irix5.3; then
    AC_MSG_RESULT([Irix 5.3])
    SPECIAL_CXXFLAGS="-O2 -mips2 -Olimit 2400"
  elif test "$host_os" = irix6.2; then
    AC_MSG_RESULT([Irix 6.2])
    SPECIAL_CXXFLAGS="-mips3 -n32 -O2 -OPT:fold_arith_limit=2500"
  elif test "$host_vendor" = sun; then
    AC_MSG_RESULT([Sun])
    SPECIAL_CXXFLAGS="-w"
  fi
else
  AC_MSG_RESULT(unknown)
fi

AC_ARG_ENABLE(wrapper,
              [  --enable-wrapper=PROGRAM
                          Wrap (i.e. prefix) compiler invocations with PROGRAM
                          (e.g. \"insure\" or \"purify\")],
              [CXX="$enableval $CXX"])
AC_SUBST(SPECIAL_CXXFLAGS)

dnl
dnl Standard C++ library and C++ Runtime
dnl
CXX_RUNTIME_LIB=
CXX_STD_LIB=
if test x"$GXX" = xyes; then
   AC_CHECK_LIB(g++,main,CXX_RUNTIME_LIB=-lg++)
   AC_CHECK_LIB(stdc++,main,CXX_STD_LIB=-lstdc++,,${CXX_RUNTIME_LIB})
fi
AC_SUBST(CXX_STD_LIB)
AC_SUBST(CXX_RUNTIME_LIB)

dnl Write the resulting Makefile.
AC_CONFIG_FILES([Makefile 
src/Makefile 
config/Makefile config/pstoedit.pc
doc/Makefile
examples/Makefile 
contrib/Makefile contrib/cairo/Makefile
contrib/java/Makefile contrib/java/java1/Makefile contrib/java/java2/Makefile 
misc/Makefile misc/swffonts/Makefile 
os2/Makefile 
msdev/Makefile othersrc/Makefile othersrc/gsdllinc/Makefile],
[])

AC_OUTPUT
 
