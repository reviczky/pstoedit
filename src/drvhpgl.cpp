/* 
   drvHPGL.cpp : This file is part of pstoedit
   HPGL / HPGL2 Device driver supporting text commands

   Copyright (C) 1993 - 2001 Peter Katzmann p.katzmann_AT_thiesen.com
   Copyright (C) 2000 - 2009 Katzmann & Glunz (fill stuff)
   Copyright (C) 2001  Peter Kuhlemann kuhlemannp_AT_genrad.com
   Copyright (C) 2002 - 2003 Peter Kuhlemann peter.kuhlemann_AT_teradyne.com
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

  
	Aug 21, 2001: Donald Arseneau (asnd_AT_triumf.ca) adds pen changes when color changes.
    Changed sections are delimited by:
    //  Start DA hpgl color addition
    //  End DA hpgl color addition

for more info about HPGL see: 
http://lprng.sourceforge.net/DISTRIB/RESOURCES/HPGL_short_summary.html
http://www.multicam.com/main/mat/hpgl.htm
 
http://www.piclist.com/techref/language/hpgl.htm
http://www.piclist.com/techref/language/hpgl/commands.htm
 

*/

#define USESPRINTF 1
#include "drvhpgl.h"
#include <math.h>
#include I_stdio





static const float HPGLScale = 10.0f;



void drvHPGL::rot(double &x, double &y, int angle)
{
	double tmp;
	switch (angle) {
	case 90:
		tmp = x;
		x = -y;
		y = tmp;
		break;
	case 180:
		x = -x;
		y = -y;
		break;
	case 270:
		tmp = x;
		x = y;
		y = -tmp;
		break;
	default:
		break;
	}
}

drvHPGL::derivedConstructor(drvHPGL):
constructBase, 
	//  Start DA hpgl color addition
	prevColor(5555),
	maxPen(0),
	penColors(0)

	
	//  End DA hpgl color addition
{
	// driver specific initializations
	// and writing of header to output file

// 	bool show_usage_line = false;

//NN	savedtextinfo.thetext = "";

	if (options->rot90) rotation = 90; else 
	if (options->rot180) rotation = 180; else 
	if (options->rot270) rotation = 270; else rotation = 0;

//NN	merge = 0;
#if 0
	for (unsigned int i = 0; i < d_argc; i++) {
		assert(d_argv && d_argv[i]);	//lint !e796
		if (Verbose())
			outf << "% " << d_argv[i] << endl;
		if (strcmp(d_argv[i], "-pen") == 0) {
			penplotter = true;
		} else if (strcmp(d_argv[i], "-pencolors") == 0) {
			i++;
			if (i >= d_argc) {
				errf << "-pencolors requires a numeric argument " << endl;
				show_usage_line = true;
			} else {
				maxPenColors = atoi(d_argv[i]);
			}
		} else if (strcmp(d_argv[i], "-fill") == 0) {
			i++;
			if (i >= d_argc) {
				errf << "-fill requires a string argument " << endl;
				show_usage_line = true;
			} else {
				fillinstruction = d_argv[i];
			}
			/*
			   Fill Type (FT) Command 
			   ========================================= 
			   This command selects the shading pattern used to fill polygons ( FP ), rectangles 
			   ( RA or RR ), wedges ( WG ), or characters ( CF ). The Fill Type command ( FT 
			   ), can use solid, shading, parallel lines (hatching), cross hatching, patterned 
			   (raster) fill, or PCL user-defined patterns. For more information see the PCL 5 
			   Printer Language Technical Reference Manual. The syntax for this command is 
			   as follows: 

			   FT fill type,[option1,[option2]]; or FT; 
			 */
// known fill types:
			// FT 1 - solid black
			// FT 3 - parallel lines FT 3[,delta,angle]
			// FT 4 - cross hatching FT 4[,delta,angle]
			// FT 10 - shading FT 10,[percentage]
		} else if (strcmp(d_argv[i], "-rot90") == 0) {
			rotation = 90;
		} else if (strcmp(d_argv[i], "-rot180") == 0) {
			rotation = 180;
		} else if (strcmp(d_argv[i], "-rot270") == 0) {
			rotation = 270;
	//NN	} else if (strcmp(d_argv[i], "-merge") == 0) {
	//NN		merge = 1;
		} else {
			errf << "Unknown fig driver option: " << d_argv[i] << endl;
			show_usage_line = true;
		}
	}

	if (show_usage_line) {
		errf <<
			"Usage -f 'HPGL: [-fill fillstring] [-pen] [-pencolors number] [-rot90 | -rot180 | -rot270] '"
			<< endl;
	}
#endif
//	if (0) {
//		const char esc = (char) 27;
//		outf << esc << ".(;";
//	}
	errf << "Info: This HPGL driver is not very elaborated - consider using -f plot-hpgl instead."
		<< endl;

	outf << "IN;SC;PU;PU;SP1;LT;VS" << (int) HPGLScale << "\n";

	penColors = new unsigned int[options->maxPenColors + 1 + 1];	// 1 offset - 0 is not used // one more for flint ;-)
	for (unsigned int p = 0; p <= (unsigned int) options->maxPenColors + 1; p++) {
		penColors[p] = 0;
	}
	//   float           x_offset;
	//   float           y_offset;
}

