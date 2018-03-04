#ifndef CBSTREAM_H
#define CBSTREAM_H
/* 

   callbackBuffer : This file is part of pstoedit
   streambuf that writes the data to a user defineable call back function

   Copyright (C) 1998 - 2009 Wolfgang Glunz, wglunz35_AT_pstoedit.net

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


This code is mainly based on the example given in:

The GNU C++ Iostream Library
	Using the streambuf Layer 
		Simple output re-direction by redefining overflow 


*/

#ifndef cppcomp_h
#include "cppcomp.h"
#endif

#include I_iostream
#include I_string_h
#include I_streamb
USESTD

#if !defined(HAVESTL) && !defined(__GNUG__)
// this should be defined for all ANSI compilers in iostream
// but this is also already defined in old GNU compilers
typedef int streamsize; // oder long ?? MSVC likes int
#endif
#if defined(_WIN32) || defined(__OS2__)
typedef int (__stdcall write_callback_type) (void * cb_data, const char* text, int length);
// length is int and not unsigned long because of gs-api
#else
typedef int ( write_callback_type) (void * cb_data, const char* text, int length);
#endif
//lint !e1712 // no default ctor
class callbackBuffer : public streambuf {
public:
	callbackBuffer(void * cb_data_p, write_callback_type* wcb) : 
		cb_data(cb_data_p), write_callback(wcb) {}
	write_callback_type * set_callback(void * cb_data_p,write_callback_type* new_cb);
	int write_to_callback(const char* text, int length);

protected:
    int sync();
    int overflow(int ch);
	int underflow(void);

    // Defining xsputn is an optional optimization.
    // (streamsize was recently added to ANSI C++, not portable yet.)
    streamsize xsputn(const char* text, streamsize n);

private:
	
	void * cb_data;
	write_callback_type * write_callback;
	callbackBuffer(const callbackBuffer &); // not defined
	const callbackBuffer & operator = (const callbackBuffer &); // not defined
};

#endif
 
