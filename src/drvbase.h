#ifndef __drvbase_h
#define __drvbase_h
/*
   drvbase.h : This file is part of pstoedit Base class for all specific
   driver classes/backends. All virtual functions have to be implemented by
   the specific driver class. See drvSAMPL.cpp
  
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
/*
// ============================================================================
//
// = LIBRARY
//     pstoedit
//
// = FILENAME
//     drvbase.h
//
// = RCSID
//     $Id$
*/

#ifndef cppcomp_h
#include "cppcomp.h"
#endif


#include I_fstream
#include I_stdio
#include I_stdlib
#include I_string_h
USESTD

#ifndef assert
#include <assert.h>
#endif
#include "pstoeditoptions.h"
#ifndef miscutil_h
#include "miscutil.h"
#endif

#if defined(HAVE_STL) && !defined(USE_FIXED_ARRAY)
#include <vector>
#endif

// for compatibility checking
static const unsigned int drvbaseVersion = 108;
// 101 introduced the driverOK function
// 102 introduced the font optimization (lasttextinfo_)
// 103 introduced the -ssp support and the virtual pathscanbemerged
// 104 introduced the fontmatrix handling
// 105 introduced the miterlimit Info and outputPageSize
// 106 introduced some new utility functions for transformation (*_trans*)
// 107 new driver descriptions -- added info about clipping
// 108 new driver descriptions -- added info about driver options

const unsigned int  maxPages     = 10000;   // maximum number of pages - needed for the array of bounding boxes
#if defined(HAVE_STL) && !defined(USE_FIXED_ARRAY)
 // we can use std::vector
#else
const unsigned int	maxPoints    = 80000;	// twice the maximal number of points in a path
const unsigned int	maxElements  = maxPoints/2;
const unsigned int	maxSegments  = maxPoints/2;// at least half of maxpoints (if we only have segments with one point)
#endif

class DLLEXPORT Point
{
public:
	Point(float x, float y) : x_(x),y_(y) {}
	Point() : x_(0.0f), y_(0.0f) {}; // for arrays
	float x_;
	float y_;
	bool operator==(const Point & p2) const { 
		return (x_ == p2.x_) && (y_ == p2.y_); //lint !e777
	}
	bool operator!=(const Point & p2) const { 
		return !(*this == p2);
	}
	Point operator+(const Point & p) const { return Point (x_+p.x_,y_+p.y_); }
	const Point & operator+=(const Point & p) { x_+=p.x_; y_+=p.y_; return *this; }
	Point operator*(float f) const { return Point (x_*f,y_*f); }

#if 0
	friend bool operator==(const Point & p1, const Point & p2) { 
		return (p1.x_ == p2.x_) && (p1.y_ == p2.y_); //lint !e777
	}
#endif
#ifdef  BUGGYGPP
	Point transform(const float * matrix) const;
#else
	Point transform(const float matrix[6]) const;
#endif

	friend ostream & operator<<(ostream & out,const Point &p) {
		return out << "x: " << p.x_ << " y: " << p.y_ ;
	}

private:
};

struct DLLEXPORT BBox // holds two points describing a Bounding Box 
{
public:
	Point ll;
	Point ur;
 
	friend ostream & operator<<(ostream & out,const BBox &bb) {
		return out << "LL: " << bb.ll << " UR: " << bb.ur ;
	}
 
};

// image needs Point !
#include "psimage.h"

static const char emptyDashPattern[] =  "[ ] 0.0";

class basedrawingelement; // forward
class DriverDescription ; // forward

