#ifndef __drvTK_h
#define __drvTK_h

/* 
   drvtk.h   - Header file for driver to output Tcl/Tk canvas
             - written by Christopher Jay Cox (cjcox@acm.org) - 9/22/97
               updated on 7/17/00
               http://www.ntlug.org/~ccox/impress/
               Based on...
  
   drvsample.h
   Interface for new driver backends

   Copyright (C) 1993 - 2009 Wolfgang Glunz, wglunz35_AT_pstoedit.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "drvbase.h"


class drvTK : public drvbase {

public:

	derivedConstructor(drvTK);

	~drvTK(); // Destructor
	class DriverOptions : public ProgramOptions { 
	public:

		Option < bool, BoolTrueExtractor> swapHW;
		Option < bool, BoolTrueExtractor> noImPress;
		Option < RSString, RSStringValueExtractor> tagNames;

		DriverOptions() :
			swapHW(true,"-R",0,0,"swap HW",0,false),
			noImPress(true,"-I",0,0,"no impress",0,false),
			tagNames(true,"-n","string",0,"tagnames",0,"")
		{
			ADD(swapHW);
			ADD(noImPress);
			ADD(tagNames);
		};

	} * options;

#include "drvfuncs.h"
	void show_text(const TextInfo & textInfo);

private:
	void print_coords();
	TempFile		tempFile;
	ofstream		&buffer;
	int			objectId;
	const struct PaperInfo * paperinfo;

	void			canvasCreate();
	void			outputEscapedText(const char* string);


};
#endif
 
 
