/* 
   drvbase.cpp : This file is part of pstoedit
   Basic, driver independent output routines

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

#include I_stdlib
#include I_iostream
//#include I_iomanip

#include I_string_h

#include I_strstream

#include <math.h>

#ifndef miscutil_h
#include "miscutil.h"
#endif

static void splitFullFileName(const char *const fullName, 
							  RSString& pathName, 
							  RSString& baseName, 
							  RSString& fileExt)
{
	if (fullName == NIL)
		return;

	char *fullName_T = cppstrdup(fullName);
	char *baseName_T = 0;

#if defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined(__EMX__) || defined (NetBSD) 
	char *c = strrchr(fullName_T, '/');
#else
	char *c = strrchr(fullName_T, '\\');
#endif
	if (c != NIL) {
		baseName_T = cppstrdup(c + 1);
		*(c + 1) = 0;
		pathName = fullName_T;
	} else {
		baseName_T = cppstrdup(fullName_T);
		pathName = "";
	}

	c = strrchr(baseName_T, '.');
	if (c != NIL) {
		fileExt =  (c + 1);
		*c = 0;
		baseName = baseName_T;
	} else {
		fileExt = "";
		baseName = baseName_T;
	}
	delete[]baseName_T;
	delete[]fullName_T;
}




drvbase::drvbase(const char *driveroptions_p, ostream & theoutStream,
				 ostream & theerrStream,
				 const char *nameOfInputFile_p,
				 const char *nameOfOutputFile_p,
				 const PsToEditOptions & globaloptions_p, 
				 const DriverDescription * Pdriverdesc_p)
:								// constructor
Pdriverdesc(Pdriverdesc_p), 
DOptions_ptr(Pdriverdesc_p->createDriverOptions()),
//  totalNumberOfPages(0),
//  bboxes(0),
	outf(theoutStream),
errf(theerrStream),
inFileName(nameOfInputFile_p),
outFileName(nameOfOutputFile_p), 
outDirName(""), outBaseName(""), d_argc(0), d_argv(0), globaloptions(globaloptions_p),
	// set some common defaults
currentDeviceHeight(792.0f ),  // US Letter
currentDeviceWidth(612.0f ),   // US Letter
x_offset(0.0f),
y_offset(0.0f),
currentPageNumber(0),
domerge(false),
defaultFontName(0),
ctorOK(true),
saveRestoreInfo(NIL), currentSaveLevel(&saveRestoreInfo), page_empty(true), driveroptions(0),
	// default for PI1 and PI2 and clippath
	currentPath(0), last_currentPath(0), outputPath(0), lastPath(0)
	// default for textInfo_ and lasttextInfo_
{

	

	// verbose = (getenv("PSTOEDITVERBOSE") != 0);

	if (verbose) {
		errf << "verbose mode turned on\n" << endl;
	}

	if (nameOfOutputFile_p) {
		RSString extension; // not needed
		splitFullFileName(nameOfOutputFile_p, outDirName, outBaseName, extension );
		if (verbose) {
			errf << "nameofOutputFile: '" << nameOfOutputFile_p;
			errf << "' outDirName: '" << outDirName;
			errf << "' outBaseName: '" << outBaseName;
			errf << "'" << endl;
		}
	}
	// preparse driveroptions and build d_argc and d_argv
	if (driveroptions_p) {
#if 1
		Argv driverargs;
		driverargs.parseFromString(driveroptions_p);
		d_argc = driverargs.argc;
		d_argv = new const char *[d_argc + 2];  // 1 more for the argv[0]
		d_argv[0] = cppstrdup(Pdriverdesc_p->symbolicname);
		d_argc = 1;
		for (unsigned int i = 0; i < driverargs.argc; i++) {
			d_argv[d_argc] = cppstrdup(driverargs.argv[i]);
			d_argc++;
		}
		d_argv[d_argc] = 0;
#else
		driveroptions = cppstrdup(driveroptions_p);
		//C_istrstream optstream(driveroptions, strlen(driveroptions));
		C_istrstream optstream(driveroptions); //, strlen(driveroptions));
		const long startOfStream = optstream.tellg();
		char currentarg[100];
		// first count number of arguments
		while (!optstream.eof()) {
			(void) optstream.width(sizeof(currentarg));
			optstream >> currentarg;
			d_argc++;
		}
		d_argv = new const char *[d_argc + 2];  // 1 more for the argv[0]
		// now fill d_args array;
		(void) optstream.seekg(startOfStream);	// reposition to start
		optstream.clear();
		// fill argv[0] with driver name (to be similar with Unix)
		d_argv[0] = cppstrdup(Pdriverdesc_p->symbolicname);
		d_argc = 1;
		while (!optstream.eof()) {
			optstream >> currentarg;
			if (strlen(currentarg) > 0) {
				d_argv[d_argc] = cppstrdup(currentarg);
				d_argc++;
			}
		}
		d_argv[d_argc] = 0;
#endif
		if (verbose) {
			errf << "got " << d_argc << " driver argument(s)" << endl;
			for (unsigned int i = 0; i < d_argc; i++) {
				errf << "Driver option " << i << ":" << d_argv[i] << endl;
			}
		}
	}

// now call the driver specific option parser.
	if (d_argc>0) {
		if (DOptions_ptr) {
			//debug errf << "DOptions_ptr: " << (void*) DOptions_ptr << endl;
			const unsigned int remaining = DOptions_ptr->parseoptions(errf,d_argc,d_argv);
			if (remaining > 0) {
				errf << "the following " << remaining  << " options could not be handled by the driver: " << endl;
				for (unsigned int i = 0; i < remaining; i++) {
					errf << DOptions_ptr->unhandledOptions[i] << endl;
				}
			}
		} else {
			cerr << "DOptions_ptr is NIL - program flow error - contact author." << endl;
		}
	}

//  bboxes = new BBox[maxPages];

	// init segment info for first segment
	// all others will be updated with each newsegment


	currentPath = &PI1;
	lastPath = &PI2;
	outputPath = currentPath;

	if ((PI1.path == 0) || (PI2.path == 0) || (clippath.path == 0)) {
		errf << "new failed in drvbase::drvbase " << endl;
		exit(1);
	}

	textInfo_.thetext.copy("");
	setCurrentFontName("Courier", true);
	setCurrentFontFamilyName("Courier");
	setCurrentFontWeight("Regular");
	setCurrentFontFullName("Courier");
	setCurrentFontSize(10.0f);
	mergedTextInfo = textInfo_; // initial value - empty buffer
	lastTextInfo_ = textInfo_;
	lastTextInfo_.currentFontSize = -textInfo_.currentFontSize;	// to force a new font the first time.
	lastTextInfo_.currentR = textInfo_.currentR + 1;	// to force new color
}

drvbase::~drvbase()
{
	currentPath = 0;
	lastPath = 0;
	outputPath = 0;
	if (d_argv) {
		for (unsigned int i = 0; i < d_argc; i++) {
			delete[](d_argv[i]);
			d_argv[i] = 0;
		}
		delete[]d_argv;
		d_argv = NIL;
	}
	if (driveroptions) {
		delete[]driveroptions;
		driveroptions = NIL;
	}
//  delete[] bboxes; bboxes = NIL;
//	delete[]outDirName;
//	outDirName = NIL;
//	delete[]outBaseName;
//	outBaseName = NIL;
	Pdriverdesc = NIL;

	delete DOptions_ptr;
	DOptions_ptr = NIL;

	if (currentSaveLevel->previous != NIL) {
		while (currentSaveLevel->previous != NIL) {
			currentSaveLevel = currentSaveLevel->previous;
			delete currentSaveLevel->next;
		}
	}
	currentSaveLevel = 0;
}

const RSString & drvbase::getPageSize() const { return globaloptions.outputPageSize(); }

const BBox & drvbase::getCurrentBBox() const
{
	if ( verbose )
		cout << " get getCurrentBBox for page: " << currentPageNumber <<
			" of " << totalNumberOfPages << endl;
	if ((totalNumberOfPages > 0)
		&& (currentPageNumber <= totalNumberOfPages)) {
		// page numbers start from 1.
		return bboxes()[currentPageNumber > 0 ? (currentPageNumber - 1) : 0];
	} else {
		static BBox dummy;
		return dummy;
	}
}

void drvbase::startup(bool mergelines)
{
	domerge = false;			// default
	if (mergelines) {
		if (Pdriverdesc->backendSupportsMerging) {
			domerge = true;
		} else {
			errf << "the selected backend does not support merging, -mergelines ignored" << endl;
		}
	}
}

void drvbase::finalize()
{
// needed because base destructor is called after derived destructor
	outputPath->clear();		// define past the end path as empty
	// close page (if no explicit showpage was done)
	showpage();
}

void drvbase::showpage()
{
	flushOutStanding();					// dump last path 
	if (!page_empty) {
		close_page();
	}
	page_empty = true;
}

bool drvbase::pathsCanBeMerged(const PathInfo & path1, const PathInfo & path2) const
{
	//
	// two paths can be merged if one of them is a stroke and the
	// other a fill or eofill AND
	// all pathelements are the same
	//
	// This is a default implementation which allows only solid edges since
	// most backends support only such edges.
	// If a backend allows more, it can overwrite this function
	// 
	if (((path1.currentShowType == stroke && path1.currentLineType == solid
		  && ((path2.currentShowType == fill)
			  || (path2.currentShowType == eofill)))
		 || (path2.currentShowType == stroke
			 && path2.currentLineType == solid && ((path1.currentShowType == fill)
												   || (path1.currentShowType == eofill))))
		&& (path1.numberOfElementsInPath == path2.numberOfElementsInPath)) {
		//errf << "Pathes seem to be mergeable" << endl;
		for (unsigned int i = 0; i < path1.numberOfElementsInPath; i++) {
			const basedrawingelement *bd1 = path1.path[i];
			const basedrawingelement *bd2 = path2.path[i];
//          if (! *(path1.path[i]) == *(path2.path[i]) ) return 0;
			//errf << "comparing " << *bd1 << " with " << *bd2 << endl;
			const bool result = (*bd1 == *bd2);
			if (verbose)
				errf << "comparing " << *bd1 << " with " << *bd2 <<	" results in " << (int) result << endl;
			if (!result)
				return false;
		}
		if (verbose)
			errf << "Pathes are mergeable" << endl;
		return true;
	} else {
		if (verbose)
			errf << "Pathes are not mergable:" <<
				" PI1 st " << (int) path1.currentShowType <<
				" PI1 lt " << (int) path1.currentLineType <<
				" PI1 el " << path1.numberOfElementsInPath <<
				" PI2 st " << (int) path2.currentShowType <<
				" PI2 lt " << (int) path2.currentLineType <<
				" PI2 el " << path2.numberOfElementsInPath << endl;
		return false;
	}
}



const basedrawingelement & drvbase::pathElement(unsigned int index) const
{
	return *(outputPath->path[index + outputPath->subpathoffset]);
}

bool basedrawingelement::operator == (const basedrawingelement & bd2) const
{
	if (this->getType() != bd2.getType()) {
		return false;
	} else {
		for (unsigned int i = 0; i < this->getNrOfPoints(); i++) {
			if (!(this->getPoint(i) == bd2.getPoint(i)))
				return false;
		}
	}
	return true;
}

bool drvbase::textIsWorthToPrint(const RSString& thetext) const
{
	// check whether it contains just blanks. This makes
	// problems, e.g. with the xfig backend.
  const char *cp = thetext.value();
  for (size_t i = thetext.length(); i>0; i--)
    if (*cp++ != ' ')
      return true;
  return false;
}

bool drvbase::textCanBeMerged(const TextInfo & text1, const TextInfo & text2) const
{
	return (
				(text1.currentFontName == text2.currentFontName)
			 && (text1.currentFontFamilyName  == text2.currentFontFamilyName)
			 && (text1.currentFontFullName  == text2.currentFontFullName)
			 && (text1.currentFontWeight  == text2.currentFontWeight)
			 && (text1.currentFontSize  == text2.currentFontSize)
			 && (text1.currentFontAngle  == text2.currentFontAngle)
			 && (text1.currentR  == text2.currentR)
			 && (text1.currentG  == text2.currentG)
			 && (text1.currentB  == text2.currentB)

			 && (fabs(text1.x - text2.x_end) < text1.currentFontSize / 10)
			 && (fabs(text1.y - text2.y_end) < text1.currentFontSize / 10)

			);

	// text matrix is ignored for the moment
}

void drvbase::show_text(const TextInfo & textinfo) 
{
		unused(&textinfo);
		if (Pdriverdesc->backendSupportsText) {
			errf << " Backends that support text need to define a show_text method " <<endl;
		}
		// in case backendSupportsText is false, the frontend already flattens text (usually)
		// Must use the -dt flag for this, since RenderMan doesn't support text
}

void drvbase::show_rectangle(
				       const float llx,
				       const float lly,
				       const float urx,
				       const float ury) 
	// writes a rectangle at points (llx,lly) (urx,ury)
{
	// outf << "Rectangle ( " << llx << "," << lly << ") (" << urx << "," << ury << ")" << endl;
	// just do show_path for a first guess

	if (globaloptions.convertFilledRectToStroke && (currentShowType() == drvbase::fill || currentShowType() == drvbase::eofill)) {
// if possible and wished - convert a filled rectangle to a single stroked line

		const float dx = urx - llx;
		const float dy = ury - lly;
		const float lw = currentLineWidth();
		const float lwhalf = lw/2.0f;

		PathInfo * savepath = currentPath;
		currentPath = outputPath; // in order to be able to use the add.. functions
		// we have to use outputPath-> instead of currentpath

		setCurrentShowType(drvbase::stroke);
		setCurrentLineCap(0); // 0 means "butt", i.e. no overlap
		setCurrentLineType(drvbase::solid); 

		if (dx > dy) {
			// horizontal line
			const float mid = (ury+lly)/2.0f;
	 		currentPath->clear();
	 		addtopath(new Moveto(llx-lwhalf,mid));
	 		addtopath(new Lineto(urx+lwhalf,mid));
	 		setCurrentLineWidth( dy+lw );
			// debug cout << "rect -> horizontal line " << endl;
		} else {
			// vertical line
			const float mid = (urx+llx)/2.0f;
			currentPath->clear();
			addtopath(new Moveto(mid,lly+lwhalf));
			addtopath(new Lineto(mid,ury+lwhalf));
			setCurrentLineWidth( dx+lw );
			// debug cout << "rect -> vertical line " << endl;
		}
		currentPath = savepath; 
	} else {
		// default - just write the rect as an ordinary polygon

		// debug cout << "rect as path " << endl;
	}	

	show_path(); 
}

void drvbase::flushTextBuffer(bool useMergeBuffer)
{
	if (useMergeBuffer) textInfo_ = mergedTextInfo; // this is ugly, I know, but to be consistent
								// with other functions that use textInfo_ directly
								// this is needed.
	const TextInfo* textToBeFlushed = useMergeBuffer ? &mergedTextInfo : &textInfo_;
	add_to_page();
	show_text(*textToBeFlushed);	
	lastTextInfo_ = *textToBeFlushed;	// save for font and color comparison
}

void drvbase::showOrMergeText()
{
	flushOutStanding(flushpath); // dump last path to avoid wrong sequence of text and graphics
	// this flushing needs to be done in any case, even if the text is not written immediately
	// but instead buffered first. But otherwise, the order gets corrupted

	if (globaloptions.mergetext) {
		if (mergedTextInfo.thetext == "") {
			mergedTextInfo = textInfo_;
			// there was nothing in the buffer so far, so just place it there.
			// for this we need a final flush somewhere
		} else if (textCanBeMerged(textInfo_,mergedTextInfo)) {
		   // text can be merged.
			if (verbose) {
				errf << "Info: merging text '" << mergedTextInfo.thetext 
					<< "' and '"
					<< textInfo_.thetext << "'" << endl;
			}
			mergedTextInfo.thetext += textInfo_.thetext;
			static const RSString space(" ");
			(mergedTextInfo.glyphnames += space ) += textInfo_.glyphnames;
			mergedTextInfo.x_end = textInfo_.x_end;
			mergedTextInfo.y_end = textInfo_.y_end;
		} else {
			// cannot be merged, so dump text collected so far and place the new
			// one in the buffer for later
			if (textIsWorthToPrint(mergedTextInfo.thetext.value())) {
				TextInfo temp = textInfo_;	// save "new" text in temp
				flushTextBuffer(true); // true -> use merge buffer
				mergedTextInfo = temp;		// set the merge buffer to the "new" text
			} else {
				// the merge buffer was not worth to be printed so forget it and 
				// start over with new text
				mergedTextInfo = textInfo_;
			}
		}
	} else {
		// always just "pass through" if it is worth to be printed
		if (textIsWorthToPrint(textInfo_.thetext)) {
			flushTextBuffer(false); // false -> use textinfo_
		}
	}
}

void drvbase::pushText(const size_t len, const char *const thetext, const float x, const float y, const char * const glyphnames)
{
		textInfo_.x = x;
		textInfo_.y = y;
		textInfo_.thetext.copy(thetext, len);
		textInfo_.glyphnames.copy(glyphnames ? glyphnames:"");
		textInfo_.remappedfont= false;
		const char *remappedFontName = drvbase::theFontMapper().mapFont(textInfo_.currentFontName);
		// errf << " Mapping of " << textInfo_.currentFontName << " returned " << (remappedFontName ? remappedFontName:" ") << endl;
		if (remappedFontName) {
			if (verbose) {
				errf << "Font remapped from '" << textInfo_.
					currentFontName << "' to '" << remappedFontName << "'" << endl;
			}
			textInfo_.currentFontName.copy(remappedFontName);
			textInfo_.remappedfont= true;
		}

		showOrMergeText();

#if 0
		if ((lasttextInfo_.y == textInfo_.y)
			&& (lasttextInfo_.x_end >= textInfo_.x)
			&& (lasttextInfo_.x < textInfo_.x)
			&& lasttextInfo_.samefont(textInfo_)) {
			if (verbose) {
				errf << "Text overlap ! '" << lasttextInfo_.thetext.
					value() << "' and '" << textInfo_.thetext.value() << endl;
			}
		}
#endif

}


static unsigned short hexdecode( char high, char low) {
	return 16*hextoint(high) + hextoint(low);
}

void drvbase::pushHEXText(const char *const thetext, const float x, const float y, const char * const glyphnames)
{
	const unsigned int textlen = strlen(thetext);
	if (textlen) {
		char * decodedText = new char[ (textlen / 2 ) + 1 ];
		for (unsigned int i = 0, j = 0; i < (textlen/2); i++) {
			decodedText[i] = (char) hexdecode(thetext[j], thetext[j+1]);
			j++;j++;
		}
		decodedText[textlen/2] = '\0';
		pushText(textlen/2,decodedText,x,y,glyphnames);
		delete [] decodedText;
	}
}

void drvbase::setCurrentWidthParams(const float ax,
									const float ay,
									const int Char,
									const float cx,
									const float cy, const float x_end, const float y_end)
{
	textInfo_.ax = ax;
	textInfo_.ay = ay;
	textInfo_.Char = Char;
	textInfo_.cx = cx;
	textInfo_.x_end = x_end;
	textInfo_.y_end = y_end;
	textInfo_.cy = cy;
}

void drvbase::setCurrentFontName(const char *const Name, bool is_non_standard_font)
{
	textInfo_.currentFontName.copy(Name);
	textInfo_.is_non_standard_font = is_non_standard_font;
}

void drvbase::setCurrentFontFamilyName(const char *const Name)
{
	textInfo_.currentFontFamilyName.copy(Name);
}

void drvbase::setCurrentFontFullName(const char *const Name)
{
	textInfo_.currentFontFullName.copy(Name);
}

void drvbase::setCurrentFontWeight(const char *const Name)
{
	textInfo_.currentFontWeight.copy(Name);
}

void drvbase::setCurrentFontSize(const float Size)
{								/* errf << "setting Size to " << Size << endl; */
	textInfo_.currentFontSize = Size;
}

