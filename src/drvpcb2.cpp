/* 
   drvpcb.cpp : This file is part of pstoedit
   simple backend for Pcb format.
   Contributed / Copyright 2004 by: Mark Rages 

   Copyright (C) 1993 - 2007 Wolfgang Glunz, wglunz34_AT_pstoedit.net
   (for the skeleton and the rest of pstoedit)

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

#include "drvpcb2.h"

#include I_stdio
#include I_string_h
#include I_iostream



drvPCB2::derivedConstructor(drvPCB2):
//(const char * driveroptions_p,ostream & theoutStream,ostream & theerrStream):
constructBase
{
// driver specific initializations
  outf << "PCB[\"\" 600000 500000]\n\n";
  outf << "Grid[2000.00000000 0 0 0]\n\n";
  outf << "Layer(10 \"silk\")\n(\n";
}

drvPCB2::~drvPCB2()
{
  outf << ")\n";
  options=0;
}


void drvPCB2::close_page()
{
  //outf << "#Seite beendet.\n";
}

void drvPCB2::open_page()
{
  //	outf << "#Neue Seite\n";
}
const float SCALE = (100000.0f/72.0f);

void drvPCB2::show_path()
{
  //	outf << "\n#Polyline:\n";


	for (unsigned int n = 1; n < numberOfElementsInPath(); n++) {
		const Point & p1 = pathElement(n-1).getPoint(0);
		const Point & p = pathElement(n).getPoint(0);
		outf << "Line[";
		outf << (int)(p1.x_*SCALE) << " " 
                     << (int)(500000-p1.y_*SCALE) << " " 
                     << (int)(p.x_*SCALE) << " " 
                     << (int)(500000-p.y_*SCALE) << " 1000 2000 0x00000020]\n";
	}
}


static DriverDescriptionT < drvPCB2 > D_pcb("pcb", "pcb format", 
											"See also: \\URL{http://pcb.sourceforge.net} ","pcb", false,	// if backend supports subpathes
												   // if subpathes are supported, the backend must deal with
												   // sequences of the following form
												   // moveto (start of subpath)
												   // lineto (a line segment)
												   // lineto 
												   // moveto (start of a new subpath)
												   // lineto (a line segment)
												   // lineto 
												   //
												   // If this argument is set to false each subpath is drawn 
												   // individually which might not necessarily represent
												   // the original drawing.
												   false,	// if backend supports curves
												   false,	// if backend supports elements with fill and edges
												   false,	// if backend supports text
												   DriverDescription::noimage,	// no support for PNG file images
												   DriverDescription::normalopen, false,	// if format supports multiple pages in one file
												   false /*clipping */ );
 
