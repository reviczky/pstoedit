/*
   cmdmain.cpp : This file is part of pstoedit
   main program for command line usage

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
#include "cppcomp.h"
#include "pstoedit.h"

#include I_iostream

int main(int argc, const char *const argv[])
{
#if defined(_WIN32) || defined (__OS2__)
	if (!pstoedit_checkversion(301))
		return 1;
	if (!getenv("PSTOEDITUSEGSEXE")) setPstoeditsetDLLUsage(true); // use the gs DLL in stand-alone mode
#else
	ignoreVersionCheck();
#endif
	// on UNIX like systems, we can use cerr as error stream
	// except for some help/diag messages which are preferred on cout 
	useCoutForDiag(true);
	const int result = pstoeditwithghostscript(argc, argv, cerr);
	return result;
}