void drvbase::setCurrentFontAngle(float value)
{
	textInfo_.currentFontAngle = value;
}

bool drvbase::is_a_rectangle() const
{
//in most cases of rectangles there are 5 Elements
	if (numberOfElementsInPath() != 5)
		return false;
	if (pathElement(0).getType() == closepath || pathElement(4).getType() == closepath ) return false;

//first and last points are identical
	if (pathElement(0).getPoint(0).x_ != pathElement(4).getPoint(0).x_ ||
		pathElement(0).getPoint(0).y_ != pathElement(4).getPoint(0).y_)
		return false;



	unsigned int start_horic_test;
	unsigned int start_vert_test;

	if (pathElement(0).getPoint(0).x_ == pathElement(1).getPoint(0).x_) {
		start_horic_test = 0;
		start_vert_test = 1;
	} else {
		start_horic_test = 1;
		start_vert_test = 0;
	}

	{
		for (unsigned int i = start_horic_test; i < 4; i++, i++)
			if (pathElement(i).getPoint(0).x_ != pathElement((i + 1) % 4).getPoint(0).x_)
				return false;
	}

	{
		for (unsigned int i = start_vert_test; i < 4; i++, i++)
			if (pathElement(i).getPoint(0).y_ != pathElement((i + 1) % 4).getPoint(0).y_)
				return false;
	}
	return true;
}

