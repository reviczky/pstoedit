#ifndef PSTOEDITOPTIONS_H
#define PSTOEDITOPTIONS_H

/*
   pstoeditoptions.h : This file is part of pstoedit
   definition of program options 

   Copyright (C) 1993 - 2005 Wolfgang Glunz, wglunz34_AT_pstoedit.net

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


#include "miscutil.h"
#include "poptions.h"

enum advancedTypeIDs { start=char_ty, rsstring_ty, charstring_ty, constcharstring_ty, argv_ty };

class DLLEXPORT RSStringValueExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, RSString &result) {
	if (instring) {
		result = instring;
		currentarg++;
		return true;
	} else {
		cout << "missing string argument for " << optname << " option" << endl;
		return false;
	}
	}
	static const char *gettypename() { return "string"; }
	static unsigned int gettypeID()  { return rsstring_ty; }
};
#if 0
// not really usable - because when using a Dialog, the char * would afterwards point into the data owned by the dialog
//
class DLLEXPORT charstringValueExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, char* &result) {
	if (instring) {
		result = cppstrdup(instring);
		currentarg++;
		return true;
	} else {
		cout << "missing string argument for " << optname << " option" << endl;
		return false;
	}
	}
	static const char *gettypename() { return "string"; }
	static unsigned int gettypeID() { return charstring_ty; }
};

 
template < >
inline ostream & Option< char *, charstringValueExtractor>::writevalue(ostream & out) const {
		out << (value ? value : "");
		return out;
}

class DLLEXPORT constcharstringValueExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, const char* &result) {
	if (instring) {
		result = instring;
		currentarg++;
		return true;
	} else {
		cout << "missing string argument for " << optname << " option" << endl;
		return false;
	}
	}
	static const char *gettypename() { return "string"; }
	static unsigned int gettypeID() { return constcharstring_ty; }
};

 
template < >
inline ostream & Option< const char *, constcharstringValueExtractor>::writevalue(ostream & out) const {
	out << (value ? value : "");
		return out;
}

#endif

class DLLEXPORT ArgvExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, Argv  &result) {
	if (instring) {
		result.addarg(instring);
		currentarg++;
		return true;
	} else {
		cout << "missing string argument for " << optname << " option" << endl;
		return false;
	}
	}
	static const char *gettypename() { return "string"; }
	static unsigned int gettypeID() { return argv_ty; }
};


static const char * const noArgument = 0;
static const char * const UseDefaultDoku = 0;

class  PsToEditOptions : public ProgramOptions {
public:
	enum PropSheetEnum {g_t=0, t_t, a_t, b_t, d_t, h_t=9999 };
	// g - general
	// t - text
	// a - unused
	// b - debug
	// d - drawing
	// h - hidden
		// cannot be const  because it needs to be changed on non UNIX systems (convertBackSlashes)
	char *nameOfInputFile  ; //= 0;
	char *nameOfOutputFile ; //= 0;	// can contain %d for page splitting

	Option < RSString, RSStringValueExtractor> nameOfIncludeFile ;// = 0;	// name of an option include file
	Option < RSString, RSStringValueExtractor> replacementfont;// = "Courier";
 	Option < bool, BoolTrueExtractor > nomaptoisolatin1 ;//= false;
	Option < bool, BoolTrueExtractor > withdisplay ;//= false;
	Option < bool, BoolTrueExtractor> noquit ;//= false;
	Option < bool, BoolTrueExtractor > nocurves ;//= false;		// normally curves are shown as curves if backend supports
	Option < bool, BoolTrueExtractor > nosubpathes ;//= false;	// normally we use subpathes if the backend support them
	Option < bool, BoolTrueExtractor > mergelines ;//= false;
	Option < bool, BoolTrueExtractor > convertFilledRectToStroke; //= false;
	Option < bool, BoolTrueExtractor > mergetext ;//= false;
	Option < bool, BoolTrueExtractor > drawtext ;//= false;
	Option < bool, BoolTrueExtractor > autodrawtext ;//= false;
	Option < bool, BoolTrueExtractor > disabledrawtext ;//= false;
	Option < bool, BoolTrueExtractor > correctdefinefont ;//= false;

	Option < bool, BoolTrueExtractor > ptioption;
	Option < bool, BoolTrueExtractor > ptaoption;
	unsigned int precisiontext ;//= 0; // derived from the above pti ->1 pta -> 2 sonst 0

	Option < double, DoubleValueExtractor > xscale;
	Option < double, DoubleValueExtractor > yscale;

	Option < bool, BoolTrueExtractor > splitpages ;//= false;
	Option < bool, BoolTrueExtractor > verbose ;//= false;
	Option < bool, BoolTrueExtractor > useBBfrominput; //= false;
	Option < bool, BoolTrueExtractor > simulateSubPaths ;//= false;
	Option < RSString, RSStringValueExtractor> unmappablecharstring ;//= 0;
	Option < bool, BoolTrueExtractor > dontloadplugins ;//= false;
	Option < bool, BoolTrueExtractor > nobindversion ;//= false;	// use old NOBIND instead of DELAYBIND
	Option < int, IntValueExtractor > pagetoextract ;//= 0;		// 0 stands for all pages
	Option < double, DoubleValueExtractor > flatness ;//= 1.0f;		// used for setflat
	Option < bool, BoolTrueExtractor > simulateClipping ;//= false;	// simulate clipping most useful in combination with -dt
	Option < bool, BoolTrueExtractor > usePlainStrings; //= false;
	Option < bool, BoolTrueExtractor > useRGBcolors ;//= false;
	Option < bool, BoolTrueExtractor > noclip ;//= false;
	Option < bool, BoolTrueExtractor > t2fontsast1 ;//= false;	// handle T2 fonts (often come as embedded fonts in PDF files) same as T1
	Option < bool, BoolTrueExtractor > keepinternalfiles ;//= false;
	Option < bool, BoolTrueExtractor > justgstest ;//= false;
	Option < bool, BoolTrueExtractor > pscover ;//= false;
	Option < bool, BoolTrueExtractor > nofontreplacement ;//= false;
	Option < int, IntValueExtractor > rotation ;//= 0;
	Option < RSString, RSStringValueExtractor> explicitFontMapFile ;//= 0;
	Option < RSString, RSStringValueExtractor > outputPageSize;//("");
	Option < bool, BoolTrueExtractor > fromgui;
	Option < bool, BoolTrueExtractor > showdialog;


//	Option < double, DoubleValueExtractor >  magnification ;//= 1.0f;
	Option < bool, BoolTrueExtractor > showdrvhelp ;//= false;
	Option < bool, BoolTrueExtractor > showdocu_long ;//= false;
	Option < bool, BoolTrueExtractor > showdocu_short ;//= false;
	Option < bool, BoolTrueExtractor > dumphelp ;//= false;

	Option < bool, BoolTrueExtractor > backendonly ;//= false;	// used for easier debugging of backends
	// directly read input file by backend
	// bypass ghostscript. The input file
	// is assumed to be produced by a
	// previous call with -f debug

	Option < Argv, ArgvExtractor > psArgs;				// Pass through arguments to PostScript interpreter
	Option < RSString, RSStringValueExtractor> drivername ;//= 0; // cannot be const char * because it is changed in pstoedit.cpp
	
	PsToEditOptions() :


#if 0 

\item[\Opt{-dt}] 
\item[\Opt{-adt}] 
\item[\Opt{-ndt}] 
\item[\Opt{-t2fontsast1}] 
\item[\Opt{-ndt}] 
\item[\Opt{-pti or -pta}]
\item[\Opt{-nfr}] 
\item[\Opt{-dis}] 
\item[\OptArg{-psarg}{~string}] 
\end{description} 
\begin{description}
\item[\Opt{-merge}] 
\item[\OptArg{-page}{~page number}]
\item[\OptArg{-rotate}{~angle (0-360)}]
\item[\Opt{-rgb}] 
\item[\Opt{-split}] 
\item[\OptArg{-uchar}{~character}] 
\item[\Opt{-usebbfrominput}] 
\item[\OptArg{-df}{~fontname}]
\item[\OptArg{-include}{~}] 
\item[\OptArg{-fontmap}{~}] 
\item[\OptArg{-f}{~format}] 
\item[\OptArg{-scale}{~factor}] scale by the specified factor. (Currently used with
\Opt{-f tgif} backend only.) 
\item[\Opt{-ssp}] 
\item[\Opt{-sclip}] 
\item[\OptArg{-pagesize}{string}] 
\item[\Opt{-bo}] 
\item[\OptArg{-flat}{~number}] 
\item[\Opt{-nb}] 
\item[\Opt{-nc}] 
\item[\Opt{-nq}]  
\item[\Opt{-v}] 
\item[\Opt{-nomaptoisolatin1}] 
\item[\Arg{input-file}] input file.  If a "-" is given, standard input is used.
\item[\Arg{output-file}] output file. If no output file or "-" is given as argument,
    \Prog{pstoedit} writes the result to standard output. 
#endif

	nameOfInputFile  (0),
	nameOfOutputFile (0),	// can contain %d for page splitting

	nameOfIncludeFile	(true, "-include","name of a PostScript file to be included",g_t,"name of an option include file" ,
		"This options allows to specify an additional PostScript file that will be "
		"executed just before the normal input is read. This is helpful for "
		"including specific page settings or for disabling potentially unsafe "
		"PostScript operators, e.g., file, renamefile, or deletefile. "	,		
		(const char *) 0),	// 
	replacementfont		(true, "-df","font name",t_t,"default replacement font for raster fonts", 
		" Sometimes fonts embedded in a PostScript "
		"program do not have a fontname. For example, this happens in PostScript "
		"files generated by \\Cmd{dvips}{1}. In such a case \\Prog{pstoedit} uses a "
		"replacement font. The default for this is Courier. Another font can be "
		"specified using the \\Opt{-df} option. \\Opt{-df Helvetica} causes all "
		"unnamed fonts to be replaced by Helvetica. ",
		(const char *) "Courier"),
	nomaptoisolatin1	(true, "-nomaptoisolatin1",noArgument,t_t,"don't map to ISO-Latin 1 encoding" , 
		"Normally \\Prog{pstoedit} maps all character "
		"codes to the ones defined by the ISO-Latin1 encoding. If you specify "
		"\\Opt{-nomaptoisolatin1} then the encoding from the input PostScript is "
		"passed unchanged to the output. This may result in strange text output but on the other hand may be the only way to get some fonts converted appropriately. Try what fits best to your concrete case. ",
		false),
	withdisplay			(true, "-dis",noArgument,b_t,"let GhostScript display the file during conversion" , 
		"Open a display during processing by Ghostscript. Some files "
		"only work correctly this way. ",
		false),
	noquit				(true, "-nq",noArgument,b_t,"don't quit GhostScript after PostScript processing - for debugging only" ,
		"No exit from the PostScript interpreter. Normally Ghostscript "
		"exits after processing the pstoedit input-file. For debugging it can be "
		"useful to avoid this. If you do, you will have to type quit at the "
		"\\verb+GS>+ prompt to exit from Ghostscript.   ",
		false),
	nocurves			(true, "-nc",noArgument,d_t,"normally curves are shown as curves if backend supports. This options forces curves to be always converted to line segments.", 
		"no curves.  \n"
		"Normally pstoedit tries to keep curves from the input and transfers them to "
		"the output if the output format supports curves. If the backend does not "
		"support curves, then pstoedit replaces curves by a series of lines (see "
		"also \\Opt{-flat} option). However, in some cases the user might wish to "
		"have this behavior also for backends that originally support curves. This "
		"can be forced via the \\Opt{-nc} option. ",
		false),		// 
	nosubpathes			(true, "-nsp",noArgument,d_t,"normally subpathes are used if the backend support them. This option turns off subpathes." ,
		UseDefaultDoku,
		false),	// 
	mergelines			(true, "-mergelines",noArgument,d_t,"merge adjacent pathes if one is a stroke and the other is a fill. This depends on the capabilities of the selected backend" ,
		"Some output formats permit the representation of filled "
		"polygons with edges that are in a different color than the fill color. "
		"Since PostScript does not support this by the standard drawing primitives directly, drawing programs typically "
		"generate two objects (the outline and the filled polygon) into the "
		"PostScript output. \\Prog{pstoedit} is able to recombine these, if they "
		"follow each other directly and you specify \\Opt{-mergelines}. "
		"However, this merging is not supported by all backends due to restrictions in the target format.",
		false),
	convertFilledRectToStroke(true,"-filledrecttostroke",noArgument,d_t,"convert filled rectangles to a stroke",
		"Rectangles filled with a solid color can be converted to a stroked line with a width that corresponds to "
		"the width of the rectangle. This is of primary interest for backends which do not "
		"support filled polygons at all. But it is restricted to rectangles only, i.e. it is not "
		"supported for general polygons",false),
	mergetext			(true, "-mergetext",noArgument,d_t,"merge adjacent text pieces" ,
		"In order to produce nice looking text output, programs producing PostScript files "
		"often split words into smaller pieces which are then placed individually on adjacent positions. "
		"However, such split text is hard to edit later on and hence it is sometime better "
		"to recombine these pieces again to form a word (or even sequence of words). "
		"For this pstoedit implements some heuristics about what text pieces are to be "
		"considered parts of a split word. This is based on the geometrical proximity of the different "
		"parts and seems to work quite well so far. But there are certainly cases "
		"where this simple heuristic fails. So please check the results carefully.",
		false),
	drawtext			(true, "-dt",noArgument,t_t,"draw text, i.e. convert text to polygons" , 
		"Draw text - Text is drawn as polygons. This might produce a large output file. This option is automatically "
		"switched on if the selected backend does not support text, e.g. "
		"\\Cmd{gnuplot}{1}.", 
		false),
	autodrawtext		(true, "-adt",noArgument,t_t,"automatic draw text. This draws text only for text that uses fonts with non standard encodings" , 
	"Automatic Draw text - This option turns on the \\Opt{-dt} option selectively for fonts that seem to be no normal text fonts, e.g. Symbol..",
		false),
	disabledrawtext		(true, "-ndt",noArgument,t_t,"fully disable any \"intelligence\" for drawing text" , 
		"Never Draw text -  fully disable the heuristics used by pstoedit to decide when to \"draw\" text "
		"instead of showing it as text. This may produce incorrect results, but in some cases it might nevertheless be useful. \"Use at own risk\". ", 
		false),

	correctdefinefont	(true, "-correctdefinefont",noArgument,t_t,"apply some \"corrective\" actions to definefont - use this for ChemDraw generated PostScript files", 
		"Some PostScript files, e.g. such as generated by ChemDraw, "
		"use the PostScript definefont operator in way that is incompatible with "
		"pstoedit's assumptions. The new font is defined by copying an old font "
		"without changing the FontName of the new font. When this option is applied, "
		"some \"patches\" are done after a definefont in order to make it again "
		"compatible with pstoedit's assumptions. This option is not enabled per default, "
		"since it may break other PostScript file. It is tested only with ChemDraw generated files.",
		false),

	ptioption			(true, "-pti",noArgument,t_t,"precision text - individual. Places text character by character - but only if non standard chararater widths are used",
		"Precision text - "
     		"Normally a text string is drawn as it occurs in the input file. However, in some situations, this might "
		"produce wrongly positioned characters. This is due to limitiations in most backends of "
		"pstoedit. They cannot represent text with arbitray inter-letter spacing which is easily "
		"possible in PDF and PostScript. "
		"With \\Opt{-pta}, each character of a text string is placed "
		"separately. With \\Opt{-pti}, this is done only in cases when there is a non zero inter-letter spacing. "
		"The downside of \"precision text\" is a bigger file size and hard to edit "
		"text.",
		false),
	ptaoption			(true, "-pta",noArgument,t_t,"precision text - always. Places text character by character",
		"see -pti",
		false),
	precisiontext (0), //= 0; // derived from the above pti ->1 pta -> 2 sonst 0

	xscale				(true, "-xscale","number",g_t,"scale by a factor in x-direction" ,
		UseDefaultDoku,
		1.0),
	yscale				(true, "-yscale","number",g_t,"scale by a factor in y-direction" , 
		UseDefaultDoku,
		1.0),

	splitpages			(true, "-split",noArgument,g_t,"split multipage documents into single pages" ,
		"Create a new file for each page of the input. For this the "
		"output filename must contain a \\%d which is replaced with the current page "
		"number. This option is automatically switched on for backends that don't "
		"support multiple pages within one file, e.g. fig or gnuplot. ",
		false),
	verbose				(true, "-v",noArgument,b_t,"turns on verbose mode", 
		"Switch on verbose mode. Some additional information is shown "
		"during processing. ",
		false),
	useBBfrominput		(true, "-usebbfrominput",noArgument,g_t,"extract BoundingBox from input file rather than determining it during processing", 
		"If specified, pstoedit uses the BoundingBox as is (hopefully) found in the input file instead of one that is calculated by its own. ",
		false),
	simulateSubPaths	(true, "-ssp",noArgument,d_t,"simulate subpaths" , 
		"simulate sub paths. \n"
		"Several backend don't support PostScript pathes containing sub pathes, i.e. "
		"pathes with intermediate movetos. In the normal case, each subpath is "
		"treated as an independent path for such backends. This can lead to bad "
		"looking results. The most common case where this happens is if you use the "
		"\\Opt{-dt} option and show some text with letters like e, o, or b, i.e. "
		"letter that have a \"hole\". When the \\Opt{-ssp} option is set, pstoedit "
		"tries to eliminate these problems. However, this option is CPU time "
		"intensive! ",
		false),
	unmappablecharstring(true, "-uchar","character",t_t,"when a character cannot be mapped to one of the standard encoding vector, use this as replacement" ,
		"Sometimes pstoedit cannot map a character "
		"from the encoding used by the PostScript file to the font encoding of the target "
		"format. In this case pstoedit replaces the input character by a special character "
		"in order to show all the places that couldn't be mapped correctly. The default "
		"for this is a \"\\#\". Using the \\Opt{-uchar} option it is possible to specify another character "
		"to be used instead. If you want to use a space, use -uchar \" \".",
		(const char *) 0),
	dontloadplugins		(true, "-dontloadplugins",noArgument,h_t,"internal option - not relevant for normal user" ,
		UseDefaultDoku,
		false),
	nobindversion		(true, "-nb",noArgument,b_t,"use old NOBIND instead of DELAYBIND - try this if GhostScript has problems", 
		"Since version 3.10 \\Prog{pstoedit} uses the "
		"\\texttt{-dDELAYBIND} option when calling GhostScript. Previously the "
		"\\texttt{-dNOBIND} option was used instead but that sometimes caused "
		"problems if a user's PostScript file overloaded standard PostScript "
		"operator with totally new semantic, e.g. lt for lineto instead of the standard meaning of \"less than\". Using \\Opt{-nb} the "
		"old style can be activated again in case the \\texttt{-dDELAYBIND} gives "
		"different results as before. In such a case please also contact the "
		"author. ",
		false),	// 
	pagetoextract		(true, "-page","page number",g_t,"extract a specific page: 0 means all pages" , 
		"Select a single page from a multi page PostScript or PDF file. ",
		0),		// 0 stands for all pages
	flatness			(true, "-flat","flatness factor",d_t,"the precision use for approximating curves by lines if needed" ,
		"If the backend does not support curves in the way "
		"PostScript does or if the \\Opt{-nc} option is specified, all curves are "
		"approximated by lines. Using the \\Opt{-flat} option one can control this "
		"approximation. This parameter is directly converted to a PostScript "
		"\\textbf{setflat} command. Higher numbers, e.g. 10 give rougher, lower "
		"numbers, e.g. 0.1 finer approximations.  ",
		1.0),		// used for setflat
	simulateClipping	(true, "-sclip",noArgument,d_t,"simulate clipping - probably you need to set this if you use -dt" , 
		"simulate clipping.  \n"
		"Most backends of pstoedit don't have native support for clipping. For that "
		"\\Prog{pstoedit} offers an option to perform the clipping of the graphics "
		"directly without passing the clippath to the backends. However, this "
		"results in curves being replaced by a lot of line segments and thus larger "
		"output files. So use this option only if your output looks different from "
		"the input due to clipping. In addition, this \"simulated clipping\" is not "
		"exactly the same as defined in PostScript. There might be lines drawn at "
		"the double size. Also clipping of text is not supported unless you also use "
		"the \\Opt{-dt} option.  ",
		false),	// simulate clipping
	usePlainStrings		(true, "-ups",noArgument,b_t,"write text as plain string instead of hex string in intermediate format - normally useful for trouble shooting and debugging only." ,
		UseDefaultDoku,
		false),
	useRGBcolors		(true, "-rgb",noArgument,g_t,"use RGB colors instead of CMYK" , 
		"Since version 3.30 pstoedit uses the CMYK colors internally. The -rgb option turns on the old behavior to use RGB values.",
		false),
	noclip				(true, "-noclip",noArgument,g_t,"don't use clipping (relevant only if backend supports clipping at all)" , 
		UseDefaultDoku,
		false),
	t2fontsast1			(true, "-t2fontsast1",noArgument,t_t,"handle T2 fonts (often come as embedded fonts in PDF files) same as T1" , 
		"Handle type 2 fonts same as type 1. Type 2 fonts sometimes occur as "
		"embedded fonts within PDF files. In the default mode, text using such fonts is drawn as polygons "
		"since pstoedit assumes that such a font is not available on the users machine. If this option "
		"is set, pstoedit assumes that the internal encoding follows the same as for a standard font "
		"and generates normal text output. This assumption may not be true in all cases. But it "
		"is nearly impossible for pstoedit to verify this assumption - it would have to do a sort of OCR.",
		false),	// handle T2 fonts (often come as embedded fonts in PDF files) same as T1
	keepinternalfiles	(true, "-keep",noArgument,b_t,"keep the intermediate files produced by pstoedit - for debug purposes only" ,
		UseDefaultDoku,
		false),
	justgstest			(true, "-gstest",noArgument,b_t,"perform a basic test for the interworking with GhostScript" ,
		UseDefaultDoku,
		false),
	pscover				(true, "-pscover",noArgument,h_t,"perform coverage statistics about the pstoedit PostScript proloque - for debug and test only" , 
		UseDefaultDoku,
		false),
	nofontreplacement	(true, "-nfr",noArgument,t_t,"don't replace non standard encoded fonts with a replacement font" , 
		"In normal mode pstoedit replaces bitmap fonts with a font as defined by the \\Opt{-df} option. This is done, because most backends can't handle such fonts. This behavior can be "
		"switched off using the \\Opt{-nfr} option but then it strongly depends on the application reading the the generated file whether the file is usable and correctly interpreted or not. Any problems are then out of control of pstoedit.",
		false),
	rotation			(true, "-rotate","angle (0-360)",g_t,"rotate the image",
		"Rotage image by angle.",
		0),
	explicitFontMapFile	(true, "-fontmap","name of font map file for pstoedit",t_t,"use a font mapping from a file" , 
		"The font map is a "
		"simple text file containing lines in the following format:BREAK  \n"
		" \n\n"
		"\\verb+document_font_name    target_font_name+ BREAK\n"
		"Lines beginning with \\verb+%+ are considerd comments BREAK\n "
		"If a font name contains spaces, use the "
		"\\verb+\"font name with spaces\"+ notation. \n"
		" \n"
		"Each font name found in the document is checked against this mapping and if "
		"there is a corresponding entry, the new name is used for the output. "
		" \n\n"
		"If  the \\Opt{-fontmap} option is not specified, \\Prog{pstoedit} "
		"automatically looks for the file \\emph{drivername}.fmp in the installation "
		"directory and uses that file as a default fontmap file if available. The "
		"installation directory is: "
		" \n\n"
		"\\begin{itemize} \n"
		" \n"
		"\\item Windows: The same directory where the \\Prog{pstoedit} executable is \n"
		"    located  \n"
		" \n"
		"\\item Unix: BREAK \n"
		"$<$\\emph{The directory where the pstoedit executably is located}$>$\\verb+/../lib/+ \n"
		" \n"
		"\\end{itemize} \n"
		" \n"
		"The mpost.fmp in the misc directory of the pstoedit distibution is a sample "
		"map file with mappings from over 5000 PostScript font names to their \\TeX "
		"equivalents. This is useful because MetaPost is frequently used with "
		"\\TeX/\\LaTeX\\ and those programs don't use standard font names. This file and "
		"the MetaPost backend are provided by Scott Pakin "
		"(\\Email{pakin_AT_cs.uiuc.edu}).  "
		" "
		"Another example is wemf.fmp to be used under Windows. See the misc "
		"directory of the pstoedit source distribution. ",
		(const char*) 0),
	outputPageSize		(true, "-pagesize","page format",g_t,"set page size (e.g. a4) - used by TK and libplot backend only",
		"set page size for output medium.  \n"
		"This option sets the page size for the output medium. Currently this "
		"is just used by the libplot backend, but might be used by other  "
		"backends in future. The page size is specified in terms of the usual "
		"page size names, e.g. letter or a4. ",
		""),
	fromgui				(true, "-fromgui",noArgument,h_t,"internal - not for normal user",
		UseDefaultDoku,
		false),
	showdialog			(true, "-showdialog",noArgument,h_t,"internal - not for normal user",
		UseDefaultDoku,
		false),
	//	magnification		(true, "-scale","scale result",1.0), 
	showdrvhelp			(true, "-help",noArgument,g_t,"show the help information",
		UseDefaultDoku,
		false) ,
	showdocu_long			(true, "-doculong",noArgument,h_t,"show help information in TeX format - long version",
		UseDefaultDoku,
		false) ,
	showdocu_short			(true, "-docushort",noArgument,h_t,"show help information in TeX format - short version",
		UseDefaultDoku,
		false) ,
	dumphelp			(true, "-dumphelp",noArgument,h_t,"show all options of all drivers in TeX format",
		UseDefaultDoku,
		false), 
	backendonly			(true, "-bo",noArgument,g_t,"backend only - assumes an input file in the internal dump format, e.g. produced by a previous run using -f dump" , 
		"You can run backend processing only (without the PostScript "
		"interpreter frontend) by first running \\textbf{pstoedit} \\Opt{-f dump} "
		"\\Arg{infile} \\Arg{dumpfile} and then running \\textbf{pstoedit} "
		"\\OptArg{-f}{~format}  \\Opt{-bo} \\Arg{dumpfile} \\Arg{outfile}. ",
		false),	// used for easier debugging of backends
	psArgs				(true, "-psarg","argument string",g_t,"additional arguments to be passed to GhostScript directly",
		"The string given with this option is passed "
		"directly to Ghostscript when Ghostscript is called to process the "
		"PostScript file for \\Prog{pstoedit}. For example:      "
		"\\textbf{-psarg}\\textbf{~\"}\\textbf{-r300x300}\\textbf{\"}. "
		"This causes the resolution to be changed to "
		"300x300 dpi. (With older versions of GhostScript, changing the resolution "
		"this way has an effect only if the \\Opt{-dis} option is given.) "
		" "
		" "
		"You can switch Ghostscript into PostScript Level 1 only mode by  "
		"\\Opt{-psarg \"level1.ps\"}. This can be useful for example if the PostScript file to be "
		"converted uses some Level 2 specific custom color models that are not supported " 
		"by pstoedit. However, this requires that the PostScript program checks for the "
		"PostScript level supported by the interpreter and \"acts\" accordingly. "

		"If you want to pass multiple options to Ghostscript you can use multiple  "
		"-psarg options \\Opt{-psarg opt1} \\Opt{-psarg opt2} \\Opt{-psarg opt2}. "
		"See the GhostScript manual for other possible options. "
		),
	drivername			(false,"-f","\"format[:options]\"",g_t,"target format identifier" , 
		"target output format recognized by "
		"\\Prog{pstoedit}.  Since other format drivers can be loaded dynamically, "
		"type  \\texttt{pstoedit -help} to get a full list of formats. See  "
		"\"Available formats and their specific options \" "
		"below for an explanation of the \\oArg{:options} to "
		"\\Opt{-f} format. If the format option is not given, pstoedit tries to guess the target format "
		" from the suffix of the output filename. However, in a lot of cases, this is not a unique mapping "
		"and hence pstoedit demands the \\Opt{-f} option.",
		( char*)0)
	{

	// nameOfInputFile (0);
	// nameOfOutputFile (0),	// can contain %d for page splitting

#define MAKESTRING(x) #x
#define ADD(x) add(&x,MAKESTRING(x))

	ADD(nameOfIncludeFile);
	ADD(replacementfont);
	ADD(nomaptoisolatin1);
	ADD(withdisplay);
	ADD(noquit);
	ADD(nocurves );
	ADD(nosubpathes);	
	ADD(mergelines);
	ADD(convertFilledRectToStroke);
	ADD(mergetext);
	ADD(drawtext);
	ADD(autodrawtext);
	ADD(disabledrawtext);
	ADD(correctdefinefont);

	ADD(ptioption);
	ADD(ptaoption);
	// ADD(precisiontext (0); //= 0; // derived from the above pti ->1 pta -> 2 sonst 0

	ADD(xscale);
	ADD(yscale);

	ADD(splitpages);
	ADD(verbose );
	ADD(useBBfrominput);
	ADD(simulateSubPaths);
	ADD(unmappablecharstring);
	ADD(dontloadplugins);
	ADD(nobindversion );
	ADD(pagetoextract);	
	ADD(flatness);		
	ADD(simulateClipping);	
	ADD(usePlainStrings);	
	ADD(useRGBcolors);
	ADD(noclip);
	ADD(t2fontsast1);	
	ADD(keepinternalfiles);
	ADD(justgstest);
	ADD(pscover);
	ADD(nofontreplacement);
	ADD(rotation );
	ADD(explicitFontMapFile);
	ADD(outputPageSize);
	ADD(fromgui);
	ADD(showdialog);
//	ADD(magnification); 
	ADD(showdrvhelp) ;
	ADD(showdocu_long) ;
	ADD(showdocu_short) ;
	ADD(dumphelp ); 
	ADD(backendonly);	
	ADD(psArgs);	

	ADD(drivername);
}

	~PsToEditOptions() {
		// delete drivername.value; // this crashes under Windows (heap problem) //lint !e605
		delete nameOfInputFile ;
		delete nameOfOutputFile  ;	
	}

//		AutoDeleter < char >DeleterFordrivername(drivername, true);

};

#endif
 