#ifndef __drvHPGL_h
#define __drvHPGL_h

/* 
   drvhpgl.h : This file is part of pstoedit
   Class declaration for hpgl output driver with no additional attributes
   and methods (minimal interface)

   Copyright (C) 1993,1994,1995,1996,1997,1998 Peter Katzmann p.katzmann@thiesen.com 

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

   class drvHPGL : public drvbase {
   
protected:
	// = PROTECTED DATA

   public:
   
	derivedConstructor(drvHPGL);
 
   ~drvHPGL(); // Destructor
	class DriverOptions : public ProgramOptions {
	public:
		Option < bool, BoolTrueExtractor > penplotter ;
		Option < int, IntValueExtractor > maxPenColors; 
		Option < RSString, RSStringValueExtractor> fillinstruction;
	//	Option < bool, BoolTrueExtractor > useRGBcolors ;
		Option < bool, BoolTrueExtractor > rot90 ;
		Option < bool, BoolTrueExtractor > rot180 ;
		Option < bool, BoolTrueExtractor > rot270 ;

			// penColors(0), maxPenColors(0)
		DriverOptions():
			penplotter(true,"-pen",0, 0, "plotter is pen plotter", 0,false),
			maxPenColors(true,"-pencolors", "number", 0, "number of pen colors available" ,0,0),
			fillinstruction(true,"-filltype", "string", 0, "select fill type e.g. FT 1" ,0,(const char*)"FT1"),
			rot90 (true,"-rot90" ,0, 0, "rotate hpgl by 90 degrees",0,false),
			rot180(true,"-rot180",0, 0, "rotate hpgl by 180 degrees",0,false),
			rot270(true,"-rot270",0, 0, "rotate hpgl by 270 degrees",0,false)
		{
			ADD( penplotter );
			ADD( maxPenColors );
			ADD( fillinstruction );
			ADD( rot90 );
			ADD( rot180 );
			ADD( rot270 );
		}
	}*options;
   
   #include "drvfuncs.h"

      // void show_rectangle(const float llx, const float lly, const float urx, const float ury);
       void show_text(const TextInfo & textInfo);

   private:
	   void print_coords();

	   //  Start DA hpgl color addition
       unsigned int prevColor;
       unsigned int maxPen;
	   unsigned int * penColors;	
       //  End DA hpgl color addition

	   int rotation;

   public:
       static void rot(double & x, double & y, int rotation);

	   NOCOPYANDASSIGN(drvHPGL)
   };

#endif 
 