void drvbase::add_to_page()
{
	if (page_empty) {
		page_empty = false;
		currentPageNumber++;
		open_page();
	}
}


DashPattern::DashPattern(const char
						 *patternAsSetDashString):dashString(patternAsSetDashString),
nrOfEntries(-1), numbers(0), offset(0)
{
	const char *pattern = patternAsSetDashString;
	// first count number of ' ' in pattern to determine number of entries
	// we normally have one less than number of blanks
	// line looks like: " [ 2.25 6.75 ] 0.0 setdash"

	while ((*pattern) && (*pattern != ']')) {
		if (*pattern == ' ')
			nrOfEntries++;
		pattern++;
	}

	// errf << nr_of_entries << " entries found in " << pattern << endl;
	if (nrOfEntries > 0) {
		pattern = patternAsSetDashString;
		// now get the numbers
		// repeat the numbers, if number of entries is odd
		unsigned int rep = nrOfEntries % 2;	// rep is 1 for odd numbers 0 for even
		numbers = new float[nrOfEntries * (rep + 1)];
		unsigned int cur = 0;
#if 1
		for (unsigned int i = 0; i <= rep; i++) {
			pattern = patternAsSetDashString;
			while ((*pattern) && (*pattern != ']')) {
				if (*pattern == ' ' && (*(pattern + 1) != ']')) {
					float f = (float) atof(pattern);
					numbers[cur] = f;
					// errf << d_numbers[cur] << endl;
					cur++;
				}
				pattern++;
			}
		}
//      if ( *(pattern+1) == ']' ) {
//          offset = (float) atof(pattern +2);
//      }
		if (*(pattern) == ']') {	// DMB // fixed by david butterfield
			offset = (float) atof(pattern + 1);	// DMB
		}
#else
		// this is the "C++" version. But this doesn't work with the GNU library under Linux
		for (unsigned int i = 0; i <= rep; i++) {
			// on some systems istrstreams expects a non const char *
			// so we need to make a copy
			char *localpattern = new char[strlen(pattern + 1) + 1];
			strcpy(localpattern, pattern + 1);	// skip leading [
			istrstream instream(localpattern);
			while (!instream.fail()) {
				float f;
				instream >> f;
				if (!instream.fail()) {
					d_numbers[cur] = f;
					// errf << d_numbers[cur] << endl;
					cur++;
				}
			}
			delete[]localpattern;
		}
#endif
	}
}

