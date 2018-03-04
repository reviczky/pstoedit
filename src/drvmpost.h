#ifndef __drvMPOST_h
#define __drvMPOST_h

/* 
   drvmpost.h : This file is part of pstoedit
   Backend for MetaPost files
   Contributed by: Scott Pakin <scott+ps2ed_AT_pakin.org>

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


#ifdef HAVESTL
#include <string>        // C++ string class
// STL no longer needed, use FontMapper from miscutil.h
//#include <set>           // C++ STL set class
//#include <memory>        // Needed for allocator class
//#include <functional>
#else
typedef RSString string;
#endif


class drvMPOST : public drvbase {

public:

	derivedConstructor(drvMPOST);

	~drvMPOST(); // Destructor
	class DriverOptions : public ProgramOptions {
	public:
		DriverOptions() 
		{
		}
	}*options;

#include "drvfuncs.h"
	void show_text(const TextInfo & textInfo);

private:
	void print_coords();
	// Previous values of graphics state variables
 	string prevFontName;
	float prevR, prevG, prevB;
 	float prevFontAngle;
 	float prevFontSize;
	float prevLineWidth;
	unsigned int prevLineCap;
	unsigned int prevLineJoin;
	string prevDashPattern;

	// Set to true if we're filling, false if we're stroking
	bool fillmode;

};

#endif
 
 
 
