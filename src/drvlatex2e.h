#ifndef __drvLATEX2E_h
#define __drvLATEX2E_h

/* 
   drvlatex2e.h : This file is part of pstoedit
   Class declaration for a LaTeX2e output driver
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
//#ifdef HAVESTL
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


#include I_iostream

USESTD

#include I_streamb
#include <math.h>
#include I_string_h


class drvLATEX2E : public drvbase {

public:

  derivedConstructor(drvLATEX2E);
  //(const char * driveroptions_P,ostream & theoutStream,ostream & theerrStream ); // Constructor

  ~drvLATEX2E(); // Destructor
	class DriverOptions : public ProgramOptions { 
	public:
		Option < bool, BoolTrueExtractor> integersonly;

		DriverOptions():
			integersonly(true,"-integers","",0,"round all coordinates to the nearest integer",0,false)
		{
			ADD(integersonly);
		}
	
	}*options;

#include "drvfuncs.h"
  void show_rectangle(const float llx, const float lly, const float urx, const float ury);
  void show_text(const TextInfo & textInfo);

//not supported yet  virtual void show_image(const Image & imageinfo); 

private:

	void print_coords();
//  typedef queue<string> STRQUEUE;
//  STRQUEUE outputQ;  
  
  	TempFile      tempFile;
	ofstream      &buffer;  
	// Queue for buffering our output until we know our bounding box

  // Bounding box for the current picture
  Point boundingbox[2];

  // Expand the bounding box if a given point is outside it.
  inline void updatebbox (const Point & newpoint) {
    if (newpoint.x_ < boundingbox[0].x_)
      boundingbox[0].x_ = newpoint.x_;
    if (newpoint.y_ < boundingbox[0].y_)
      boundingbox[0].y_ = newpoint.y_;
    if (newpoint.x_ > boundingbox[1].x_)
      boundingbox[1].x_ = newpoint.x_;
    if (newpoint.y_ > boundingbox[1].y_)
      boundingbox[1].y_ = newpoint.y_;
  }

  // Convert from PostScript points to TeX points ("bp"-->"pt" in TeX
  // terminology).
  static inline void scalepoint (Point & somepoint) {
	const float adjustment = 72.27f / 72.0f;
    somepoint.x_ *= adjustment;
    somepoint.y_ *= adjustment;
  }

  // Previous position, color, line thickness, and font
  Point currentpoint;
  float prevR, prevG, prevB;
  bool thicklines;
  string prevfontname;
  float prevfontsize;
};

//#endif // HAVE STL

#endif
 
 