DashPattern::~DashPattern()
{
	delete[]numbers;
	numbers = 0;
	nrOfEntries = 0;
}


void drvbase::guess_linetype()
{
	DashPattern dp(dashPattern());
	const float *const d_numbers = dp.numbers;
	const int nr_of_entries = dp.nrOfEntries;

	drvbase::linetype curtype = solid;
	if (nr_of_entries > 0) {
		int rep = nr_of_entries % 2;	// rep is 1 for odd numbers 0 for even
		// now guess a pattern from
		// solid, dashed, dotted, dashdot, dashdotdot ; // corresponding to the CGM patterns
		switch (nr_of_entries * (rep + 1)) {
		case 2:
			if (d_numbers[1] == 0.0f) {
				curtype = drvbase::solid;	// if off is 0 -> solid
			} else if ((d_numbers[0] / d_numbers[1]) > 100) {
				curtype = drvbase::solid;	// if on/off > 100 -> use solid
			} else if (d_numbers[0] < 2.0f) {
				// if on is < 2 then always dotted
				// ok we miss '.             .             .'
				curtype = drvbase::dotted;
			} else {
				curtype = drvbase::dashed;
			}
			break;
		case 4:
			if ((d_numbers[1] == 0.0f) && (d_numbers[3] == 0.0f)) {
				curtype = drvbase::solid;	// if off is 0 -> solid
			} else if ((d_numbers[0] < 2.0f) || (d_numbers[2] < 2.0f)) {
				curtype = drvbase::dashdot;
			} else {
				curtype = drvbase::dashed;
			}
			break;
		case 6:
			if ((d_numbers[1] == 0.0f) && (d_numbers[3] == 0.0f)
				&& (d_numbers[5] == 0.0f)) {
				curtype = drvbase::solid;	// if off is 0 -> solid
			} else if ((d_numbers[0] < 2.0f) || (d_numbers[2] < 2.0f)
					   || (d_numbers[2] < 2.0f)) {
				curtype = drvbase::dashdotdot;
			} else {
				curtype = drvbase::dashed;
			}
			break;
		default:
			curtype = drvbase::dashed;
			break;
		}
	} else {
		// no entry
		curtype = drvbase::solid;
	}
//   errf << "linetype from " << dashPattern() << " is " << curtype << endl;
	setCurrentLineType(curtype);
}

