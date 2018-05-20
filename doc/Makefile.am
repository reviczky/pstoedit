EXTRA_DIST=readme.txt changelog.htm pstoedit.tex pstoedit.trans
man_MANS = pstoedit.1


# Section for pstoedit documentation
# (adapted from `Makefile' in the latex2man package) 
# Rolf Niepraschk, niepraschk@ptb.de, 99/08/20
#

SUFFIXES = .htm .1 .tex .ps .info .texi .txt

all : alldoc
gendocs : alldoc
alldoc : pstoedit.1 pstoedit.htm pstoedit.txt pstoedit.pdf

P=../src/pstoedit

generalhelplong.tex: $P
	$P -doculong | grep -v "DLL interface" > generalhelplong.tex

generalhelpshort.tex: $P
	$P -docushort | grep -v "DLL interface" > generalhelpshort.tex

driverhelp.tex: $P
	$P -dumphelp | grep -v "DLL interface" | sed -e 's/_/\\_/' -e 's/>=/$>=$ /' > driverhelp.tex

latex_src = pstoedit.tex version.tex generalhelplong.tex generalhelpshort.tex driverhelp.tex

pstoedit.dvi : $(latex_src)
	latex $(srcdir)/pstoedit.tex

pstoedit.ps : pstoedit.dvi
	dvips -Pwww  -o pstoedit.ps pstoedit.dvi

pstoedit.pdf : $(latex_src)
	pdflatex $(srcdir)/pstoedit.tex
	pdflatex $(srcdir)/pstoedit.tex

pstoedit.info : pstoedit.texi
	makeinfo pstoedit.texi

pstoedit.txt : pstoedit.1
	groff -t -man -Tlatin1 pstoedit.1    |	\
		sed -e 's/.//g' 	\
		    -e 's/7|//g' 	\
		    -e 's/[6-9]//g' 	\
		    -e 's///g'      	\
		    -e 's/­$$/- /'      > pstoedit.txt

pstoedit.1 : $(latex_src)
	latex2man -t $(srcdir)/pstoedit.trans -M $(srcdir)/pstoedit.tex pstoedit.1

pstoedit.htm : $(latex_src)
	latex2man -t $(srcdir)/pstoedit.trans -H $(srcdir)/pstoedit.tex pstoedit.htm

pstoedit.texi : $(latex_src)
	latex2man -t $(srcdir)/pstoedit.trans -T $(srcdir)/pstoedit.tex pstoedit.texi

pstoedit.rtf : pstoedit.1
	rman -f RTF pstoedit.1 pstoedit.rtf

MOSTLYCLEANFILES = pstoedit.aux pstoedit.log pstoedit.out pstoedit.dvi

CLEANFILES = pstoedit.ps pstoedit.pdf pstoedit.1 pstoedit.htm pstoedit.texi \
	  pstoedit.info pstoedit.txt pstoedit.rtf generalhelplong.tex \
	  generalhelpshort.tex driverhelp.tex
