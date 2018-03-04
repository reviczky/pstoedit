EXTRA_DIST=pstoedit.ph makefile.txt

if HAVE_PLUGINSRC
# LIBPLUGINS_ENTRY=libp2edrvplugins.la
endif

if HAVE_LIBPLOTTER
LIBPLOTTER_ENTRY=libp2edrvlplot.la
endif

if HAVE_LIBMING
LIBMING_ENTRY=libp2edrvswf.la
endif

if HAVE_LIBMAGICK
LIBMAGICK_ENTRY=libp2edrvmagick++.la
endif

if HAVE_LIBEMF
LIBEMF_ENTRY=libp2edrvwmf.la
endif

AM_CPPFLAGS=					\
	${LIBPLOTTER_CFLAGS}			\
	${LIBMING_CFLAGS}			\
	${MKSTEMP_CFLAGS}			\
	${LIBMAGICK_CFLAGS}			\
	${LIBEMF_CFLAGS}			\
	${LIBPNG_CFLAGS}			\
        -DDEFAULTGS=$(GS)			\
	-DPSTOEDITDATADIR="\"$(pkgdatadir)\""	\
	-DPSTOEDITLIBDIR="\"$(pkglibdir)\""	\
        ${SPECIAL_CXXFLAGS}

lib_LTLIBRARIES=libpstoedit.la
bin_PROGRAMS=pstoedit
pkglib_LTLIBRARIES=libp2edrvstd.la		\
	${LIBPLUGINS_ENTRY}			\
	${LIBPLOTTER_ENTRY}			\
	${LIBMING_ENTRY}			\
	${LIBMAGICK_ENTRY}			\
	${LIBEMF_ENTRY}

OLD_pkglibdir = $(libdir)/$(PACKAGE)$(PACKAGE_VERSION)

pkglibdir = $(libdir)/$(PACKAGE)


sample_drivers_src=drvsampl.cpp drvsampl.h
specific_drivers_src=				\
	drvgnplt.cpp drvgnplt.h			\
	drvtgif.cpp drvtgif.h			\
	drvfig.cpp drvfig.h			\
	drvidraw.cpp  drvidraw.h

libp2edrvstd_la_SOURCES=			\
	initlibrary.cpp 			\
	drvgcode.cpp    drvgcode.h		\
	drvcairo.cpp    drvcairo.h		\
	drvlwo.cpp      drvlwo.h		\
	drvrib.cpp      drvrib.h		\
	drvrpl.cpp      drvrpl.h		\
	drvdxf.cpp      drvdxf.h 	        \
	dxfacad.h 	dxficad.h		\
	drvjava.cpp 	drvjava.h		\
	drvjava2.cpp 	drvjava2.h		\
	drvpdf.cpp 	drvpdf.h		\
	drvkillu.cpp 	drvkillu.h		\
	drvtext.cpp 	drvtext.h		\
	drvsk.cpp 	drvsk.h			\
	drvmpost.cpp 	drvmpost.h		\
	drvasy.cpp 	drvasy.h		\
	drvmma.cpp 	drvmma.h		\
	drvlatex2e.cpp 	drvlatex2e.h		\
	drvnoi.cpp 	drvnoi.h		\
	drvpic.cpp 	drvpic.h		\
	drvhpgl.cpp 	drvhpgl.h		\
	drvpcb1.cpp	drvpcb1.h                 \
	drvpcb2.cpp	drvpcb2.h                 \
	drvpcbfill.cpp	drvpcbfill.h                 \
	drvgschem.cpp	drvgschem.h                 \
	drvtk.cpp	drvtk.h                 \
	drvvtk.cpp	drvvtk.h                 \
	drvsvm.cpp	drvsvm.h svm_actions.h	\
	$(specific_drivers_src)                 \
        $(sample_drivers_src)                   
libp2edrvstd_la_LIBADD=-L. libpstoedit.la
libp2edrvstd_la_LDFLAGS=-no-undefined -module

#libp2edrvplugins_la_SOURCES=drvmif.cpp drvcgm.cpp drvrtf.cpp drvwwmf.cpp
#libp2edrvplugins_la_LIBADD=
#libp2edrvplugins_la_LDFLAGS=
#libp2edrvplugins_la_CFLAGS=-I../Check/ -I../othersrc/cdlib

libp2edrvlplot_la_SOURCES=drvlplot.cpp drvlplot.h initlibrary.cpp
libp2edrvlplot_la_LIBADD=$(LIBPLOTTER_LDFLAGS) -L. libpstoedit.la
libp2edrvlplot_la_LDFLAGS=-no-undefined -module

libp2edrvswf_la_SOURCES=drvswf.cpp drvswf.h initlibrary.cpp
libp2edrvswf_la_LIBADD=$(LIBMING_LDFLAGS) -L. libpstoedit.la
libp2edrvswf_la_LDFLAGS=-no-undefined -module

# note the __ stand for ++ - automake treats all no alphanums chars as _
libp2edrvmagick___la_SOURCES=drvmagick++.cpp drvmagick++.h initlibrary.cpp
libp2edrvmagick___la_LIBADD=-L. libpstoedit.la
libp2edrvmagick___la_LDFLAGS=-no-undefined ${LIBMAGICK_LDFLAGS}  -module
# moved to configure.ac because needed only for g++ libp2edrvmagick___la_CPPFLAGS=$(LIBMAGICK_CFLAGS) $(AM_CPPFLAGS) -Wno-long-long

if USE_EMFBYSOURCE
libp2edrvwmf_la_SOURCES=emflib.cpp drvwmf.cpp drvwmf.h initlibrary.cpp
libp2edrvwmf_la_LDFLAGS=-no-undefined -L. libpstoedit.la -module
else
libp2edrvwmf_la_SOURCES=drvwmf.cpp drvwmf.h initlibrary.cpp
libp2edrvwmf_la_LDFLAGS=-no-undefined $(LIBEMF_LDFLAGS) -L. libpstoedit.la -module
endif

libpstoedit_la_SOURCES =			\
	dynload.cpp dynload.h			\
	callgs.cpp				\
	lexyy.cpp				\
	miscutil.cpp 				\
	cbstream.cpp cbstream.h			\
	psimage.cpp				\
	drvbase.cpp				\
	base64writer.cpp			\
	poptions.cpp				\
	subpath.cpp  subpath.h			\
	pstoedit.cpp				\
	psfront.h				\
	genericints.h 				\
	version.h

libpstoedit_la_LIBADD = ${CXX_STD_LIB} ${CXX_RUNTIME_LIB} -ldl -lm
libpstoedit_la_LDFLAGS = -no-undefined

pstoedit_SOURCES = cmdmain.cpp
pstoedit_LDFLAGS = -no-undefined -L. libpstoedit.la ${LIBLD_LDFLAGS}

pkginclude_HEADERS =				\
	pstoedit.h				\
	pstoedll.h				\
	pstoeditoutputlib.h			\
        pstoeditoptions.h			\
	cppcomp.h				\
	drvbase.h				\
	drvfuncs.h				\
	miscutil.h				\
	genericints.h				\
	base64writer.h				\
	poptions.h				\
	papersizes.h				\
	ordlist.h				\
	psimage.h