void drvbase::dumpImage()
{
	flushOutStanding();					// dump last path to avoid wrong sequence of text and graphics
	add_to_page();
	imageInfo.calculateBoundingBox();
	show_image(imageInfo);
	delete[]imageInfo.data;
	imageInfo.nextfreedataitem = 0;
	imageInfo.data = 0;
}

unsigned int drvbase::nrOfSubpaths() const
{
	unsigned int nr = 0;
	for (unsigned int n = 0; n + 1 < numberOfElementsInPath(); n++) {
		const basedrawingelement & elem = pathElement(n);
		if (elem.getType() == moveto)
			nr++;
	}
	return nr;
}


void drvbase::dumpRearrangedPathes()
{
	// Count the subpaths
	unsigned int numpaths = nrOfSubpaths();
	if (verbose)
		errf << "numpaths: " << numpaths << endl;
	// Rearrange the path if necessary
	if ((numpaths > 1) && (currentLineWidth() == 0.0) && (currentShowType() != drvbase::stroke)) {
		if (verbose)
			errf << "Starting rearrangment of subpaths" << endl;
		outputPath->rearrange();
		numpaths = nrOfSubpaths();
	}
	if (!numpaths)
		numpaths = 1;

	const unsigned int origCount = numberOfElementsInPath();
	unsigned int starti = 0;
	for (unsigned int i = 0; i < numpaths; i++) {
		unsigned int end = starti;
		outputPath->subpathoffset = 0;
		while (true)				// Find the next end index
		{
			end++;
			if (end >= origCount)
				break;
			else if (pathElement(end).getType() == moveto)
				break;
		}
		if (end <= origCount) {
			if (verbose)
				errf << "dumping subpath from " << starti << " to " << end << endl;
			outputPath->subpathoffset = starti;
			outputPath->numberOfElementsInPath = end - starti;
			show_path();		// from start to end
		}
		starti = end;
	}
	outputPath->numberOfElementsInPath = origCount;
	outputPath->subpathoffset = 0;
}

bool drvbase::close_output_file_and_reopen_in_binary_mode()
{
	if (Verbose()) cerr << "begin close_output_file_and_reopen_in_binary_mode" << endl;

	if (outFileName.value() || (&outf != &cout) )
		// output is to a file, and outf is not cout
	{
	 	ofstream *outputFilePtr = (ofstream *) (& outf);
//		ofstream *outputFilePtr = dynamic_cast<ofstream *> (& outf);

		//dbg cerr << "outputfileptr = " << (void*) outputFilePtr << " outf " << (void*) (&outf)<< endl;

		outputFilePtr->close();
		if (Verbose()) cerr << "after close " << endl;
#if (defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined(__EMX__) || defined (NetBSD)  ) && !defined(DJGPP)
		// binary is not available on UNIX, only on PC
		outputFilePtr->open(outFileName.value(), ios::out);
#else
		// use redundant ios::out because of bug in djgpp
		outputFilePtr->open(outFileName.value(), ios::out | ios::binary);
		
#endif
		if (Verbose()) cerr << "after open " << endl;
		return true;
	} else {
		cerr << "Error: This driver cannot write to stdout since it writes binary data " << endl;
		return false;
	}
//	return 0; // not reached - but to make some compilers happy
}



void drvbase::beginClipPath()
{
	// now we start a clippath, so we need to dump
	// all previous pathes
	flushOutStanding();
	last_currentPath = currentPath;
	currentPath = &clippath;
	outputPath = currentPath;
	setCurrentShowType(drvbase::stroke);
}

void drvbase::endClipPath(cliptype clipmode)
{
	add_to_page();
	ClipPath(clipmode);
	clippath.clear();
	currentPath = last_currentPath;
	outputPath = currentPath;
}

// default versions
//  virtual 
void drvbase::ClipPath(cliptype /* clipmode */ )
{
}

//  virtual
void drvbase::Save()
{
}

//  virtual 
void drvbase::Restore()
{
}

void drvbase::flushOutStanding( flushmode_t flushmode )
{
	switch ( flushmode ) {
		case  flushall:
			// this needs to be fixed concerning the ordering (which was first  - the text or the path)
			flushOutStanding(flushpath);
			flushOutStanding(flushtext); 
			break;
		case flushtext:
			if (textIsWorthToPrint(mergedTextInfo.thetext.value())) {
				flushTextBuffer(true); 
				mergedTextInfo.thetext="";		// clear the merge buffer
			} 
			break;
		case flushpath:
			dumpPath(false); // false -> no flush text
			break;
		default:
			break;
	}
}