class       DLLEXPORT    drvbase 
    // = TITLE
    // Base class for backends to pstoedit
    //
    // = CLASS TYPE
    // Abstract
    //
    // = AUDIENCE
    // Developers of new backends
    //
    // = DESCRIPTION
    //  Abstract base class for backends to pstoedit. 
    // This class defines the virtual functions that every backend has
    // to implement and some functions that are common to all backends
    //
{
	friend class sub_path; // needs PathInfo
	friend class sub_path_list;
public:
	// = PUBLIC TYPES 

	//lint -esym(578,drvbase::fill,fill)
	enum showtype { stroke, fill, eofill }; 
	enum cliptype { clip , eoclip };
	enum linetype { solid=0, dashed, dotted, dashdot, dashdotdot }; // corresponding to the CGM patterns
	struct DLLEXPORT TextInfo {
		float 		x;
		float		y;
		float		FontMatrix[6];
		float 		x_end; // pen coordinates after show in PostScript
		float		y_end; // 
		RSString 	thetext;
		RSString	glyphnames;
		bool		is_non_standard_font;
		RSString    currentFontName;
		RSString    currentFontUnmappedName;
		RSString    currentFontFamilyName;
		RSString    currentFontFullName;
		RSString    currentFontWeight;
		float       currentFontSize;
		float		currentFontAngle;
		float       currentR; // Colors
		float       currentG;
		float       currentB;
		RSString    colorName; // extracted from PostScript colorspace if /Separation type
		float		cx; // next five items correspond to the 
		float		cy; // params for the awidthshow operator
		int			Char; // of PostScript
		float		ax; 
		float		ay;
		bool		mappedtoIsoLatin1;
		bool		remappedfont; // was remapped via fontmap
		bool		samefont(const TextInfo& cmp) const {
			return ( currentFontName == cmp.currentFontName ) && //lint !e1702
				//	( currentFontFamilyName == cmp.currentFontFamilyName ) && 
				//	( currentFontFullName == cmp.currentFontFullName ) && 
					( currentFontWeight == cmp.currentFontWeight ) && //lint !e1702
					( currentFontSize == cmp.currentFontSize ) && //lint !e777 // testing floats for ==
					( currentFontAngle == cmp.currentFontAngle ) ; //lint !e777 // testing floats for ==
		}

		bool		samecolor(const TextInfo& cmp) const {
			return ( currentR == cmp.currentR ) && //lint !e777 // testing floats for ==
					( currentG == cmp.currentG ) && //lint !e777 // testing floats for ==
					( currentB == cmp.currentB ); //lint !e777 // testing floats for ==
		}
		TextInfo() :
			x(0.0f),
			y(0.0f),
			x_end(0.0f),
			y_end(0.0f),
//			thetext(0),  // use standard ctor
			is_non_standard_font(false),
			currentFontSize(10.0f),
			currentFontAngle(0.0f),
			currentR(0.0f),
			currentG(0.0f),
			currentB(0.0f),
			colorName(""),
			cx(0.0f),
			cy(0.0f),
			Char(32), // 32 means space
			ax(0.0f),
			ay(0.0f), 
			mappedtoIsoLatin1(true), 
			remappedfont(false) {
				for (int i = 0; i < 6 ; i++ ) FontMatrix[i] = 0.0f;
			}
		~TextInfo() { }
	private:
		// declared but not defined
		// const TextInfo & operator = (const TextInfo &); // default is ok
		// TextInfo(const TextInfo &); // default is ok
	};

private:
	// = PRIVATE TYPES 

protected:
	// = PROTECTED TYPES 

	struct DLLEXPORT PathInfo {
		showtype	currentShowType;
		linetype	currentLineType;
		unsigned int    currentLineCap; // Shape of line ends for stroke (0 = butt, 1 = round, 2 = square)
		unsigned int    currentLineJoin;
		float			currentMiterLimit;
		unsigned int    nr;
#if defined(HAVE_STL) && !defined(USE_FIXED_ARRAY)
		std::vector<basedrawingelement *> path;
#else
		basedrawingelement * * path; // a path is an array of pointers to basedrawingelements
#endif
		bool	 	isPolygon; // whether current path was closed via closepath or not
		unsigned int	numberOfElementsInPath;
		unsigned int	subpathoffset; // normally 0, but if subpathes are simulated
									   // then this is set to the begin of the current subpath 
									   // before show_path is executed
		float           currentLineWidth;
		float           edgeR; // edge colors
		float           edgeG;
		float           edgeB;
		float           fillR; // fill colors
		float           fillG;
		float           fillB;
		RSString		colorName;
		bool			pathWasMerged; // true, if this path is a result of a merge operation
		RSString	    dashPattern; // just the dump of currentdash as string
		PathInfo() :
			currentShowType(drvbase::stroke),
			currentLineType(drvbase::solid),
			currentLineCap(0),
			currentLineJoin(0),
			currentMiterLimit(10.0f),
			nr(0),
			path(0),
			isPolygon(false),
			numberOfElementsInPath(0),
			subpathoffset(0),
			currentLineWidth(0.0f),
			edgeR(0.0f),
			edgeG(0.0f),
			edgeB(0.0f),
			fillR(0.0f),
			fillG(0.0f),
			fillB(0.0f),
			colorName(""),
			pathWasMerged(false),
			dashPattern(emptyDashPattern)
			{
#if defined(HAVE_STL) && !defined(USE_FIXED_ARRAY)
#else
			    path = new basedrawingelement *[maxElements];
#endif
			}

		virtual ~PathInfo() { // added virtual because of windows memory handling
			// the path content is deleted by clear
			clear();
#if defined(HAVE_STL) && !defined(USE_FIXED_ARRAY)
			// use std dtor
#else
			delete [] path;
#endif
		}
		void addtopath(basedrawingelement * newelement,
			           ostream & errf);
		void clear();
		void copyInfo(const PathInfo & p);
			// copies the whole path state except the path array
		void rearrange();
            // rearrange subpaths for backends which do not support them 
	private:
		// Inhibitors (declared, but not defined)
		const PathInfo& operator=(const PathInfo&);
		PathInfo(const PathInfo &);
	};

//lint -esym(1712,SaveRestoreInfo) // no default ctor
	class DLLEXPORT SaveRestoreInfo {
	public:
		unsigned int clippathlevel; // number of clippathes since opening (=save)
		unsigned int savelevel;	
		SaveRestoreInfo * previous;	
		SaveRestoreInfo * next;
		SaveRestoreInfo(SaveRestoreInfo * parent) : clippathlevel(0), previous(parent), next(NIL) 
		{ 
			if (parent) {
				parent->next=this;
				savelevel = parent->savelevel + 1;
			} else {
				savelevel = 0;
			}
		}
	};

public:
	// = PUBLIC DATA

	const DriverDescription& driverdesc; // reference to the derived class' driverdescription

	ProgramOptions* DOptions_ptr;

	PSImage 		imageInfo;

	static FontMapper& theFontMapper();

	static bool Verbose();  // need a wrapper function because static initialized data cannot be DLLEXPORTed
	static void SetVerbose(bool param);
	static unsigned int &totalNumberOfPages();
	
	static BBox	* bboxes() ; // [maxPages]; // array of bboxes - maxpages long
	static RSString& pstoeditHomeDir(); // usually the place where the binary is installed
	static RSString& pstoeditDataDir(); // where the fmp and other data files are stored

protected:
	// = PROTECTED DATA

	ostream &	outf;           // the output stream
	ostream &	errf;           // the error stream
	const RSString	inFileName; // full name of input file
	const RSString	outFileName; // full name of output file

	RSString       	outDirName; 	// output path with trailing slash or backslash
	RSString       	outBaseName; 	// just the basename (no path, no suffix)
	unsigned int	d_argc;
	const char **			d_argv; // array of driver argument strings
	class PsToEditOptions & globaloptions; /* non const because driver can also add an option during ctor */
	float           currentDeviceHeight; // normally 792 pt (US Letter); used for flipping y values.
	float           currentDeviceWidth;  
	float           x_offset;
	float           y_offset;
	friend class PSFrontEnd; // PSFrontEnd needs access to currentPageNumber
	unsigned int    currentPageNumber;
	bool			domerge;
	const char *	defaultFontName; // name of default font 
	bool			ctorOK;			// indicated Constructor failure
									// returned via driverOK() function

	SaveRestoreInfo saveRestoreInfo;
	SaveRestoreInfo * currentSaveLevel;

private:
	// = PRIVATE DATA
	static bool	verbose; // access via Verbose() 
	bool    	page_empty;	// indicates whether the current page is empty or not
	char * 		driveroptions; // string containing options for backend
	PathInfo 	PI1,PI2,clippath; // pi1 and pi2 are filled alternatively (to allow merge), clippath when a clippath is read
	PathInfo * 	currentPath; // for filling from lexer
	PathInfo * 	last_currentPath; // need to save during usage of currentPath for clippath
protected: // for drvrtf
	PathInfo * 	outputPath;  // for output driver
private:
	PathInfo * 	lastPath;    // for merging
	TextInfo 	textInfo_;	 // this is used both by the lexer to fill in text related info 
							 // but also by the backends for query of info, e.g. via "fontchanged"
	TextInfo	mergedTextInfo; // for collecting pieces of text when doing text merge
	TextInfo 	lastTextInfo_; // for saving font settings. This is the last really dumped text

public:
	// = PUBLIC METHODS

	// = CONSTRUCTION, DESTRUCTION AND COPYING

	drvbase(
		const char * driverOptions_p,
		ostream & theoutStream,
		ostream & theerrStream,		
		const char* nameOfInputFile_p,
		const char* nameOfOutputFile_p,
		PsToEditOptions & globaloptions_p,
		const class DriverDescription & driverdesc_p
	); // constructor
	virtual ~drvbase();  		// destructor

	// = BACKEND GENERIC FUNCTIONS 
        // These functions are not backend specific and should not have to be
        // changed for new backends

	void		startup(bool merge);

	virtual void finalize(); 
	// needed because base destructor will be called after derived destructor
	// and thus the base destructor could no longer use the backend.
	// virtual because so we can achieve that it is called in the
	// context (DLL) of the real backend. Otherwise it would be called
	// in the context of the main program which causes memory problems
	// under windows since the plugins are NOT and extension DLL
	//

	void		setdefaultFontName(const char * n) {defaultFontName = n;}

	virtual bool textIsWorthToPrint(const RSString & thetext) const; // in the default implementation full blank strings are ignored

	virtual bool textCanBeMerged(const TextInfo & text1, const TextInfo & text2) const; // checked whether two pieces of text can be merged into one.

	void            setCurrentDeviceHeight(const float deviceHeight) 
			{ currentDeviceHeight = deviceHeight; }

	void            setCurrentDeviceWidth(const float deviceWidth) 
			{ currentDeviceWidth = deviceWidth; }

	float           getScale() const { return 1.0f; }

	inline long l_transX			(float x) const	{
		return (long)((x + x_offset) + .5);	// rounded long	
	}

	inline long l_transY			(float y) const {
		return (long)((-1.0f*y + y_offset) + .5);	// rounded long, mirrored
	}

	inline int i_transX			(float x) const	{
		return (int)((x + x_offset) + .5);	// rounded int	
	}

	inline int i_transY			(float y) const {
		return (int)((-1.0f*y + y_offset) + .5);	// rounded int, mirrored
	}

	inline float f_transX			(float x) const	{
		return (x + x_offset) ;	
	}

	inline float f_transY			(float y) const {
		return (-1.0f*y + y_offset) ;	
	}

	const RSString & getPageSize() const; // { return globaloptions.outputPageSize; }

	bool close_output_file_and_reopen_in_binary_mode(); //

	bool		fontchanged() const { return ! textInfo_.samefont(lastTextInfo_); }
	bool		textcolorchanged() const { return ! textInfo_.samecolor(lastTextInfo_); }

	void		setColorName(const char * const name) {
				textInfo_.colorName = name ;  
			 	currentPath->colorName = name; 
	}
	void		setRGB(const float R,const float G, const float B)
			{ 
			 if ( ( R > 1.0 ) || ( G > 1.0 ) || ( B > 1.0 ) ||
			      ( R < 0.0 ) || ( G < 0.0 ) || ( B < 0.0 ) ) {
				errf << "Warning: color value out of range (0..1). Color change ignored." << R << ' ' << G << ' ' << B << endl;
			 } else {
			 	textInfo_.currentR = R ; textInfo_.currentG = G; textInfo_.currentB = B; 
			 	currentPath->edgeR = R ; currentPath->edgeG = G; currentPath->edgeB = B; 
			 	currentPath->fillR = R ; currentPath->fillG = G; currentPath->fillB = B; 
			 }
			}

	void    	showpage();

	const BBox & getCurrentBBox() const;

	void beginClipPath();
	void endClipPath(cliptype clipmode) ;
	// virtuals - to be implemented by backends
	virtual void ClipPath(cliptype clipmode);
	virtual void Save();
	virtual void Restore();

	// = DRAWING RELATED METHODS

	void		addtopath(basedrawingelement * newelement);
	void		removeFromElementFromPath();

	unsigned int 	&numberOfElementsInPath() { return outputPath->numberOfElementsInPath; }
	unsigned int 	numberOfElementsInPath() const { return outputPath->numberOfElementsInPath; }

	const basedrawingelement & pathElement(unsigned int index) const;

	void            setCurrentLineType(const linetype how) 
			{ currentPath->currentLineType = how; }

	void            setCurrentLineWidth(const float linewidth) 
			{ currentPath->currentLineWidth = linewidth; }
			
	void 		setDash(const char * const dash)
			{ currentPath->dashPattern.assign(dash); }

	void	 	setIsPolygon(bool what) { currentPath->isPolygon=what; } // whether current path was closed via closepath or not

	void	 	setPathNumber(unsigned int nr) { currentPath->nr=nr; } 

	void            setCurrentLineJoin(const unsigned int joinType) 
			{ currentPath->currentLineJoin = joinType; }

	void            setCurrentMiterLimit(const float miterLimit) 
			{ currentPath->currentMiterLimit = miterLimit; }

	void            setCurrentLineCap(const unsigned int capType) 
			{ currentPath->currentLineCap = capType; }

	void            setCurrentShowType(const showtype how) 
			{ currentPath->currentShowType = how; }

	void 		dumpPath(bool doFlushText = true);  // shows current path

	//lint -esym(578,flushall)  // flushall hides same name in stdio
	enum		flushmode_t { flushall, flushtext, flushpath }; 

	void		flushOutStanding( flushmode_t flushmode = flushall);

	void		dumpRearrangedPathes(); // show the current subpathes after calling rearrange

	unsigned int nrOfSubpaths() const;

	void 		dumpImage();  // shows current image

	// = TEXT RELATED METHODS

 	void		setCurrentWidthParams(  const float ax,
						const float ay,
						const int Char,
						const float cx,
						const float cy,
				 		const float x_end = 0.0f, 
				 		const float y_end = 0.0f);

	void		setMappedtoisolatin1 (  const bool mapped )
			{ textInfo_.mappedtoIsoLatin1 = mapped; }

	void            setCurrentFontName(const char *const Name,bool is_non_standard_font);

	void            setCurrentFontFamilyName(const char *const Name);

	void            setCurrentFontFullName(const char *const Name);

	void            setCurrentFontWeight(const char *const Name);

	void            setCurrentFontSize(const float Size);

	void		setCurrentFontAngle(float value);

	const float *	getCurrentFontMatrix() const { return textInfo_.FontMatrix; }
	void 		setCurrentFontMatrix(const float mat[6]) { for (unsigned short i = 0; i< 6; i++) textInfo_.FontMatrix[i] = mat[i]; }

	// the push*Text methods set the textinfo_ member and then call the 
	// showOrMergeText(textinfo_) ;
	void    	pushText(const size_t len,
				        const char *const thetext,
					const float x, 
					const float y,
					const char * const glyphnames=0);

	void		pushHEXText(const char *const thetext, 
					const float x, 
					const float y,
					const char * const glyphnames=0);

	void		flushTextBuffer(bool useMergeBuffer); // flushes text from the text (merge) buffer 
	void		showOrMergeText();

	// = BACKEND SPECIFIC FUNCTIONS

	// If a backend only deals with a special set of font names
	// the following function must return a 0 terminated list
	// of font names.
	virtual const char * const * 	knownFontNames() const { return 0; }

	// The next functions are virtual with a default empty implementation

	virtual void    show_image(const PSImage & /* imageinfo */) {
		cerr << "show_image called, although backend does not support images" << endl;
		//unused(&imageinfo);
	}

	// if during construction something may go wrong, a backend can
	// overwrite this function and return false in case of an error.
	// or it can just set the ctorOK to false.
	virtual bool driverOK() const { return ctorOK; } // some  

	// needed to check for pseude drivers which do not have a real backend
	// but instead just do the job in the gs frontend.
	virtual bool withbackend() const { return true; }

protected:
	// = PROTECTED METHODS

	showtype		currentShowType() const { return outputPath->currentShowType; }
	linetype		currentLineType() const { return outputPath->currentLineType; }
	unsigned int	currentLineCap() const { return outputPath->currentLineCap; }
	unsigned int	currentLineJoin() const { return outputPath->currentLineJoin; }
	float			currentMiterLimit() const { return outputPath->currentMiterLimit; }
	bool	 		isPolygon() const { return outputPath->isPolygon;} // whether current path was closed via closepath or not
	virtual bool	pathsCanBeMerged  (const PathInfo & p1, const PathInfo & p2) const;
	bool			pathWasMerged() const  { return outputPath->pathWasMerged; }
	float           currentLineWidth() const { return outputPath->currentLineWidth; }
	unsigned int    currentNr() const { return outputPath->nr; } 
	float           edgeR() const { return outputPath->edgeR; } // edge colors
	float           edgeG() const { return outputPath->edgeG; }
	float           edgeB() const { return outputPath->edgeB; }
	float           fillR() const { return outputPath->fillR; } // fill colors
	float           fillG() const { return outputPath->fillG; }
	float           fillB() const { return outputPath->fillB; }
	const RSString & currentColorName() const { return outputPath->colorName; }
	const char *    dashPattern() const { return outputPath->dashPattern.c_str(); }
	float           currentR() const { return outputPath->fillR; } // backends that do not support merging 
	float           currentG() const { return outputPath->fillG; } // do not need to differentiate and
	float           currentB() const { return outputPath->fillB; } // can use these functions.
	void			add_to_page();

private:
	// = PRIVATE METHODS

	void 		guess_linetype();

	bool		is_a_rectangle() const;


	// = BACKEND SPECIFIC FUNCTIONS

        //  These next functions are pure virtual and thus need to be implemented for every new backend.

	virtual void    close_page() = 0;
	// writes a trailer whenever a page is finished (triggered by showpage)

	virtual void    open_page() = 0;
	// writes a page header whenever a new page is needed

	virtual void    show_path() = 0;

	void show_or_convert_path();

	void simulate_fill();

	// the next functions are virtual with default implementations

	virtual void    show_text(const TextInfo & textinfo) ;

	virtual void    show_rectangle(
				       const float llx,
				       const float lly,
				       const float urx,
				       const float ury); 
	// writes a rectangle at points (llx,lly) (urx,ury)


	// = INHIBITORS (declared, but not defined)
	drvbase(); // avoid default ctor
	drvbase(const drvbase &);
	drvbase & operator=(const drvbase&);

};

