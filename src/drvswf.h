#ifndef __drvSWF_h
#define __drvSWF_h

/* 
   drvSWF.h : This file is part of pstoedit
   Class declaration for a SWF output driver with no additional attributes
   and methods (minimal interface)

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

class drvSWF : public drvbase {

public:

	derivedConstructor(drvSWF);
	//(const char * driveroptions_P,ostream & theoutStream,ostream & theerrStream ); // Constructor

	~drvSWF(); // Destructor
	class DriverOptions : public ProgramOptions { 
	public:
		Option < bool, BoolTrueExtractor > cubic;
		Option < bool, BoolTrueExtractor > trace;
		DriverOptions():
			cubic(true,"-cubic",0,0,"cubic ???",0,false),
			trace(true,"-trace",0,0,"trace ???",0,false)
		{
			ADD(cubic);
			ADD(trace);
		}
	}*options;

	virtual void show_image(const PSImage & imageinfo); 
#include "drvfuncs.h"
	void show_text(const TextInfo & textInfo);

private:
	void print_coords();


	int imgcount;
	float swfscale;
	class SWFMovie * movie;

	typedef float coordtype ;

	coordtype swfx(const Point & p) const { return swfscale * (p.x_ + x_offset); }
	coordtype swfy(const Point & p) const { return swfscale * (currentDeviceHeight + y_offset - p.y_ ); }

};

#endif
 
 
