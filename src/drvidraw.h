#ifndef __drvIDRAW_h
#define __drvIDRAW_h

/* 
   drvidraw.h : This file is part of pstoedit
   Backend for idraw files
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


enum {IDRAW_NUMCOLORS=12};

class drvIDRAW : public drvbase {

public:
  derivedConstructor(drvIDRAW);
  //(const char * driveroptions_P,ostream & theoutStream,ostream & theerrStream ); // Constructor

  ~drvIDRAW(); // Destructor
	class DriverOptions : public ProgramOptions {
	public:
		DriverOptions() 
		{
		}
	}*options;

void show_image(const PSImage & imageinfo);

private:
	void print_coords();

  static const char *psfont2xlfd(const char *); // Map PS --> XLFD font name
  float IDRAW_SCALING;
  const char *rgb2name(float, float, float) const ;// Map RGB --> color name
  void print_header(const char *);    // Output idraw header
  struct {                            // Map of RGB to color name
    double red, green, blue;
    const char *name;
  } color[IDRAW_NUMCOLORS];

  // Scale a PostScript value to an idraw value
#if 0
  inline const float iscale(float invalue) const { return invalue; }
#else
  inline const unsigned int iscale(float invalue) const {
     return (unsigned int) (invalue/IDRAW_SCALING + 0.5);
  }
#endif
private:

     //  int           objectId;
       TempFile      tempFile;
       ofstream      &buffer;
       int           imgcount;



#include "drvfuncs.h"
	   void show_text(const TextInfo & textInfo);

	
};

#endif
 
 
 