drvHPGL::~drvHPGL()
{
	// driver specific deallocations
	// and writing of trailer to output file
	outf << "PU;PA0,0;SP;EC;PG1;EC1;OE\n";
	// fillinstruction = NIL;
	delete [] penColors;
	penColors = NIL;
	options= NIL;
}

void drvHPGL::print_coords()
{
	const unsigned int elems = numberOfElementsInPath();
	if (elems) {
		for (unsigned int n = 0; n < elems; n++) {
			const basedrawingelement & elem = pathElement(n);
			switch (elem.getType()) {
			case moveto:
				{
					const Point & p = elem.getPoint(0);
					double x = (p.x_ + x_offset) * HPGLScale;
					double y = (p.y_ + y_offset) * HPGLScale;
					rot(x, y, rotation);
#if USESPRINTF
					char str[256];
					sprintf_s(TARGETWITHLEN(str,256), "PU%i,%i;", (int) x, (int) y);
					outf << str;
#else
					outf << "PU" << (int) x << "," << (int) y << ";";
#endif
				}
				break;
			case lineto:
				{
					{
						const Point & p = elem.getPoint(0);
						double x = (p.x_ + x_offset) * HPGLScale;
						double y = (p.y_ + y_offset) * HPGLScale;
						rot(x, y, rotation);
#if USESPRINTF
						char str[256];
						sprintf_s(TARGETWITHLEN(str,256), "PD%i,%i;", (int) x, (int) y);
						outf << str;
#else
						outf << "PD" << (int) x << "," << (int) y << ";";
#endif
					}
					if (isPolygon() && (n == elems)) {
						const basedrawingelement & elemnull = pathElement(0);
						const Point & pnull = elemnull.getPoint(0);
						double x = (pnull.x_ + x_offset) * HPGLScale;
						double y = (pnull.y_ + y_offset) * HPGLScale;
						rot(x, y, rotation);
#if USESPRINTF
						char str[256];
						sprintf_s(TARGETWITHLEN(str,256), "PD%i,%i;", (int) x, (int) y);
						outf << str;
#else
						outf << "PD" << (int) x << "," << (int) y << ";";
#endif
					}
				}
				break;
			case closepath:
				{
					const Point & p = pathElement(0).getPoint(0);
					double x = (p.x_ + x_offset) * HPGLScale;
					double y = (p.y_ + y_offset) * HPGLScale;
					rot(x, y, rotation);
#if USESPRINTF
					char str[256];
					sprintf_s(TARGETWITHLEN(str,256), "PD%i,%i;", (int) x, (int) y);
					outf << str;
#else
					outf << "PD" << (int) x << "," << (int) y << ";";
#endif
				}
				break;
			case curveto:
				errf << "\t\tFatal: unexpected case curveto in drvHPGL " << endl;
				abort();
				break;
			default:
				errf << "\t\tFatal: unexpected case default in drvHPGL " << endl;
				abort();
				break;
			}
		}
	}
}