void drvbase::dumpPath(bool doFlushText)
{
	if (doFlushText) flushOutStanding(flushtext); // flush text, so merge is not supported in case of
								 // text path text sequence

	guess_linetype();			 // needs to be done here, because we must write to currentpath

#ifdef fixlater
	// this does not work as it is at the moment since
	// * it changes the showtype also for subsequent segments which might have
	//   more than 2 points AND
	// * it is not valid, if the only element (besides moveto) is a curveto.

	if (currentPath->numberOfElementsInPath == 2) {
		// a polygon with two points is drawn as a line

		// PROBLEM ! This resetting has an impact on the subsequent segments
		// if subpathes are not supported by the backend !!!!
		currentPath->isPolygon = false;
		currentPath->currentShowType = drvbase::stroke;
	}
#endif

	if (currentPath->currentShowType != drvbase::stroke) {
		/* don't show border with fill */
		setCurrentLineWidth(0.0f);
	}

	if (domerge && pathsCanBeMerged(PI1, PI2)) {
		// make PI1 the outputPath and clear PI2
		if (verbose) {
			errf << "Path " << PI1.nr << " type " << (int) PI1.currentShowType << endl;
			errf << PI1.fillR << " " << PI1.fillG << " " << PI1.fillB << endl;
			errf << PI1.edgeR << " " << PI1.edgeG << " " << PI1.edgeB << endl;
			errf << PI1.currentLineWidth << endl;

			errf << "Path " << PI2.nr << " type " << (int) PI2.currentShowType << endl;
			errf << PI2.fillR << " " << PI2.fillG << " " << PI2.fillB << endl;
			errf << PI2.edgeR << " " << PI2.edgeG << " " << PI2.edgeB << endl;
			errf << PI2.currentLineWidth << endl;
			errf << " have been merged\n";
		}
		// merge PI2 into PI1
		if (PI1.currentShowType == stroke) {
			// PI2 is the fill
			PI1.currentShowType = PI2.currentShowType;
			PI1.fillR = PI2.fillR;
			PI1.fillG = PI2.fillG;
			PI1.fillB = PI2.fillB;
		} else {
			// PI1 is the fill, so copy the line parameters from PI2
			PI1.currentLineWidth = PI2.currentLineWidth;
			PI1.edgeR = PI2.edgeR;
			PI1.edgeG = PI2.edgeG;
			PI1.edgeB = PI2.edgeB;
		}
		if (verbose) {
			errf << " result is \n";
			errf << "Path " << PI1.nr << " type " << (int) PI1.currentShowType << endl;
			errf << PI1.fillR << " " << PI1.fillG << " " << PI1.fillB << endl;
			errf << PI1.edgeR << " " << PI1.edgeG << " " << PI1.edgeB << endl;
			errf << PI1.currentLineWidth << endl;
		}
		outputPath = &PI1;
		PI1.pathWasMerged = true;
		PI2.clear();
	} else {
		outputPath = lastPath;
	}
	if (numberOfElementsInPath() > 0) {

		// nothing to do for empty pathes
		// pathes may be empty due to a merge operation

		if (verbose) {
			errf << "working on";
			switch (currentShowType()) {
			case drvbase::stroke:
				errf << " stroked ";
				break;
			case drvbase::fill:
				errf << " filled ";
				break;
			case drvbase::eofill:
				errf << " eofilled ";
				break;
			default:
				break;
			}
			errf << "path " << currentNr() << " with " <<
				numberOfElementsInPath() << " elements" << endl;
		}

		if (numberOfElementsInPath() > 1) {
			// cannot draw single points 
			add_to_page();
			if (isPolygon()) {	/* PolyGon */
				if (is_a_rectangle()) {
					const float llx =
						min(min
							(pathElement(0).getPoint(0).x_,
							 pathElement(1).getPoint(0).x_),
							min(pathElement(2).getPoint(0).x_, pathElement(3).getPoint(0).x_));
					const float urx =
						max(max
							(pathElement(0).getPoint(0).x_,
							 pathElement(1).getPoint(0).x_),
							max(pathElement(2).getPoint(0).x_, pathElement(3).getPoint(0).x_));
					const float lly =
						min(min
							(pathElement(0).getPoint(0).y_,
							 pathElement(1).getPoint(0).y_),
							min(pathElement(2).getPoint(0).y_, pathElement(3).getPoint(0).y_));
					const float ury =
						max(max
							(pathElement(0).getPoint(0).y_,
							 pathElement(1).getPoint(0).y_),
							max(pathElement(2).getPoint(0).y_, pathElement(3).getPoint(0).y_));

					show_rectangle(llx, lly, urx, ury);
				} else {
					if (globaloptions.simulateSubPaths)
						dumpRearrangedPathes();
					else
						show_path();
				}
			} else {			/* PolyLine */
				if (globaloptions.simulateSubPaths)
					dumpRearrangedPathes();
				else
					show_path();
			}
		}
		// cleanup
		outputPath->clear();
	}
	// swap current and last pointers
	PathInfo *help = currentPath;
	currentPath = lastPath; // currentPath will be filled next be Lexer
	lastPath = help;

	currentPath->copyInfo(*help);	// initialize next path with state of last path
	// currentPath is the path filled next by lexer

	outputPath = currentPath;
}

void drvbase::removeFromElementFromPath()
{
	currentPath->numberOfElementsInPath--;
}

void drvbase::addtopath(basedrawingelement * newelement)
{
	if (newelement) {
		if (currentPath->numberOfElementsInPath < maxElements) {
			currentPath->path[currentPath->numberOfElementsInPath] = newelement;
#ifdef DEBUG
			cout << "pathelement " << currentPath->
				numberOfElementsInPath << " added " << *newelement << endl;
#endif
			currentPath->numberOfElementsInPath++;
		} else {
			errf <<
				"Fatal: number of path elements exceeded. Increase maxElements in drvbase.h"
				<< endl;
			exit(1);
		}
	} else {
		errf << "Fatal: newelement is NIL in addtopath " << endl;
		exit(1);
	}
}

void drvbase::PathInfo::clear()
{
	for (unsigned int i = 0; i < numberOfElementsInPath; i++) {
		// delete path[i];
		path[i]->deleteyourself(); // see note in drvbase.h 
		path[i] = 0;
	}
	numberOfElementsInPath = 0;
	pathWasMerged = false;
}

void drvbase::PathInfo::copyInfo(const PathInfo & p)
{
	// copies the whole path state except the path array
	currentShowType = p.currentShowType;
	currentLineType = p.currentLineType;
	currentLineCap = p.currentLineCap;
	currentLineJoin = p.currentLineJoin;
	currentMiterLimit = p.currentMiterLimit;
	nr = p.nr;
	// Path is not copied path(0),
	isPolygon = p.isPolygon;
	// numberOfElementsInPath = p.numberOfElementsInPath;
	currentLineWidth = p.currentLineWidth;
	edgeR = p.edgeR;
	edgeG = p.edgeG;
	edgeB = p.edgeB;
	fillR = p.fillR;
	fillG = p.fillG;
	fillB = p.fillB;
	colorName = p.colorName;
	dashPattern = p.dashPattern;
}