//lint -esym(1712,DashPattern) // no default ctor
class DLLEXPORT DashPattern {
public:
	DashPattern(const char * patternAsSetDashString);
	~DashPattern();
	const RSString dashString;
	int nrOfEntries;
	float * numbers;
	float offset;

private:
	NOCOPYANDASSIGN(DashPattern)
	DashPattern();
};

typedef const char * (*makeColorNameType)(float r, float g, float b);
const unsigned int maxcolors = 10000 ; //  maximum number of colors 

//lint -esym(1712,ColorTable) // no default ctor
class DLLEXPORT ColorTable 
{
public:
	ColorTable(const char * const * defaultColors,
		   const unsigned int numberOfDefaultColors,
		   makeColorNameType makeColorName);
	~ColorTable();
	unsigned int  getColorIndex(float r, float g, float b) ; // non const
	const char *  getColorString(float r, float g, float b); // non const ;
	bool 		  isKnownColor(float r, float g, float b) const;
	const char *  getColorString(unsigned int index) const;
		
private:
	const char * const * const defaultColors_;
	const unsigned int  numberOfDefaultColors_;
	char * newColors[maxcolors];
    const makeColorNameType makeColorName_ ;

	NOCOPYANDASSIGN(ColorTable)
};



#ifdef __TCPLUSPLUS__
// turbo C++ has problems with enum for template parameters
typedef unsigned int Dtype;
const Dtype moveto = 1;
const Dtype lineto = 2;
const Dtype closepath = 3;
const Dtype curveto = 4;
#else
enum  Dtype {moveto, lineto, closepath, curveto};
#endif
// closepath is only generated if backend supportes subpathes
// curveto   is only generated if backend supportes it

