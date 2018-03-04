#ifndef __drvPIC_h
#define __drvPIC_h

/* 
   drvpic.h : This file is part of pstoedit
   Class declaration for PIC output driver

   Copyright (C) 1999 Egil Kvaleberg, egil@kvaleberg.no

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

class drvPIC : public drvbase {

public:
	 derivedConstructor(drvPIC);
	~drvPIC(); // Destructor
	class DriverOptions : public ProgramOptions {
	public:
		// Option < bool, BoolTrueExtractor > groff;
		Option < bool, BoolTrueExtractor > troff_mode;
		Option < bool, BoolTrueExtractor > landscape;
		Option < bool, BoolTrueExtractor > portrait;
		Option < bool, BoolTrueExtractor > keepFont;
		Option < bool, BoolTrueExtractor > textAsText;
		Option < bool, BoolTrueExtractor > debug;

		DriverOptions() : 
			// groff(true,"-groff","",0,"groff mode",0,false),
			troff_mode(true,"-troff","",0,"troff mode (default is groff)",0,false),
			landscape(true,"-landscape","",0,"landscape output",0,false),
			portrait(true,"-portrait","",0,"portrait output",0,false),
			keepFont(true,"-keepfont","",0,"print unrecognized literally",0,false),
			textAsText(true,"-text","",0,"try not to make pictures from running text",0,false),
			debug(true,"-debug","",0,"enable debug output",0,false)
		{
			// ADD(groff);
			ADD(troff_mode);
			ADD(landscape);
			ADD(portrait);
			ADD(keepFont);
			ADD(textAsText);
			ADD(debug);
		}
	} *options;

#include "drvfuncs.h"
	void show_text(const TextInfo & textInfo);

private:
	void print_coords();
	float x_coord(float x, float y) const ;
	float y_coord(float x, float y) const ;
	void ps_begin();
	void ps_end();

	/*
	int troff_mode;         // troff, as opposed to groff
	int landscape;          // original is in landscape mode
	int keep_font;          // print unrecognized literally
	int text_as_text;       // try not to make pictures from running text
	int debug;              // obvious...
	*/

	float largest_y;
	float pageheight;
	int withinPS;
};

#endif
 
 
