EXTRA_DIST=readme.txt changelog.htm pstoedit.1 pstoedit.htm pstoedit.main.tex pstoedit.tex pstoedit.trans
man_MANS = pstoedit.1


# Section for pstoedit documentation
# (adapted from `Makefile' in the latex2man package) 
# Rolf Niepraschk, niepraschk@ptb.de, 99/08/20
#

SUFFIXES = .htm .1 .tex .ps .info .texi .txt 

P=../src/pstoedit

generalhelplong.tex: $P
	$P -doculong | grep -v "DLL interface" > generalhelplong.tex

generalhelpshort.tex: $P
	$P -docushort | grep -v "DLL interface" > generalhelpshort.tex

driverhelp.tex: $P
	$P -dumphelp | grep -v "DLL interface" | sed -e 's/_/\\_/' -e 's/>=/$>=$ /' > driverhelp.tex

pstoedit.gen.tex : ../src/version.h generalhelplong.tex generalhelpshort.tex driverhelp.tex pstoedit.main.tex
	cpp -Uunix -traditional-cpp -C -P pstoedit.main.tex | sed 's/BREAK/\\\\/g' > pstoedit.gen.tex
	cp pstoedit.gen.tex pstoedit.tex

gendocs : alldoc

alldoc : pstoedit.gen.tex pstoedit.1 pstoedit.htm pstoedit.txt pstoedit.dvi pstoedit.ps pstoedit.pdf 

pstoedit.dvi : pstoedit.tex
	latex pstoedit.tex

pstoedit.ps : pstoedit.dvi
	dvips -Pwww  -o pstoedit.ps pstoedit.dvi

pstoedit.pdf : pstoedit.tex
	pdflatex pstoedit.tex
	pdflatex pstoedit.tex

pstoedit.info : pstoedit.texi
	makeinfo pstoedit.texi

pstoedit.txt : pstoedit.1
	groff -t -man -Tlatin1 pstoedit.1    |	\
		sed -e 's/.//g' 	\
		    -e 's/7|//g' 	\
		    -e 's/[6-9]//g' 	\
		    -e 's///g'      	\
		    -e 's/­$$/- /'      > pstoedit.txt

pstoedit.1 : pstoedit.tex
	latex2man -t pstoedit.trans -M pstoedit.tex pstoedit.1 

pstoedit.htm : pstoedit.tex
	latex2man -t pstoedit.trans -H pstoedit.tex pstoedit.htm

pstoedit.texi : pstoedit.tex
	latex2man -t pstoedit.trans -T pstoedit.tex pstoedit.texi
	
pstoedit.rtf : pstoedit.1
	rman -f RTF pstoedit.1 pstoedit.rtf
	
	

clean :
	echo rm -f pstoedit.aux pstoedit.log pstoedit.dvi 

realclean : clean
	echo $(RM) pstoedit.ps pstoedit.pdf pstoedit.1 pstoedit.htm pstoedit.texi \
	  pstoedit.info pstoedit.txt pstoedit.rtf
	

# -- eof --