//lint -esym(1769,basedrawingelement)
	// default ctor sufficient since no members anyway

class DLLEXPORT basedrawingelement 
{
public:
	// default ctor sufficient since no members anyway
//	basedrawingelement(unsigned int size_p) /*: size(size_p) */ {}
	virtual const Point &getPoint(unsigned int i) const = 0;
	virtual Dtype getType() const = 0;
	friend ostream & operator<<(ostream & out, const basedrawingelement &elem);
	bool operator==(const basedrawingelement& bd2) const;
	virtual unsigned int getNrOfPoints() const = 0;
	virtual basedrawingelement* clone() const = 0; // make a copy
	// deleteyourself is needed because under Windows, the deletion
	// of memory needs to be done by the same dll which did the allocation.
	// this is not simply achieved if plugins are loaded as DLL.
	virtual void deleteyourself() { delete this; } 
	virtual ~basedrawingelement() {}
private:
//	const unsigned int size;
};


inline void copyPoints(unsigned int nr, const Point src[], Point target[])
{
// needed because CenterLine cannot inline for loops
	for (unsigned int i = 0 ; i < nr ; i++ ) target[i] = src[i]; 
}

template <unsigned int nr, Dtype curtype>
class drawingelement : public basedrawingelement
{
public:
// CenterLine !!!!
// "drvbase.h", line 455: sorry, not implemented: cannot expand inline function  drawingelement 
//   <1 , 0 >::drawingelement__pt__19_XCUiL11XC5DtypeL10(Point*) with  for statement in inline

