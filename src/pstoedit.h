#ifndef PSTOEDIT_H
#define PSTOEDIT_H
/*
   pstoedit.h : This file is part of pstoedit
   main control procedure

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

#ifdef __cplusplus
/* for C++ users more functions are available than for C users */ 


#ifndef cppcomp_h
#include "cppcomp.h"
#endif

/* #include <iostream.h>   */
#ifdef HAVE_STL
#include <iosfwd>
USESTD
#else
#if defined (__GNUG__)  && (__GNUC__>=3) 
/* if we dont define HAVE_STL for g++ > 3.0, then we can use the simple forwards */
	#include <iostream.h>
#else
	class istream;
	class ostream;
#endif
#endif

/* end of cplusplus */
#endif

#include "pstoedll.h"

typedef int (*execute_interpreter_function)(int argc, const char * const argv[]);



#ifdef __cplusplus
typedef const char * (*whichPI_type)(ostream &, int, const char *, const char *);

class DescriptionRegister;

/* for "C" functions we cannot use "bool"   */

extern "C" DLLEXPORT
int pstoedit(	int argc,
				const char * const argv[],
				ostream& errstream,
  				execute_interpreter_function call_PI,
				whichPI_type whichPI,
				const DescriptionRegister* const pushinsPtr =0
			);

/*
	sets either cout (true) or errstream is used
*/
extern "C" DLLEXPORT
void useCoutForDiag(int flag); 


extern "C" DLLEXPORT 
int pstoeditwithghostscript(int argc,
							const char * const argv[],
							ostream& errstream,
							const DescriptionRegister* const pushinsPtr=0
							);

#endif

#ifdef __cplusplus
extern "C" DLLEXPORT 
#endif
int pstoedit_plainC(int argc,
                    const char * const argv[],
                    const char * const psinterpreter  /* if 0, then pstoedit will look for one using whichpi() */
                   );

/* extern "C" DLLEXPORT const class DriverDescription* const * getPstoeditDriverInfo(ostream& errstream); */
#ifdef __cplusplus
extern "C" DLLEXPORT 
#endif
struct DriverDescription_S* getPstoeditDriverInfo_plainC(void);
//
// function to clear the memory allocated by the getPstoeditDriverInfo_plainC
// function. This avoids conflicts with different heaps.
//
#ifdef __cplusplus
extern "C" DLLEXPORT 
#endif
void clearPstoeditDriverInfo_plainC(struct DriverDescription_S * ptr);


#ifdef __cplusplus
extern "C" DLLEXPORT 
#endif
struct DriverDescription_S* getPstoeditNativeDriverInfo_plainC(void); /* for the pstoedit native drivers - not the ones that are provided as short cuts to Ghostscript */


#ifdef __cplusplus
extern "C" DLLEXPORT 
#endif
int pstoedit_checkversion (unsigned int callersversion);
									 
void ignoreVersionCheck(void); /* not exported to the DLL interface, just used internally */


#ifdef __cplusplus

#if defined(_WIN32) || defined(__OS2__)
#include "cbstream.h"
extern "C" DLLEXPORT void setPstoeditOutputFunction(void * cbData,write_callback_type* cbFunction);

//
// under Windows we need to be able to switch between two modes of calling GS - 1 via DLL and 2 with EXE
// the EXE is needed when being called from gsview - whereas in all other cases the DLL is the better way
// So the useDLL is set to false - and the pstoedit stand-alone program sets it to true
// gsview uses the default which is false - hence the ghostscript is called via its exe
// 
// need to use int instead of bool because of C mode
extern "C" DLLEXPORT void setPstoeditsetDLLUsage( int useDLL_p);
extern "C" DLLEXPORT int getPstoeditsetDLLUsage();


#endif

#endif


#endif

 
 
 
