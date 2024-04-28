EXTRA_DIST=PstoeditQtGui.aps PstoeditQtGui.cpp PstoeditQtGui.h PstoeditQtGui.qrc PstoeditQtGui.rc PstoeditQtGui.sln PstoeditQtGui.ui PstoeditQtGui.pro
EXTRA_DIST+=resource.h pstoeditlogo-medium.ico PstoeditQtGuiMain.cpp
#EXTRA_DIST+=PstoeditQtGui.vcxproj PstoeditQtGui.vcxproj.filters PstoeditQtGui.vcxproj.user 

GUI:    PstoeditQtGui.pro
	qmake6 -o Makefile.qt PstoeditQtGui.pro
	make -f Makefile.qt
	make -f Makefile.qt clean