	drawingelement(float x_1, float y_1, float x_2 = 0.0, float y_2 = 0.0, float x_3 = 0.0, float y_3 = 0.0)
	: basedrawingelement()
	{
#if defined (__GNUG__) || defined (_MSC_VER) && _MSC_VER >= 1100
	const Point  p[] = {Point(x_1,y_1),Point(x_2,y_2),Point(x_3,y_3)};
	copyPoints(nr,p,points);
#else
	// Turbo C++ hangs if the other solution is used.
	// and the HP CC compiler does not like it either
	// so use this for all compilers besides GNU and MS VC++
	// This, however, is somewhat slower than the solution above
	Point  * p = new Point[3];
	p[0] = Point(x_1,y_1);
	p[1] = Point(x_2,y_2);
	p[2] = Point(x_3,y_3);
	copyPoints(nr,p,points);
	delete [] p;
#endif

	}

	drawingelement(const Point p[])
	: basedrawingelement()
	{
//	for (unsigned int i = 0 ; i < nr ; i++ ) points[i] = p[i]; 
		copyPoints(nr,p,points);
	}
	drawingelement(const drawingelement<nr,curtype> & orig)
	: basedrawingelement() //lint !e1724 // Argument to copy constructor for class drawingelement<<1>,<2>> should be a const reference
	{ // copy ctor
		if (orig.getType() != curtype ) {
			cerr << "illegal usage of copy ctor of drawingelement" << endl;
			exit(1);
		} else {
			copyPoints(nr,orig.points,points);
		}
	}
	virtual basedrawingelement* clone() const {
		return new drawingelement<nr,curtype>(*this);
	}
	const Point &getPoint(unsigned int i) const  { 
#ifndef _lint
		assert( (i+1) < (nr+1) );
						// nr can be 0 - so unsigned i could never be < 0
					    // but if nr==0, i==0 is also invalid. (logically i has to be <nr)
#endif
		return points[i]; 
	}
	virtual Dtype getType() const 		     { return (Dtype) curtype; }
						// This cast (Dtype) is necessary
						// to eliminate a compiler warning
						// from the SparcCompiler 4.1.
						// although curtype is of type Dtype
	virtual unsigned int getNrOfPoints() const { return nr; }
private:
	Point points[(nr > 0) ? nr : (unsigned int)1]; //lint !e62 //Incompatible types (basic) for operator ':'
	const drawingelement<nr,curtype> &  operator=( const drawingelement<nr,curtype> & rhs ); // not implemented
};


