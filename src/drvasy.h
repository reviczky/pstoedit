#ifndef __drvASY_h
#define __drvASY_h

/* 
   drvasy.h : This file is part of pstoedit
   Backend for Asymptote files
   Contributed by: John Bowman

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


#ifdef HAVESTL
#include <string>        // C++ string class
#include <list>
#else
typedef RSString string;
#endif

class drvASY : public drvbase {

public:

  derivedConstructor(drvASY);

  ~drvASY(); // Destructor
  class DriverOptions : public ProgramOptions {
  public:
    DriverOptions() {}
  }*options;

#include "drvfuncs.h"
  void show_text(const TextInfo & textInfo);
  void show_image(const PSImage & imageinfo);
  
  void ClipPath(cliptype);
  void Save();
  void Restore();

private:
  void print_coords();
  void save();
  void restore();
  // Previous values of graphics state variables
  string prevFontName;
  string prevFontWeight;
  float prevR, prevG, prevB;
  float prevFontAngle;
  float prevFontSize;
  float prevLineWidth;
  unsigned int prevLineCap;
  unsigned int prevLineJoin;
  string prevDashPattern;

  // Set to true for fill or eofill, false for stroke
  bool fillmode;
  // Set to true for clip or eoclip, false otherwise
  bool clipmode;
  // Set to true for eofill or eoclip, false otherwise
  bool evenoddmode;
  // Set to true on first page, false otherwise
  bool firstpage;
  
  int imgcount;
  
  unsigned int level;
  // gsave nesting level
  
  std::list<bool> clipstack;
  std::list<bool> gsavestack;
};

#endif
