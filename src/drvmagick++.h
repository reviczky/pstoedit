#ifndef __drvMAGICK_h
#define __drvMAGICK_h

/* 
   drvMAGICK.h : This file is part of pstoedit
   Class declaration for a MAGICK++ output driver with no additional attributes
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
// wogltest #define _IOSFWD_

#include "drvbase.h"
#ifdef _WIN32
// Magic++ needs WIN32
#ifndef WIN32
#define WIN32
#endif
#endif
#include <Magick++.h>
using namespace std;
using namespace Magick;

class drvMAGICK : public drvbase {

public:

	derivedConstructor(drvMAGICK);

	~drvMAGICK(); // Destructor
	class DriverOptions : public ProgramOptions {
	public:
		DriverOptions() 
		{
		}
	}*options;

#include "drvfuncs.h"
//	void show_rectangle(const float llx, const float lly, const float urx, const float ury);
	void show_text(const TextInfo & textInfo);

public:

	virtual void    show_image(const PSImage & imageinfo); 

private:
	void create_vpath(VPathList & vpath);
	int	      imgcount;

	Image * imageptr; //Magick::Image meant here
	
};

#endif
 
 