void drvHPGL::open_page()
{
	//  Start DA hpgl color addition
	prevColor = 5555;
	maxPen = 0;
	//  End DA hpgl color addition
	outf << "IN;SC;PU;PU;SP1;LT;VS" << (int) HPGLScale << "\n";
}

void drvHPGL::close_page()
{
	outf << "PU;SP;EC;PG1;EC1;OE\n";
}


#if 0
void drvHPGL::endtext()
{
	if (merge) {
		flush_text(savedtextinfo);
		savedtextinfo.thetext = "";
	}
}
#endif

void drvHPGL::show_text(const TextInfo & textinfo)
{

#if 0
	// this is now handled by drvbase
	if (merge) {
		if (savedtextinfo.thetext == "") {
			savedtextinfo = textinfo;
		} else if ((textinfo.currentFontAngle == savedtextinfo.currentFontAngle)
				   && (textinfo.currentFontSize == savedtextinfo.currentFontSize)
				   && (fabs(textinfo.x - savedtextinfo.x_end) < textinfo.currentFontSize / HPGLScale)
				   && (fabs(textinfo.y - savedtextinfo.y_end) < textinfo.currentFontSize / HPGLScale)) {
			savedtextinfo.thetext += textinfo.thetext;
			savedtextinfo.x_end = textinfo.x_end;
			savedtextinfo.y_end = textinfo.y_end;
		} else {
			flush_text(savedtextinfo);
			savedtextinfo = textinfo;
		}
	} else 
#endif
	const double pi = 3.1415926535;

	const double angleofs = rotation * pi / 180;
	const double dix = 100.0 * cos(textinfo.currentFontAngle * pi / 180.0 + angleofs);
	const double diy = 100.0 * sin(textinfo.currentFontAngle * pi / 180.0 + angleofs);

	double x = (textinfo.x + x_offset) * HPGLScale;
	double y = (textinfo.y + y_offset) * HPGLScale;
	rot(x, y, rotation);

/*
drvhpgl.cpp:316: warning: ISO C++ does not support the `%lg' printf format
drvhpgl.cpp:316: warning: ISO C++ does not support the `%lg' printf format
drvhpgl.cpp:318: warning: ISO C++ does not support the `%lg' printf format
drvhpgl.cpp:318: warning: ISO C++ does not support the `%lg' printf format
*/

#if USESPRINTF
	char str[256];
	sprintf_s(TARGETWITHLEN(str,256), "DI%g,%g;", dix, diy);
	outf << str;
	sprintf_s(TARGETWITHLEN(str,256), "SI%g,%g;", textinfo.currentFontSize / 1000 * HPGLScale, textinfo.currentFontSize / 1000 * HPGLScale);
	outf << str;
	sprintf_s(TARGETWITHLEN(str,256), "PU%i,%i;", (int) x, (int) y);
	outf << str;
#else
	outf << "DI" << dix << "," << diy << ";";
	outf << "SI" << textinfo.currentFontSize / 1000 * HPGLScale << "," << textinfo.currentFontSize / 1000  * HPGLScale << ";";
	outf << "PU" << (int) x << "," << (int) y << ";";
#endif
	outf << "LB" << textinfo.thetext.value() << "\003;" << endl;
}

