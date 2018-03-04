#ifndef __drvwmf_h__
#define __drvwmf_h__

/*
   drvwmf.h : This file is part of pstoedit
   Class declaration for the WMF output driver.
   The implementation can be found in drvwmf.cpp

   Copyright (C) 1998 Thorsten Behrens and Bjoern Petersen
   Copyright (C) 2000 Thorsten Behrens

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


#ifdef _WIN32

#include <windows.h>
#undef min		// is declared inline in drvbase... (sigh)
#undef max

#else

// use Allen Barnett's libemf
#ifdef OLDLIBEMF
#include <emf.h>
#else
#include <libEMF/emf.h>
#endif

#endif

#include "drvbase.h"


class drvWMF : public drvbase {

public:

	derivedConstructor(drvWMF);		// Constructor

	~drvWMF();						// Destructor
	class DriverOptions : public ProgramOptions { 
	public:
		Option < bool, BoolTrueExtractor> mapToArial;
		Option < bool, BoolTrueExtractor> emulateNarrowFonts;
		Option < bool, BoolTrueExtractor> drawBoundingBox;
		Option < bool, BoolTrueExtractor> pruneLineEnds;
		Option < bool, BoolTrueExtractor> notforWindows;
		Option < bool, BoolTrueExtractor> winbb;
		Option < bool, BoolTrueExtractor> OpenOfficeMode ;

		DriverOptions() :
		mapToArial(true,"-m",0,0,"map to Arial",0,false),
		emulateNarrowFonts(true,"-nf",0,0,"emulate narrow fonts",0,false),
		drawBoundingBox(true,"-drawbb",0,0,"draw bounding box",0,false),
		pruneLineEnds(true,"-p",0,0,"prune line ends",0,false),
		notforWindows(true,"-nfw",0,0,"not for Windows (meaningful under *nix only)",
		"Newer versions of Windows (2000, XP, Vista) will not accept WMF/EMF files generated when this option is set and the input contains Text. "
		"But if this option is not set, then the WMF/EMF driver will estimate interletter spacing of text using "
		"a very coarse heuristic. This may result in ugly looking output. On the other hand, OpenOffice "
		"can still read EMF/WMF files where pstoedit delegates the calculation of the inter letter spacing "
		"to the program reading the WMF/EMF file. So if the generated WMF/EMF file shall never be processed "
		"under Windows, use this option. If WMF/EMF files with high precision text need to be generated under *nix "
		"the only option is to use the -pta option of pstoedit. However that causes every text to be split into single characters "
		"which makes the text hard to edit afterwards. Hence the -nfw options provides a sort of compromise between "
		"portability and nice to edit but still nice looking text. Again - this option has no meaning when pstoedit "
		"is executed under Windows anyway. In that case the output is portable "
		"but nevertheless not split and still looks fine.", false),
		winbb(true,"-winbb",0,0,"let the Windows API calculate the Bounding Box (Windows only)",0,false),
		OpenOfficeMode(true,"-OO", 0, 0, "generate OpenOffice compatible EMF file",0,false)

		{
			ADD(mapToArial);
			ADD(emulateNarrowFonts);
			ADD(drawBoundingBox);
			ADD(pruneLineEnds);
			ADD(notforWindows);
			ADD(winbb);
			ADD(OpenOfficeMode);
		}
	
	} * options;

#include "drvfuncs.h"
	void show_rectangle(const float llx, const float lly, const float urx, const float ury);
	void show_text(const TextInfo & textInfo);

	virtual void show_image(const PSImage & imageinfo); 

private:

	enum polyType {TYPE_FILL,TYPE_LINES};

	void	drawPoly			(POINT*, int*, polyType type);

	void	setDrawAttr			();

	int		fetchFont			(const TextInfo & textinfo, short int, short int);

	float scale() const;
	long transx(float x) const;
	long transy(float x) const;

	void initMetaDC(HDC hdc);

// This contains all private data of drvwmf.

	HDC				metaDC;
	HDC				desktopDC;

	LOGPEN			penData;
	HPEN			coloredPen;
	HPEN			oldColoredPen;

	LOGBRUSH		brushData;
	HBRUSH			coloredBrush;
	HBRUSH			oldColoredBrush;

	HFONT			myFont;
	HFONT			oldFont;

	long			maxX, maxY;
	long			minX, minY;
	long			maxStatus, minStatus;

	bool			enhanced;	

	RSString		tempName;
	FILE*			outFile;	

};

#endif /* #ifndef __drvwmf_h__ */
 
 

 
 
