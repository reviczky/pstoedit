/*
   pstoeditoutputlib.h : 

   Copyright (C) 2002 - 2014 Wolfgang Glunz, wglunz35_AT_pstoedit.net

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

/* 
   the struct version of DriverDescription 
   this is needed in order to allow plain old C programs to use the .dll
*/

#include "drvbase.h"

class DLLEXPORT PstoeditOutputInterface {
public:

	PstoeditOutputInterface();
	~PstoeditOutputInterface();

	drvbase * getdriver(const char * const drivername);

private: 
	class PstoeditLibraryPrivate * privatedata;

	// disallowed - not implemented
	PstoeditOutputInterface(const PstoeditOutputInterface&);
	const PstoeditOutputInterface& operator=(const PstoeditOutputInterface&);
};