void drvHPGL::show_path()
{
	//  Start DA hpgl color addition

	/* 
	 *  This block should be a separate subroutine, called by each of
	 *  the hpgl subroutines.  
	 *
	 *  The object is to generate pen switching commands when the color
	 *  changes.  We keep a list of pen colors, which approximate the 
	 *  desired rgb colors.  Choose an existing pen number when the 
	 *  rgb color approximates that color, and add a new color to the
	 *  list when the rgb color is distinctly new.
	 */

	if (numberOfElementsInPath()) {

		if (options->maxPenColors > 0) {
			const unsigned int reducedColor = 256 * (unsigned int) (currentR() * 16) +
				16 * (unsigned int) (currentG() * 16) + (unsigned int) (currentB() * 16);

			if (prevColor != reducedColor) {
				// If color changed, see if color has been used before
				unsigned int npen = 0;
				if (maxPen > 0) {
					for (unsigned int j = 1; j <= maxPen; j++) {	// 0th element is never used - 0 indicates "new" color
						if (penColors[j] == reducedColor) {
							npen = j;
						}
					}
				}
				// If color is new, add it to list, if room
				if (npen == 0) {
					if (maxPen < (unsigned int)options->maxPenColors) {
						maxPen++;
					}
					npen = maxPen;
					//cout << "npen : " << npen << " maxPenColors" << maxPenColors << endl;
					penColors[npen] = reducedColor;
				}
				// Select new pen
				prevColor = reducedColor;
				outf << "PU; \nSP" << npen << ";\n";
			}
			//  End DA hpgl color addition
		}

		switch (currentShowType()) {
		case drvbase::stroke:
			break;
		case drvbase::eofill:
			// Probably Wrong, but treat eoffil like fill
		case drvbase::fill:
			{
				const Point & p = pathElement(0).getPoint(0);
				double x = (p.x_ + x_offset) * HPGLScale;
				double y = (p.y_ + y_offset) * HPGLScale;
				rot(x, y, rotation);
#if USESPRINTF
				char str[256];
				sprintf_s(TARGETWITHLEN(str,256), "PU%i,%i;", (int) x, (int) y);
				outf << str;
#else
				outf << "PU" << (int) x << "," << (int) y << ";";
#endif
				outf << options->fillinstruction.value << ";PM0;";
			}
			break;
		default:				// cannot happen
			errf << "unexpected ShowType " << (int) currentShowType();
			break;
		}
		if (!options->penplotter) {
#if USESPRINTF
			char str[256];
			sprintf_s(TARGETWITHLEN(str,256), "PW%lg;", currentLineWidth());
			outf << str;
#else
			outf << "PW" << currentLineWidth() << ";";
#endif
		}

		print_coords();

		switch (currentShowType()) {	// To check which endsequenz we need
		case drvbase::stroke:
			break;
		case drvbase::eofill:
			// Probably Wrong, but treat eoffil like fill
		case drvbase::fill:
			outf << "PM2;FP;EP;";	// EP also draws path
			break;
		default:				// cannot happen
			outf << "unexpected ShowType " << (int) currentShowType();
			break;
		}
		outf << endl;
	}
}

#if 0
// not needed anymore - at least not as long the default is acceptable.
void drvHPGL::show_rectangle(const float llx, const float lly, const float urx, const float ury)
{
	unused(&llx);
	unused(&lly);
	unused(&urx);
	unused(&ury);

	// just do show_path for a first guess
	show_path();
}
#endif

static DriverDescriptionT < drvHPGL > D_HPGL("hpgl", "HPGL code", "","hpgl", false,	// backend supports subpathes
												 // if subpathes are supported, the backend must deal with
												 // sequences of the following form
												 // moveto (start of subpath)
												 // lineto (a line segment)
												 // lineto 
												 // moveto (start of a new subpath)
												 // lineto (a line segment)
												 // lineto 
												 //
												 // If this argument is set to false each subpath is drawn 
												 // individually which might not necessarily represent
												 // the original drawing.
												 false,	// backend supports curves
												 false,	// backend supports elements which are filled and have edges
												 true,	// backend supports text
												 DriverDescription::noimage,	// no support for PNG file images
												 DriverDescription::normalopen, false,	// backend support multiple pages
												 false /*clipping */ );
