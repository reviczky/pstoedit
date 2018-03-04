EXTRA_DIST=pstoedit.pc.in pstoedit-config.af pstoedit-config.in pstoedit.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= pstoedit.pc

bin_SCRIPTS=pstoedit-config
BUILT_SOURCES=pstoedit-config
pstoedit-config: pstoedit-config.in

pstoedit-config.in: pstoedit-config.af
	autofig $<

m4datadir = $(datadir)/aclocal
m4data_DATA = pstoedit.m4