// CenterLine !!!!
// "drvbase.h", line 477: sorry, not implemented: cannot expand inline function  
// drawingelement <3 , 3 >::drawingelement__pt__19_XCUiL13XC5DtypeL13(Point*) with  for statement in inline

#if 0
template <unsigned int nr, Dtype curtype>
inline drawingelement<nr,curtype>::drawingelement(Point p[]) 
	: basedrawingelement()
{
	for (unsigned int i = 0 ; i < nr ; i++ ) points[i] = p[i]; 
}
#endif

typedef drawingelement<(unsigned int) 1,moveto>  	Moveto;
typedef drawingelement<(unsigned int) 1,lineto> 	Lineto;
typedef drawingelement<(unsigned int) 1,closepath> 	Closepath;
typedef drawingelement<(unsigned int) 3,curveto> 	Curveto;


#define derivedConstructor(Class)			\
	Class(const char * driveroptions_p, 	\
              ostream & theoutStream, 			\
              ostream & theerrStream, 			\
              const char* nameOfInputFile_p,	\
              const char* nameOfOutputFile_p,	\
              PsToEditOptions & globaloptions_p, /* non const because driver can also add options */ 		\
              const class DriverDescription & descref)	

// use of static_cast instead of dynamic_cast, because some tools complain about problems then since
// in theory dynamic_cast could return 0
#define constructBase drvbase(driveroptions_p,theoutStream,theerrStream,nameOfInputFile_p,nameOfOutputFile_p,globaloptions_p,descref), options(static_cast<DriverOptions*>(DOptions_ptr))


