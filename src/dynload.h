/*
   dynload.h : This file is part of pstoedit
   declarations for dynamic loading of drivers

   Copyright (C) 1993 - 2014 Wolfgang Glunz, wglunz35_AT_pstoedit.net

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

#include "cppcomp.h"

#include I_ostream 
#include I_istream
#include I_iostream

class DLLEXPORT DynLoader 
{
public:
	typedef void (*fptr)(); // any signature possible, but is has to be a function pointer.
							// the new standard does not allow to mix normal and function pointers.
	static fptr ptr_to_fptr(void *); // need a function since C++ does not allow a simple cast.
	DynLoader(const char * libname_P = 0, std::ostream & errstream_p = std::cerr, int verbose_p = 0);
	~DynLoader();
	void open(const char * libname);
	void close();
	fptr getSymbol(const char * name, int check = 1) const;
	int  knownSymbol(const char * name) const;
	int  valid() const { return handle != 0; }
	const void * gethandle() const { return handle; }
private:
	char * libname;
	void* handle;	
	std::ostream & errstream;
	int verbose;

	NOCOPYANDASSIGN(DynLoader)
};

#ifdef HAVE_STL
#include <iosfwd>
using namespace std;
#else
class ostream;
#endif
#ifndef LEANDYNLOAD
// lean - just the dlopen/dlclose stuff
void loadPlugInDrivers(const char * pluginDir,ostream& errstream,bool verbose);
#endif 
 
 
 