ostream & operator << (ostream & out, const basedrawingelement & elem)
{
	out << "type: " << (int) elem.getType() << " params: ";
	for (unsigned int i = 0; i < elem.getNrOfPoints(); i++) {
		out << elem.getPoint(i).x_ << " " << elem.getPoint(i).y_ << " ";
	}
	out << endl;
	return out;
}

ColorTable::ColorTable(const char *const *defaultColors, const unsigned int numberOfDefaultColors, makeColorNameType makeColorName):
defaultColors_(defaultColors),
numberOfDefaultColors_(numberOfDefaultColors), makeColorName_(makeColorName)
{
//dbg   cerr << " Constructing a color table with " << numberOfDefaultColors << " default colors" << endl;
	for (unsigned int i = 0; i < maxcolors; i++)
		newColors[i] = 0;
//dbg   cerr << 1/(1/numberOfDefaultColors) << endl;
}

ColorTable::~ColorTable()
{
	unsigned int current = 0;
	while (newColors[current] != 0) {
		delete[] newColors[current];
		newColors[current] = NIL;
		current++;
	}
	// cannot assign since it is const - defaultColors_ = NIL;
	//lint -esym(1540,ColorTable::defaultColors_)
}


unsigned int ColorTable::getColorIndex(float r, float g, float b)
{
// registers a possibly new color and returns the index 
// under which the color was registered
	const char *cmp = makeColorName_(r, g, b);
	for (unsigned int i = 0; i < numberOfDefaultColors_; i++) {
		if (strcmp(cmp, defaultColors_[i]) == 0) {
			return i;
		}
	}
// look in new colors
	unsigned int j ;
	for (j = 0; ((j < maxcolors) && (newColors[j] != 0)); j++) {
		if (strcmp(cmp, newColors[j]) == 0) {
			return j + numberOfDefaultColors_;
		}
	}
// not found so far
// j is either maxcolors or the index of the next free entry
// add a copy to newColors
	if (j < maxcolors) {
		const unsigned int size = strlen(cmp) + 1;
		newColors[j] = new char[size];
		strcpy_s(newColors[j], size, cmp);
		return j + numberOfDefaultColors_;
	} else {
//      cerr << "running out of colors" << endl;
		return 0;
	}

}

const char *  ColorTable::getColorString(float r, float g, float b)	// non const
{
	return getColorString(getColorIndex(r, g, b));
}

bool ColorTable::isKnownColor(float r, float g, float b) const
{
// Possible improvements:
// could return the next free entry as negative number in case
// the color is not found. This would make it possible to
// use this function in getColorEntry as well, or (better)
// make a pure registercolor(index,.....) instead of
// getColorEntry.
	const char *cmp = makeColorName_(r, g, b);
	for (unsigned int i = 0; i < numberOfDefaultColors_; i++) {
		if (strcmp(cmp, defaultColors_[i]) == 0) {
			return true;
		}
	}
	// look in new colors
	for (unsigned int j = 0; ((j < maxcolors) && (newColors[j] != 0)); j++) {
		if (strcmp(cmp, newColors[j]) == 0) {
			return true;		// j+numberOfDefaultColors_;
		}
	}
	// not found so far
	return false;
}

const char *  ColorTable::getColorString(unsigned int index) const
{
	return (index < numberOfDefaultColors_) ? defaultColors_[index] :
		newColors[index - numberOfDefaultColors_];
}



const DriverDescription *DescriptionRegister:: getDriverDescForName(const char *drivername) const
{
	unsigned int i = 0;
	while (rp[i] != 0) {
		if ((strcmp(drivername, rp[i]->symbolicname) == 0)) {
			return rp[i];
		}
		i++;
	}
	return 0;
}

const DriverDescription *DescriptionRegister:: getDriverDescForSuffix(const char *suffix) const
{
	unsigned int i = 0;
	const DriverDescription * founditem = 0; 
	while (rp[i] != 0) {
		if ((strcmp(suffix, rp[i]->suffix) == 0)) {
			if (founditem) {
				// already found an entry for this suffix - so it is not unique -> return 0
				return 0;
			} else {
				founditem = rp[i]; // first chance - but loop throug all items
			}
		}
		i++;
	}
	return founditem;
}

void DescriptionRegister::explainformats(ostream & out, bool withdetails) const
{
	if (withdetails) {
		// out << "\\subsection{Available formats and their specific options}" << endl;
	} else {
		out << "Available formats :\n";
	}
	unsigned int i = 0;
	while (rp[i] != 0) {
		if (withdetails) {
			out << "\\subsubsection{" << rp[i]->symbolicname << " - " << rp[i]->short_explanation <<"}" << endl;
			if (strlen(rp[i]->long_explanation)>0) { out << rp[i]->long_explanation << endl << endl; }
		} else {
			out << '\t' << rp[i]->symbolicname << ":\t";
			if (strlen(rp[i]->symbolicname) < 7) {
				out << '\t';
			}
			out << "\t." << rp[i]->suffix << ":\t";
			out << rp[i]->short_explanation << " " << rp[i]->additionalInfo;
		}

		if (!withdetails && rp[i]->checkfunc) {
			if (!(rp[i]->checkfunc())) {
				out << " (no valid key found)";
			}
		}

		if (!withdetails) out << "\t(" << rp[i]->filename << ")" << endl;

		ProgramOptions* dummy = rp[i]->createDriverOptions();
		if (!withdetails && dummy->numberOfOptions() ) {
			out << "This driver supports the following additional options: (specify using -f \"format:-option1 -option2\")" << endl;
		}
		dummy->showhelp(out,withdetails,withdetails);
		delete dummy;
		
		if (withdetails) {
			out << "%%// end of options " << endl;
		} else {
			out << "-------------------------------------------" << endl;
		}
	
		i++;
	}
}
void DescriptionRegister::mergeRegister(ostream & out,
										const DescriptionRegister & src, const char *filename)
{
	int i = 0;
	while (src.rp[i]) {
		const unsigned int srcversion = src.rp[i]->getdrvbaseVersion();
		if (srcversion != 0) {
			if (srcversion == drvbaseVersion) {
				src.rp[i]->filename = filename;
				registerDriver(src.rp[i]);
			} else {
				out << src.rp[i]->short_explanation << "(" << filename << ")" <<
					" - backend has other version than expected by pstoedit core "
					<< srcversion << " <> " << drvbaseVersion << endl;
				out <<
					"The pstoedit.dll (core) and the additional DLLs (plugins.dll or importps.dll) must have the same version number."
					<< endl;
				out << 
					"Please get a consistent set of pstoedit.dll (plugins.dll and or importps.dll) from www.pstoedit.net/pstoedit/ " 
					<< endl;

			}
		}
		i++;
	}
}
void DescriptionRegister::registerDriver(DriverDescription * xp)
{
	//  cout << " registering " << (void *) xp << endl;
	// check for duplicate:
	for (int i = 0; i < ind; i++) {
		if (strcmp(rp[i]->symbolicname, xp->symbolicname) == 0) {
			// duplicate found
			if (xp->checkfunc && xp->checkfunc() && !(rp[i]->checkfunc())) {
				// the new one has a license, so use this instead
				rp[i] = xp;
			}
			return; // just use the first version - except for the above case.
		}
	}
	rp[ind] = xp;
	ind++;
}