class DLLEXPORT DescriptionRegister
{
	enum {maxelems = 100 };
public:
	DescriptionRegister() :ind(0) { 
		for (int i = 0; i < maxelems; i++) rp[i] = 0; 
	//	cout << " R constructed " << (void *) this << endl;
	}
#if 0
	// removed - since otherwise one gets a runtime error when the .so is unloaded 
	// (happens with g++ only). This is a noop anyway.
	~DescriptionRegister() {
	//	cout << " R destructed " << (void *) this << endl;
	}
#endif

	static DescriptionRegister& getInstance();

	void registerDriver(DriverDescription* xp);
	void mergeRegister(ostream & out,const DescriptionRegister & src,const char * filename);
	void explainformats(ostream & out,bool withdetails=false) const;
	void listdrivers(ostream &out) const;
	const DriverDescription * getDriverDescForName(const char * drivername) const;
	const DriverDescription * getDriverDescForSuffix(const char * suffix) const;

	DriverDescription* rp[maxelems];

	int nrOfDescriptions() const { return ind; }
private:
	
	int ind;

	NOCOPYANDASSIGN(DescriptionRegister)
};

//extern DLLEXPORT DescriptionRegister* globalRp;
extern "C" DLLEXPORT DescriptionRegister * getglobalRp(void);

//extern __declspec ( dllexport) "C" {
//not needed // DescriptionRegister* getglobalRp();
typedef DescriptionRegister* (*getglobalRpFuncPtr)(void);
//}

//class Rinit 
//{
//public:
//	Rinit() { if (!globalRp) {globalRp = new DescriptionRegister; ref = 1 ; } else { ref++;} }
//	~Rinit() { ref--; if (ref == 0) delete globalRp; }
//
//private:
//	static	int ref;
//};

//static Rinit Rinit_var;

//now in drvdesc.h typedef bool (*checkfuncptr)();
// static bool nocheck() { return true; }

typedef bool (*checkfuncptr)(void);
class drvbase;

struct OptionDescription {
	OptionDescription(const char * n = 0, const char * p = 0, const char * d = 0) :Name(n), Parameter(p), Description(d) {}
	const char * const Name;
	const char * const Parameter;   // e.g. "String" or "numeric", or 0 (implicitly a boolean option then (no argument)
	const char * const Description; // 
private:
//	OptionDescription(const OptionDescription&);
	const OptionDescription& operator=(const OptionDescription&);
// no special copy ctor, assignment op or dtor needed since this class is NOT owner of the (static) strings.
};

// An Array of OptionDescription is delimited by an element where Name is 0
//FIXME const OptionDescription endofoptions(0,0,0);

//FIXME const OptionDescription nodriverspecificoptions[] = {OptionDescription("driver has no further options",0,0),endofoptions};


//lint -esym(1712,DriverDescription) // no default ctor
class DLLEXPORT DriverDescription {
public:
	enum opentype {noopen, normalopen, binaryopen};
	enum imageformat { noimage, png, bmp, eps, memoryeps }; // format to be used for transfer of raster images

	DriverDescription(const char * const s_name, 
			const char * const short_expl, 
			const char * const long_expl,
			const char * const suffix_p, 
			const bool 	backendSupportsSubPathes_p,
			const bool 	backendSupportsCurveto_p,
			const bool 	backendSupportsMerging_p, // merge a separate outline and filling of a polygon -> 1. element
			const bool 	backendSupportsText_p,
			const imageformat  backendDesiredImageFormat_p,
			const opentype  backendFileOpenType_p,
			const bool	backendSupportsMultiplePages_p,
			const bool	backendSupportsClipping_p,
			const bool	nativedriver_p = true,
			checkfuncptr checkfunc_p = 0);
	virtual ~DriverDescription() {
		//		symbolicname = NIL; // these are const
		//		explanation= NIL;
		//		suffix= NIL;
		//		additionalInfo= NIL;
	} //lint !e1540

	virtual drvbase * CreateBackend (const char * const driveroptions_P,
			 ostream & theoutStream, 
			 ostream & theerrStream,   
			 const char* const nameOfInputFile,
		     const char* const nameOfOutputFile,
			 PsToEditOptions & globaloptions_p
			 ) const = 0;
	virtual ProgramOptions * createDriverOptions() const = 0;

	virtual unsigned int getdrvbaseVersion() const { return 0; } // this is only needed for the driverless backends (ps/dump/gs)

	const char * additionalInfo() const;
 // Data members
	const char * const symbolicname;
	const char * const short_explanation;
	const char * const long_explanation;
	const char * const suffix;

	const bool 	backendSupportsSubPathes;
	const bool 	backendSupportsCurveto;
	const bool 	backendSupportsMerging; // merge a separate outline and filling of a polygon -> 1. element
	const bool 	backendSupportsText;
	const imageformat  backendDesiredImageFormat;
	const opentype  backendFileOpenType;
	const bool	backendSupportsMultiplePages;
	const bool	backendSupportsClipping;
	const bool	nativedriver;
	RSString 	filename; 
	// where this driver is loaded from
	// Note: formerly this was a RSString - but that caused problems under X64 / Windows
	// it seems as constructing and deleting heap during start-up phase when not all DLLs are fully initialized
	// can cause these problems (well - I know then order of init among DLLs/SOs is not guaranteed by C++)
	// But the passed strings are temporary - so we need to take a copy to the heap (without destroying it later -> small leak)

	const checkfuncptr checkfunc;

	static const char * currentfilename; // the name of the file from which the plugin is loaded

	NOCOPYANDASSIGN(DriverDescription)
};

class DescriptionRegister;

//lint -esym(1712,DriverDescription*) // no default ctor
template <class T>
class DLLEXPORT DriverDescriptionT : public DriverDescription {
public:
	DriverDescriptionT(const char * s_name, 
			const char * short_expl_p, 
			const char * long_expl_p, 
			const char * suffix_p, 
			const bool 	backendSupportsSubPathes_p,
			const bool 	backendSupportsCurveto_p,
			const bool 	backendSupportsMerging_p, // merge a separate outline and filling of a polygon -> 1. element
			const bool 	backendSupportsText_p,
			const imageformat  backendDesiredImageFormat_p, // supports images from a PNG files
			const DriverDescription::opentype  backendFileOpenType_p,
			const bool	backendSupportsMultiplePages_p,
			const bool	backendSupportsClipping_p,
			const bool  nativedriver_p = true,
			checkfuncptr checkfunc_p = 0 ):
	DriverDescription( 
			s_name, 
			short_expl_p,
			long_expl_p, 
			suffix_p, 
			backendSupportsSubPathes_p,
			backendSupportsCurveto_p,
			backendSupportsMerging_p, 
			backendSupportsText_p,
			backendDesiredImageFormat_p,
			backendFileOpenType_p,
			backendSupportsMultiplePages_p,
			backendSupportsClipping_p,
			nativedriver_p,
			checkfunc_p
			)
		{}
	virtual drvbase * CreateBackend (
			const char * const driveroptions_P,
		    ostream & theoutStream, 
		    ostream & theerrStream,   
			const char* const nameOfInputFile,
	       	const char* const nameOfOutputFile,
			PsToEditOptions & globaloptions_p /* non const because driver can also add arguments */
			 ) const
	{ 
		drvbase * backend = new T(driveroptions_P, theoutStream, theerrStream, nameOfInputFile, nameOfOutputFile, globaloptions_p, *this); 
		return backend;
	} 

	ProgramOptions * createDriverOptions() const {
		// ProgramOptions * p = ;
		// cerr << "creating driver options" <<  (void*) p << endl; 
		return new typename T::DriverOptions;
	}


//	virtual void DeleteBackend(drvbase * & ptr) const { delete (T*) ptr; ptr = 0; }
	virtual unsigned int getdrvbaseVersion() const { return drvbaseVersion; }

private: 
	// typedef DriverDescriptionT<T> SHORTNAME;
	// NOCOPYANDASSIGN(SHORTNAME)
	NOCOPYANDASSIGN(DriverDescriptionT<T>)
};

#if !( (defined (__GNUG__)  && (__GNUC__>=3) && defined (HAVE_STL)) || defined (_MSC_VER) && (_MSC_VER >= 1300) )
// 1300 is MSVC.net (7.0)
// 1200 is MSVC 6.0
//G++3.0 comes with a STL lib that includes a definition of min and max

// some systems have a minmax.h which is indirectly included
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifndef min
template <class T>
inline T min(T x, T y)
{
	return (x<y) ? x:y;
}
#endif

#ifndef max
template <class T>
inline T max(T x, T y)
{
	return (x>y) ? x:y;
}
#endif

#endif


inline float bezpnt(float t, float z1, float z2, float z3, float z4)
{
	// Determine ordinate on Bezier curve at length "t" on curve
	if (t <= 0.0f) {
		return z1; // t = 0.0f;
	}
	if (t >= 1.0f) {
		return z4; // t = 1.0f;
	}
	const float t1 = (1.0f - t);
	return t1 * t1 * t1 * z1 + 3.0f * t * t1 * t1 * z2 + 3.0f * t * t * t1 * z3 + t * t * t * z4;
}

inline Point PointOnBezier(float t, const Point & p1, const Point & p2, const Point & p3, const Point & p4)
{
	return Point(bezpnt(t,p1.x_,p2.x_,p3.x_,p4.x_), bezpnt(t,p1.y_,p2.y_,p3.y_,p4.y_));
}




#endif
 
 
 
 
 
 
 