// int Rinit::ref = 0;
DLLEXPORT DescriptionRegister *globalRp = 0; 

extern "C" DLLEXPORT DescriptionRegister * getglobalRp()
{
	return &DescriptionRegister::getInstance();
}

#ifdef  BUGGYGPP
Point Point::transform(const float * matrix) const
#else
Point Point::transform(const float matrix[6]) const
#endif
{
	const float tx = matrix[0] * x_ + matrix[2] * y_ + matrix[4];
	const float ty = matrix[1] * x_ + matrix[3] * y_ + matrix[5];
	return  Point(tx, ty);
}



const char * DriverDescription::currentfilename = "built-in";
DriverDescription::DriverDescription(	const char *const s_name, 
										const char *const short_expl, 
										const char *const long_expl, 
										const char *const suffix_p, 
										const bool backendSupportsSubPathes_p, 
										const bool backendSupportsCurveto_p, 
										const bool backendSupportsMerging_p,	// merge a separate outline and filling of a polygon -> 1. element
										const bool backendSupportsText_p, 
										const imageformat backendDesiredImageFormat_p, 
										const opentype backendFileOpenType_p, 
										const bool backendSupportsMultiplePages_p, 
										const bool backendSupportsClipping_p, 
										const bool nativedriver_p,
										checkfuncptr checkfunc_p):
symbolicname(s_name), short_explanation(short_expl), long_explanation(long_expl), suffix(suffix_p), additionalInfo((checkfunc_p != 0) ? (checkfunc_p()? "" : "(license key needed, see pstoedit manual)") : ""), backendSupportsSubPathes(backendSupportsSubPathes_p), backendSupportsCurveto(backendSupportsCurveto_p), backendSupportsMerging(backendSupportsMerging_p),	// merge a separate outline and filling of a polygon -> 1. element
	backendSupportsText(backendSupportsText_p), 
	backendDesiredImageFormat(backendDesiredImageFormat_p),
backendFileOpenType(backendFileOpenType_p),
backendSupportsMultiplePages(backendSupportsMultiplePages_p),
backendSupportsClipping(backendSupportsClipping_p), 
nativedriver(nativedriver_p),
filename(DriverDescription::currentfilename), checkfunc(checkfunc_p)
{
	DescriptionRegister & registry = DescriptionRegister::getInstance();
//dbg	cout << "registering driver " << s_name << "\t at registry " << (void*) &registry << endl;
	registry.registerDriver(this);
}

#if 0
// not needed - pure virtual
drvbase *DriverDescription::
CreateBackend(const char *const driveroptions_P, ostream & theoutStream,
			  ostream & theerrStream, const char *const nameOfInputFile,
			  const char *const nameOfOutputFile, 
			  const PsToEditOptions & globaloptions) const
{
	unused(driveroptions_P);
	unused(&theoutStream);
	unused(&theerrStream);
	unused(nameOfInputFile);
	unused(nameOfOutputFile);
	unused(&globaloptions);
	return 0;
}
#endif

#if 0
// def BUGGYGPP
//
// GNU g++ causes a seg fault, when the singleton instance is destroyed triggered by a dlclose()
// so we have to live with this small memory leak due to the allocation on the heap
//
// This problem is now solved by the removal of the dtor for this class.
//
DescriptionRegister & DescriptionRegister::getInstance()
{
	static DescriptionRegister * theSingleInstance = new DescriptionRegister;
	globalRp = theSingleInstance;
	return *theSingleInstance;
}
#else
DescriptionRegister & DescriptionRegister::getInstance()
{
	static DescriptionRegister theSingleInstance;
	globalRp = &theSingleInstance;
	return theSingleInstance;
}
#endif

//
// SINGLETONSONHEAP might be useful if problems occur during the unloading of libpstoedit.so
//

// Implementation of SingleTon "Objects".

BBox * drvbase::bboxes() {	// array of bboxes - maxpages long
#ifdef SINGLETONSONHEAP
	static BBox * dummy = new BBox[maxPages];	return dummy;
#else
	static BBox dummy[maxPages]; return &(dummy[0]);
#endif
}

unsigned int drvbase::totalNumberOfPages = 0;

RSString& drvbase::pstoeditHomeDir(){// usually the place where the binary is installed
#ifdef SINGLETONSONHEAP
	static RSString *dummy = new RSString(""); 	return *dummy;
#else
	static RSString dummy(""); 	return dummy;
#endif
}
RSString& drvbase::pstoeditDataDir() {// where the fmp and other data files are stored
#ifdef SINGLETONSONHEAP
	static RSString *dummy = new RSString(""); 	return *dummy;
#else
	static RSString dummy(""); 	return dummy;
#endif
}

// the global static FontMapper 
FontMapper& drvbase::theFontMapper() {
#ifdef SINGLETONSONHEAP
	static FontMapper *dummy = new FontMapper;	return *dummy;
#else
	static FontMapper dummy;	return dummy;
#endif
}

bool drvbase::verbose = false; // offensichtlich kann man keine initialisierten Daten DLLEXPORTieren
bool drvbase::Verbose() { return verbose; }
void drvbase::SetVerbose(bool param) { verbose = param; }
 
