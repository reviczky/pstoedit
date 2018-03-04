/*
   pstoedit.cpp : This file is part of pstoedit
   main control procedure 

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

#define WITHCLIPSUPPORT 1

#include "cppcomp.h"

#include I_fstream
#include I_stdio
#include I_stdlib
#include I_string_h

#include <assert.h>

#include "pstoeditoptions.h"

#include "pstoedit.h"

// for the DLL export data types (the description struct)
#include "pstoedll.h"

#include "version.h"

#include "drvbase.h"

#include "dynload.h"

#define strequal(s1,s2) (strcmp(s1,s2) == 0)

#ifndef USEPROLOGFROMFILE
#include "pstoedit.ph"
#endif

#include "psfront.h"

#ifdef HAVEDIALOG
#include "pstoeditdialog.h"

// typedef void (pstoeditDialog_func) (void  *); /* in reality the params is is PsToEditOptions * */
typedef void (pstoeditDialog_func) (PsToEditOptions *);
typedef void (setPstoeditDialog_func) (pstoeditDialog_func*);

static pstoeditDialog_func* RunDialog = 0;
extern "C" DLLEXPORT void setPstoeditDialogFunction(pstoeditDialog_func* p)
{
	RunDialog = p;
}

#endif


extern const char *defaultPIoptions(ostream & errstream, int verbose);	// in callgs.cpp

static void writeFileName(ostream & outstream, const char *const filename)
{
	const unsigned int len = strlen(filename);
	for (unsigned int i = 0; i < len; i++) {
		if (filename[i] == '\\') {
			outstream << '/';	// '/' works on DOS also
		} else {
			outstream << filename[i];
		}
	}
}





static int grep(const char *const matchstring, const char *const filename, ostream & errstream)
{
// for call: gsresult = grep("% normal end reached by pstoedit.pro",gsout);
	ifstream inFile;
#if ( defined (__GNUG__) || defined(__mips) )
	inFile.open(filename);		// for some reasons adding flags does not work correctly with  g++
#else
	#ifdef HAVESTL
	// at least MSVC doesn't know nocreate when using STL
		inFile.open(filename, ios::binary );
	#else
		inFile.open(filename, ios::binary | ios::nocreate);
	#endif
#endif

	if (inFile.fail()) {
		errstream << "Could not open file " << filename << " in grep" << endl;
		return 1;				// fail
	} else {
		const size_t matchlen = strlen(matchstring);
		const size_t bufferlen = matchlen + 1;
		// allocate buffer for reading begin of lines
		char *buffer = new char[bufferlen];


		while ((void) inFile.get(buffer, bufferlen, '\n'),
			   // debug            errstream << " read in grep :`" << buffer << "'" << inFile.gcount() << " " << matchlen << " eof:"<< inFile.eof() << endl,
			   !inFile.eof() /* && !inFile.fail() */ ) {
// debug                errstream << " read in grep :`" << buffer << "'" << inFile.gcount() << " " << matchlen << endl;
			// gcount returns int on MSVC 
			// make a temp variable to avoid a compiler warning on MSVC
			// (signed/unsigned comparison)
			const unsigned int inFile_gcountresult = inFile.gcount();
#ifdef HAVESTL
			// Notes regarding ANSI C++ version (from KB)
			// istream::get( char* pch, int nCount, char delim ) is different in three ways: 
			// When nothing is read, failbit is set.
			// An eos is always stored after characters extracted (this happens regardless of the outcome).
			// A value of -1 for nCount is an error.

			// If the line contains just a \n then the failbit
			// is set in the ANSI version

//
// Note: This caused again problems with g++3.0 in non STL mode, so this is avoided now by never writing an empty line
//

			if (inFile_gcountresult == 0) {
				inFile.clear();
			} else
#endif

			if ((inFile_gcountresult == matchlen) && (strcmp(buffer, matchstring) == 0)) {
				delete[]buffer;
				return 0;
			}
			if (inFile.peek() == '\n')
				(void) inFile.ignore();
		}
		delete[]buffer;
	}
	return 1;					// fail
}

// *INDENT-OFF*
class drvNOBACKEND : public drvbase {	// not really needed - just as template argument
public:
	derivedConstructor(drvNOBACKEND);	// Constructor
	~drvNOBACKEND() {
		options=0;
	} // Destructor   
	
	class DriverOptions : public ProgramOptions {
	public:
		DriverOptions() 
		{
		}
	}*options;
	
	bool withbackend() const {
		return false;
	}
private:						// yes these are private, library users should use the public interface
	// provided via drvbase
	void open_page() {
	}
	void close_page() {
	}
	void show_text(const TextInfo & /* textInfo */ ) {
	}
	void show_path() {
	}
	void show_rectangle(const float /* llx */ , const float /* lly */ , const float /* urx */ ,
						const float /* ury */ ) {
	}
};
// *INDENT-ON*

drvNOBACKEND::derivedConstructor(drvNOBACKEND):constructBase
{
}

static DriverDescriptionT < drvNOBACKEND > D_psf("psf", "Flattened PostScript (no curves)",
												 "","fps", 1, 0, 1, 1,  DriverDescription::memoryeps,
												 DriverDescription::normalopen, true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_ps("ps", "Simplified PostScript with curves", "","spsc", 1, 1, 1, 1, 
												DriverDescription::memoryeps, DriverDescription::normalopen, true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_debug("debug", "for test purposes", "","dbg", 1, 1,
												   1, 1, DriverDescription::memoryeps, DriverDescription::normalopen,
												   true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_dump("dump", "for test purposes (same as debug)",
												  "","dbg", 1, 1, 1, 1, DriverDescription::memoryeps,
												  DriverDescription::normalopen, true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_gs("gs",
												"any device that GhostScript provides - use gs:format, e.g. gs:pdfwrite",
												"","gs", 1, 1, 1, 1, DriverDescription::noimage,
												DriverDescription::normalopen, true, true,false);
#if 0
// -88 no longer supported - too lazy to create an own class for the moment.
static const OptionDescription driveroptionsAI[] = {
	OptionDescription("-88",0,"generate AI88 format"),
	endofoptions};
#endif
static DriverDescriptionT < drvNOBACKEND > D_ps2ai("ps2ai",
												   "Adobe Illustrator via ps2ai.ps of GhostScript",
												   "","ai", 1, 1, 1, 1, DriverDescription::noimage,
												   DriverDescription::normalopen, false, true,
												   false);

class Closer {
public:
	Closer():fromgui(false) {
	};
	~Closer() {
		if (fromgui) {
			cerr << "Program finished, please press CR to close window\n";
			(void) cin.get();
		}
	}
	bool fromgui;
};

#if 0
#ifdef _DEBUG

#include <windows.h>
#include <ctype.h>
#include <crtdbg.h>
void checkheap(const char * tracep, const void * pUserData) {
	const int r1 =	 _CrtIsValidHeapPointer(pUserData);
	cout << tracep << r1 << " " << pUserData << endl;
}
#else
void checkheap(const char * tracep, const void * pUserData) {};
#endif
#endif

static void loadpstoeditplugins(const char *progname, ostream & errstream, bool verbose)
{
	static bool pluginsloaded = false;
	if (pluginsloaded)
		return;
	RSString plugindir = getRegistryValue(errstream, "common", "plugindir");
	if (plugindir.value() && strlen(plugindir.value() )) {
		loadPlugInDrivers(plugindir.value(), errstream, verbose);	// load the driver plugins
		pluginsloaded = true;
	}
	// also look in the directory where the pstoedit .exe/dll was found
	char szExePath[1000];
	szExePath[0] = '\0';
	const unsigned long r = P_GetPathToMyself(progname, szExePath, sizeof(szExePath));
	if (verbose)  errstream << "pstoedit : path to myself:" << progname << " " << r << " " << szExePath<< endl;
	char *p = 0;
	if (r && (p = strrchr(szExePath, directoryDelimiter)) != 0) {
		*p = '\0';
		if (strcmp(szExePath, plugindir.value() ? plugindir.value() : "") != 0) {
			loadPlugInDrivers(szExePath, errstream,verbose);
			pluginsloaded = true;
		}
	}
#ifdef PSTOEDITLIBDIR
	// also try to load drivers from the PSTOEDITLIBDIR
	loadPlugInDrivers(PSTOEDITLIBDIR, errstream,verbose);
	pluginsloaded = true;
#endif

	// delete[]plugindir;
}


extern FILE *yyin;				// used by lexer 
						// This has to be declared here because of the extern "C"
						// otherwise we could declare it locally where it is used


static void usage(ostream & outstream, bool forTeX, bool withdetails, bool withcategories = false)
{
	PsToEditOptions dummy;
	if (withcategories) {
		const char * const propSheetNames[] = {
		"General options",
		"Text and font handling related options",
		"About",
		"Debug options",
		"Drawing related options",
		"Hidden options",
		0};
		// PropSheetEnum {g_t, t_t, a_t, b_t, d_t, h_t };
		for (unsigned int sheet = PsToEditOptions::g_t ; sheet <= PsToEditOptions::d_t ; sheet++ ) {
			if (sheet == PsToEditOptions::a_t) continue; // skip "about"
			outstream << "\\subsection{"  << propSheetNames[sheet] << "}" << endl;
			dummy.showhelp(outstream,forTeX,withdetails,sheet);
		} 
		outstream << "\\subsection{Input and outfile file arguments}" << endl;
		outstream << "[ inputfile [outputfile] ] " << endl;
	} else {
		dummy.showhelp(outstream,forTeX,withdetails);
		outstream << "[ inputfile [outputfile] ] " << endl;
	}
}
static void shortusage(ostream & outstream)
{
	// just short version
	outstream << "Usage: \"pstoedit -f format inputfile outputfile\" or run \"pstoedit -help\" to get a complete list available options." << endl;
}

bool f_useCoutForDiag = false; // default is cout - but some clients may redirect it to a file
// Note - this could be seen as a "hack". The better way would be to pass the coutstream as parameter
// but that would cause an incompatible interface change and is not feasible.

extern "C" DLLEXPORT
void useCoutForDiag(int flag) { f_useCoutForDiag = (bool) flag; } 


extern "C" DLLEXPORT
	int pstoedit(int argc, const char *const argv[], ostream & errstream,
				 execute_interpreter_function call_PI,
				 whichPI_type whichPI, const DescriptionRegister * const pushinsPtr)
{

	Closer closerObject;

	PsToEditOptions options;
	ostream& diag = f_useCoutForDiag ? cout : errstream;

#ifdef _DEBUG
	const char buildtype [] = "debug build";
#else
	const char buildtype [] = "release build";
#endif
#ifdef __VERSION__
	#ifdef __GNUG__
		const char compversion [] = "g++ " __VERSION__;
	#else
		const char compversion [] = "unknown compiler " __VERSION__;
        #endif
#else
	#ifdef _MSC_VER
		#define XNUMTOSTRING(x) NUMTOSTRING(x)
		#define NUMTOSTRING(x) #x
		const char compversion [] = "MS VC++ " XNUMTOSTRING(_MSC_VER);
	#elif defined(__SUNPRO_CC)
		#define XNUMTOSTRING(x) NUMTOSTRING(x)
		#define NUMTOSTRING(x) #x
		const char compversion [] = "SUN Studio " XNUMTOSTRING(__SUNPRO_CC);
	#else
		const char compversion [] = "unknown compiler ";
	#endif
#endif


	// int arg = 1;
	drvbase::SetVerbose( false );	// init
	const unsigned int remaining = options.parseoptions(errstream,argc,argv);

	if (!options.quiet) {
		errstream << "pstoedit: version " << version << " / DLL interface " <<
		drvbaseVersion << " (build " << __DATE__ << " - " << buildtype << " - " << compversion << ")" 
		" : Copyright (C) 1993 - 2009 Wolfgang Glunz\n";
	}

	//  handling of derived parameters
	drvbase::SetVerbose(options.verbose);
	closerObject.fromgui = (bool) options.fromgui;
	if (options.ptioption) options.precisiontext = 1;
	if (options.ptaoption) options.precisiontext = 2;

	switch (remaining) {
	case 0: { // use stdin and stdout
		break;
			}
	case 1: { // use file and stdout
		options.nameOfInputFile = cppstrdup(options.unhandledOptions[0]);
		break;
			}
	case 2: { // use both files
		options.nameOfInputFile = cppstrdup(options.unhandledOptions[0] );
		options.nameOfOutputFile = cppstrdup(options.unhandledOptions[1] );
		break;
			}
	default:{
		diag << "more than two file arguments " << endl;
		shortusage(diag);
		return 1;
			}
	}

#ifdef HAVEDIALOG
	// setPstoeditDialogFunction(rundialog);
	
	if (options.showdialog) {
		static initdone = false;
		// the next code is needed for a full stand-alone pstoedit.exe.
		if (!initdone) {
			if ( !AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
			{
				// TODO: change error code to suit your needs
				_tprintf(_T("Fatal Error: MFC initialization failed\n"));
				//nRetCode = 1;
			} 
			initdone = true;
		}
		rundialog(options);
#if 0
		if (RunDialog) {
			RunDialog(&options);
		} else {
			cerr << "Dialoghandler is not defined - cannot show dialog" << endl;
		}
#endif
	}
#endif

	// options.showvalues(cout);

//  obsolete now because of <driver>.fmp look-up
//  char * fontmapfile = getRegistryValue(errstream,"common","fontmap");
//  if (fontmapfile != 0) drvbase::theFontMapper.readMappingTable(errstream,fontmapfile);

	//
	// if input or output filename is -, then set it back to 0 in order
	// to keep the logic below unchanged
	//
	if (options.nameOfInputFile && strequal(options.nameOfInputFile, "-")) {
		delete[]options.nameOfInputFile;
		options.nameOfInputFile = 0;
	}
	if (options.nameOfOutputFile && strequal(options.nameOfOutputFile, "-")) {
		delete[]options.nameOfOutputFile;
		options.nameOfOutputFile = 0;
	}

	if (!options.dontloadplugins) {
		loadpstoeditplugins(argv[0], errstream, options.verbose);	// load the driver plugins
	}

	if ((pushinsPtr != 0) && (pushinsPtr != getglobalRp())) {
		getglobalRp()->mergeRegister(errstream, *pushinsPtr, "push-ins");
	}

	if (options.showdocu_short) {
		// show general options
		usage(diag,true,false);
		return 1;
	}
	if (options.showdocu_long) {
		// show general options
		usage(diag,true,true,true);
		return 1;
	}
	if (options.dumphelp) {
		// show driver specific options
		getglobalRp()->explainformats(diag,true);
		return 1;
	}
	if (options.showdrvhelp) {
		usage(diag,false,false);
		const char *gstocall = whichPI(diag, options.verbose, options.gsregbase.value.value(),options.GSToUse.value.value());
		if (gstocall != 0) {
			diag << "Default interpreter is " << gstocall << endl;
		}
		getglobalRp()->explainformats(diag);
		return 1;
	}

	if (options.justgstest) {
		const char *gstocall = whichPI(errstream, options.verbose, options.gsregbase.value.value(),options.GSToUse.value.value());
		if (gstocall == 0) {
			return 3;
		}
		Argv commandline;
		commandline.addarg(gstocall);
// rcw2: Current RiscOS port of gs can't find its fonts without this...
#ifdef riscos
		commandline.addarg("-I<GhostScript$Dir>");
#endif
		const char *pioptions = defaultPIoptions(errstream, options.verbose);
		if (pioptions && (strlen(pioptions) > 0)) {
			commandline.addarg(pioptions);
		}

		if (!options.verbose)
			commandline.addarg("-q");

		if (options.nobindversion) {
			// NOBIND disables bind in, e.g, gs_init.ps
			// these files are loaded before pstoedit.pro
			// so any already bound call to, e.g., show could
			// not be intercepted by pstoedit's show

			commandline.addarg("-dNOBIND");
		} else {
			commandline.addarg("-dDELAYBIND");
		}
		commandline.addarg("-dWRITESYSTEMDICT");


		if (options.verbose) {
			commandline.addarg("-dESTACKPRINT");
			// commandline.addarg("-sDEBUG=true");
		}
		if (options.withdisplay) {
			commandline.addarg("-dNOPAUSE");
		} else {
			commandline.addarg("-dNODISPLAY");
		}
		commandline.addarg("-dNOEPS"); // otherwise EPSF files create implicit showpages and a save/restore pair which disturbs the setPageSize handling
// from the gs (8.0) news:
/*
Ghostscript now automatically encapsulates EPS files, so that EPS files will display and print/convert correctly. Previously EPS files without a showpage would display, but no output file would be written when converting to a bitmap. 
To set the page size to match the EPS bounding box, use -dEPSCrop. To scale an EPS to fit the current page size, use -dEPSFitPage. If neither of these options are used and the EPS file does not fit within the current page, a warning will be generated. 
To get the pre 8.00 behaviour, either use -dNOEPS or run the file with (filename) (r) file cvs exec not (filename) run. 
*/


		for (unsigned int psi = 0; psi < options.psArgs().argc; psi++) {
			commandline.addarg(options.psArgs().argv[psi]);
		}
		if (options.nameOfInputFile) {
			commandline.addarg(options.nameOfInputFile);
		}
		if (!options.verbose)
			commandline.addarg("quit.ps");
		//if (verbose)
		errstream << "now calling the interpreter via: " << commandline << endl;
		// gsresult = system(commandline);
		const int gsresult = call_PI(commandline.argc, commandline.argv);
		if (gsresult != 0) {
			errstream << "PostScript/PDF Interpreter finished. Return status " << gsresult 
				<< " executed command : " << commandline << endl;
		}
		return gsresult;
	}
	if (options.drivername.value.value() == 0) {
		// try to find driver according to suffix of input file
		const char * suffixOfInputFile = 0;
		if (!options.nameOfOutputFile) {
			diag << "No output format specified (-f option) and format could not be deduced from suffix of output file since no output file name was given" << endl;
			shortusage(diag);
			return 1;
		} else {
			suffixOfInputFile = strrchr(options.nameOfOutputFile,'.');
			if (!suffixOfInputFile) {
				diag << "No output format specified (-f option) and format could not be deduced from suffix of output file since no suffix was found" << endl;
				shortusage(diag);
				return 1;
			} else {
				const DriverDescription *suffixDriverDesc = getglobalRp()->getDriverDescForSuffix((suffixOfInputFile+1)); // +1 == skip "."
				if (suffixDriverDesc) {
					errstream << "No explicit output format specified - using " << suffixDriverDesc->symbolicname << " as derived from suffix of output file" << endl;
					options.drivername = suffixDriverDesc->symbolicname;
				} else {
					diag << "No output format specified (-f option) and format could not be uniquely deduced from suffix " << suffixOfInputFile << " of output file" << endl;
					// usage(errstream);
					getglobalRp()->explainformats(diag); // ,true);
					return 1;
				}
			}
		}
	}
	// from here on options.drivername is != 0
	{
		auto_ptr<char> driveroptionscopy (cppstrdup(options.drivername.value.value()) );
		char *driveroptions = strchr(driveroptionscopy.operator->(), ':');
		if (driveroptions) {
			*driveroptions = '\0';	// replace : with 0 to separate drivername
			options.drivername = driveroptionscopy.operator->();
			driveroptions++;
		}
		const DriverDescription *currentDriverDesc = getglobalRp()->getDriverDescForName(options.drivername.value.value());
		if (currentDriverDesc == 0) {
			diag << "Unsupported output format " << options.drivername.value.value() << endl;
			getglobalRp()->explainformats(diag);
			return 1;
		}

		if ( currentDriverDesc->backendFileOpenType!=DriverDescription::normalopen && !options.nameOfOutputFile ) {
			diag << "The driver for the selected format cannot write to standard output because it writes binary data" << endl;
			return 1;
		}


		if (driveroptions && (strcmp(driveroptions, "-help") == 0)) {
			ProgramOptions* dummy = currentDriverDesc->createDriverOptions();
			if (dummy->numberOfOptions() ) {
				diag << "The driver for this output format supports the following additional options: (specify using -f \"format:-option1 -option2\")" << endl;
				dummy->showhelp(diag,false,false);
			}
			delete dummy;
			return 1;
		}
		if (strcmp(options.drivername.value.value(), "gs") == 0) {
// TODO:
			// Check for input file (exists, or stdin) stdout handling
			if (!options.nameOfInputFile) {
				diag << "Cannot read from standard input if GS drivers are selected" << endl;
				return 1;
			}
			// an input file was given as argument

			// just test whether InputFile is readable.
			// The file will be read directly from the PostScript
			// interpreter later on
			convertBackSlashes(options.nameOfInputFile);

			if (!fileExists(options.nameOfInputFile)) {
				diag << "Could not open file " << options.nameOfInputFile << " for input" << endl;
				return 1;
			}

			if (!options.nameOfOutputFile) {
				diag <<
					"Cannot write to standard output if GS drivers are selected" << endl;
				return 1;
			}

			if (options.backendonly) {
				diag << "The -bo option cannot be used if GS drivers are selected " << endl;
				return 1;
			}

			if (!driveroptions) {
				diag <<
					"The gs output driver needs a gs-device as argument, e.g. gs:pdfwrite" << endl;
				return 1;
			}
			// special handling of direct ghostscript drivers
			Argv commandline;
//          char commandline[1000];
			// TODO check for overflow
//          commandline[0]= '\0';
			const char *gstocall = whichPI(errstream, options.verbose, options.gsregbase.value.value(),options.GSToUse.value.value());
			if (gstocall == 0) {
				return 3;
			}
			commandline.addarg(gstocall);
// rcw2: Current RiscOS port of gs can't find its fonts without this...
#ifdef riscos
			commandline.addarg("-I<GhostScript$Dir>");
#endif
			if (!options.verbose)
				commandline.addarg("-q");

			const char *pioptions = defaultPIoptions(errstream, options.verbose);
			if (pioptions && (strlen(pioptions) > 0)) {
				commandline.addarg(pioptions);
			}
			commandline.addarg("-dNOPAUSE");
			commandline.addarg("-dBATCH");
			RSString tempbuffer("-sDEVICE=") ; // char tempbuffer[1000];
			// tempbuffer[0] = '\0';
			// tempbuffer += "-sDEVICE="; // strcat_s(tempbuffer, 1000, "-sDEVICE=");
			tempbuffer += driveroptions; // strcat_s(tempbuffer, 1000,driveroptions);	// e.g., pdfwrite ;
			commandline.addarg(tempbuffer.value() );
			for (unsigned int psi = 0; psi < options.psArgs().argc; psi++) {
				commandline.addarg(options.psArgs().argv[psi]);
			}
			tempbuffer = "-sOutputFile=";
			tempbuffer +=  options.nameOfOutputFile; 
			// strcat_s(tempbuffer,1000, "-sOutputFile=");
			// strcat_s(tempbuffer,1000, options.nameOfOutputFile);
			commandline.addarg(tempbuffer.value());
			commandline.addarg("-c");
			commandline.addarg("save");
			commandline.addarg("pop");
			commandline.addarg("-f");
			commandline.addarg(options.nameOfInputFile);
			if (options.verbose)
				errstream << "Now calling the interpreter via: " << commandline << endl;
			// gsresult = system(commandline);
			const int gsresult = call_PI(commandline.argc, commandline.argv);
			if (gsresult != 0) {
				errstream << "PostScript/PDF Interpreter finished. Return status " << gsresult 
					<< " executed command : " << commandline << endl;
			}
			return gsresult;
		} else {
			//istream *inputFilePtr = 0;
			// cannot be const because nameOfInputFile cannot be const
			// because it needs to be changed on non UNIX systems (convertBackSlashes)
			const char stdinFileName[] = "%stdin";
			if (options.nameOfInputFile) {
				// an input file was given as argument

				// just test whether InputFile is readable.
				// The file will be read directly from the PostScript
				// interpreter later on
				convertBackSlashes(options.nameOfInputFile);
				if (!fileExists(options.nameOfInputFile)) {
					errstream << "Could not open file " << options.nameOfInputFile << " for input" << endl;
					return 1;
				}
			} else {
				options.nameOfInputFile = cppstrdup(stdinFileName);
			}
			ostream *outputFilePtr = 0;
			ofstream outFile;
			drvbase *outputdriver = 0;
			char *nameOfOutputFilewithoutpercentD = 0;

			{
				// setup pstoeditHome directory
				char szExePath[1000];
				szExePath[0] = '\0';
				const int r = P_GetPathToMyself(argv[0], szExePath, sizeof(szExePath));
				if (r && options.verbose) {
					errstream << "path to myself: " << szExePath << endl;
				}
				char *p;
				if (r && (p = strrchr(szExePath, directoryDelimiter)) != 0) {
					*p = '\0';
					drvbase::pstoeditHomeDir() = RSString(szExePath);
				} else {
					drvbase::pstoeditHomeDir() = "";
				}
				if (options.verbose)  errstream << "pstoedit home directory : " << drvbase::pstoeditHomeDir().value() << endl;

				drvbase::pstoeditDataDir() = drvbase::pstoeditHomeDir();

#if (defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined (NetBSD) ) && !defined(DJGPP)
#ifdef PSTOEDITDATADIR
		// usually something like /usr/share/pstoedit
				drvbase::pstoeditDataDir() = PSTOEDITDATADIR;
#else
				drvbase::pstoeditDataDir() +=  "/../share/pstoedit";
#endif
#endif

				if (options.verbose)  errstream << "pstoedit data directory : " << drvbase::pstoeditDataDir().value() << endl;
			}


			if (options.nameOfOutputFile) {
				options.splitpages = options.splitpages || (!currentDriverDesc->backendSupportsMultiplePages);
				// if -split is given or if backend does not support multiple pages
				// BUT, do this only if a real output file is given, not when writing to stdout.
				outputFilePtr = &outFile;
				convertBackSlashes(options.nameOfOutputFile);
				nameOfOutputFilewithoutpercentD = cppstrdup(options.nameOfOutputFile, 20);	// reserve 20 chars for page number
				sprintf_s(TARGETWITHLEN(nameOfOutputFilewithoutpercentD, strlen(options.nameOfOutputFile)+ 20) , options.nameOfOutputFile, 1);	//first page is page 1
				if (currentDriverDesc->backendFileOpenType != DriverDescription::noopen) {
					if (currentDriverDesc->backendFileOpenType == DriverDescription::binaryopen) {
#if (defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined(__EMX__) || defined (NetBSD)  ) && !defined(DJGPP)
// binary is not available on UNIX, only on PC
						outFile.open(nameOfOutputFilewithoutpercentD, ios::out);
#else
						// use redundant ios::out because of bug in djgpp
						outFile.open(nameOfOutputFilewithoutpercentD, ios::out | ios::binary);
#endif
						// errstream << "opened " << newname << " for binary output" << endl;
					} else {
						outFile.open(nameOfOutputFilewithoutpercentD);
						// errstream << "opened " << newname << " for output" << endl;
					}
					if (outFile.fail()) {
						errstream << "Could not open file " << nameOfOutputFilewithoutpercentD <<
							" for output" << endl;
						return 1;
					}			// fail
				}				// backend opens file by itself
				outputdriver =
					currentDriverDesc->CreateBackend(driveroptions,
													 *outputFilePtr,
													 errstream,
													 options.nameOfInputFile,
													 nameOfOutputFilewithoutpercentD,
													 options);
			} else {
				if (options.splitpages) {
					errstream << "Cannot split pages if output goes to standard output" << endl;
					return 1;
				} else {
					outputFilePtr = &cout;
					outputdriver =
						currentDriverDesc->CreateBackend(driveroptions,
														 *outputFilePtr,
														 errstream,
														 options.nameOfInputFile,
														 0, 
														 options);
				}
			}

			if ((!outputdriver) || (outputdriver && (!outputdriver->driverOK()))) {
				errstream << "Creation of driver failed " << endl;
				return 1;
			}


			if (options.explicitFontMapFile.value.value()) {
				if (fileExists(options.explicitFontMapFile.value.value())) {
					if (options.verbose) {
						errstream << "Loading fontmap from " << options.explicitFontMapFile.value << endl;
					}
					drvbase::theFontMapper().readMappingTable(errstream, options.explicitFontMapFile.value.value());
				} else {
					RSString extendedFontMapFile = options.explicitFontMapFile.value;
					extendedFontMapFile += ".fmp";
					if (fileExists(extendedFontMapFile.value())) {
						if (options.verbose) {
							errstream << "Loading fontmap from " << extendedFontMapFile << endl;
						}
						drvbase::theFontMapper().readMappingTable(errstream, options.explicitFontMapFile.value.value());
					} else {
						errstream << "Warning: Couldn't open fontmap file. Neither " <<
						options.explicitFontMapFile.value << " nor " << extendedFontMapFile << ". Option ignored." << endl;
					}
				}
			} else {
				// look for a driver specific fontmap 
				// also look in the directory where the pstoedit .exe/dll was found

				/*
				   char szExePath[1000];
				   szExePath[0] = '\0';
				   const int r = P_GetPathToMyself(argv[0], szExePath,
				   sizeof(szExePath));
				   if (r && verbose) {
				   errstream << " path to myself: " << szExePath << endl;
				   }
				   char *p;
				 */
				if (drvbase::pstoeditDataDir() != "") {
					// *p = '\0';
					RSString test(drvbase::pstoeditDataDir());
					test += directoryDelimiter;
					test += options.drivername.value;
					test += ".fmp";
					// errstream << test.value() << endl;
					if (fileExists(test.value())) {
						if (options.verbose) {
							errstream <<
								"loading driver specific fontmap from " << test.value() << endl;
						}
						drvbase::theFontMapper().readMappingTable(errstream, test.value());
					}
				}
			}

			RSString gsoutName;
			RSString gsout;
			int gsresult = 0;
			if (options.backendonly) {
				if ( outputdriver && !outputdriver->withbackend() ) {
					errstream << "the -bo option cannot be used with this non native pstoedit driver" << endl;
					return 1;
				}
				gsout = options.nameOfInputFile;
				gsresult = 0;	// gs was skipped, so there is no problem
			} else {
				RSString gsin = full_qualified_tempnam("psin");
				const char *successstring;	// string that indicated success of .pro
				ofstream inFileStream(gsin.value());
				inFileStream << "/pagetoextract " << options.pagetoextract << " def" << endl;
				if (options.nomaptoisolatin1) {
					inFileStream << "/maptoisolatin1 false def" << endl;
				}
				if (options.usePlainStrings) {
					inFileStream << "/pstoedit.ashexstring false def" << endl;
				}
				if (options.t2fontsast1) {
					inFileStream << "/t2fontsast1 true def" << endl;
				} else {
					inFileStream << "/t2fontsast1 false def" << endl;
				}
				if (options.useRGBcolors) {
					inFileStream << "/pstoedit.useRGBcolors true def" << endl;
				}
#if WITHCLIPSUPPORT
				if (currentDriverDesc->backendSupportsClipping && !(options.noclip) ) {
					inFileStream << "/pstoedit.dumpclippath true def" << endl;
				}
#endif
				if (options.verbose) {
					inFileStream << "/pstoedit.verbosemode true def" << endl;
				}
				if (options.nofontreplacement) {
					inFileStream << "/pstoedit.checkfontnames false def " << endl;
				}
				if (options.passglyphnames) {
					inFileStream << "/pstoedit.passglyphnames true def " << endl;
				}
				if (options.useoldnormalization) {
					inFileStream << "/pstoedit.useoldnormalization true def " << endl;
				}
				if (options.pscover) {
					inFileStream << "/pstoedit.withpscover true def" << endl;
				}
				if (options.flatness != 1.0) {
					inFileStream << "/pstoedit.flatnesstouse " << options.flatness << " def" << endl;
				}
				if (options.drawtext || (!currentDriverDesc->backendSupportsText)) {
					inFileStream << "/pstoedit.textastext false def" << endl;
				} else {
					inFileStream << "/pstoedit.textastext true def" << endl;
				}
				if (options.disabledrawtext) {
					inFileStream << "/pstoedit.disabledrawtext true def" << endl;
					// if true, this is dominant over -dt or -adt
				}
				if (options.autodrawtext) {
					inFileStream << "/pstoedit.autodrawtext true def" << endl;
				}
				if (options.correctdefinefont) {
					inFileStream << "/pstoedit.correctdefinefont true def" << endl;
				}
				if (options.unmappablecharstring.value.value() && strlen(options.unmappablecharstring.value.value())) {
					inFileStream << "/pstoedit.globalunmappablecharacter (" <<
						options.unmappablecharstring.value.value()[0] << ") def" << endl;
				}
				inFileStream << "/pstoedit.precisiontext " << options.precisiontext << " def" << endl;

				if (options.simulateClipping) {
					inFileStream << "/pstoedit.simulateclipping true def" << endl;
				}
				if (options.useAGL) {
					inFileStream << "/pstoedit.useAGL true def" << endl;
				}

				inFileStream << "/pstoedit.rotation " << options.rotation << " def" << endl;
				inFileStream << "/pstoedit.xscale " << options.xscale << " def" << endl;
				inFileStream << "/pstoedit.yscale " << options.yscale << " def" << endl;
				inFileStream << "/pstoedit.xshift " << options.xshift << " def" << endl;
				inFileStream << "/pstoedit.yshift " << options.yshift << " def" << endl;
				if (options.centered) { inFileStream << "/pstoedit.centered " << "true" << " def" << endl; }
	
				if (strequal(options.drivername.value.value(), "ps")
					|| strequal(options.drivername.value.value(), "psf")) {
					inFileStream << "/pstoedit.escapetext true def" << endl;
				}
				if ((strequal(options.drivername.value.value(), "debug")) || (strequal(options.drivername.value.value(), "pdf"))) {
					inFileStream << "/pstoedit.usepdfmark true def" << endl;
				}
 
				inFileStream << "/pstoedit.replacementfont (" << options.replacementfont << ") def" << endl;
				gsout = gsoutName = full_qualified_tempnam("psout");
				assert((gsin != gsout)
					   && ("You seem to have a buggy version of tempnam" != 0));
				// tempnam under older version of DJGPP are buggy
				// see search for BUGGYTEMPNAME in this file !! 
				if (options.nameOfOutputFile) {
					inFileStream << "/pstoedit.targetoutputfile (";
					writeFileName(inFileStream, options.nameOfOutputFile);
					inFileStream << ") def" << endl;
					inFileStream << "/pstoedit.nameOfOutputFilewithoutpercentD (";
					writeFileName(inFileStream, nameOfOutputFilewithoutpercentD);
					inFileStream << ") def" << endl;
				} else {
					inFileStream << "/pstoedit.redirectstdout true def" << endl;
					inFileStream << "/pstoedit.targetoutputfile (";
					writeFileName(inFileStream, "stdout");
					inFileStream << ") def" << endl;
				}
				inFileStream << "/pstoedit.outputfilename (";
				writeFileName(inFileStream, gsout.value());
				inFileStream << ") def" << endl;

				inFileStream << "/pstoedit.inputfilename  (";
				writeFileName(inFileStream, options.nameOfInputFile);
				inFileStream << ") def" << endl;

				if (options.nameOfIncludeFile.value.value()) {
					ifstream filetest(options.nameOfIncludeFile.value.value());
					if (!filetest) {
						errstream << "Could not open file " <<
							options.nameOfIncludeFile << " for inclusion" << endl;
						return 1;
					}
					filetest.close();
					inFileStream << "/pstoedit.nameOfIncludeFile  (";
					writeFileName(inFileStream, options.nameOfIncludeFile.value.value());
					inFileStream << ") def" << endl;
				}

				switch (currentDriverDesc->backendDesiredImageFormat) {
					case DriverDescription::noimage : 
							break;
					case DriverDescription::bmp : 
							inFileStream << "/pstoedit.backendSupportsFileImages true def" << endl;
							inFileStream << "/pstoedit.withimages true def" << endl;
							inFileStream << "/pstoedit.imagedevicename (bmp16m) def" << endl;
							inFileStream << "/pstoedit.imagefilesuffix (.bmp)   def" << endl; 
							break;
					case DriverDescription::png : 
							inFileStream << "/pstoedit.backendSupportsFileImages true def" << endl;
							inFileStream << "/pstoedit.withimages true def" << endl;
							inFileStream << "/pstoedit.imagedevicename (png16m) def" << endl;
							inFileStream << "/pstoedit.imagefilesuffix (.png)   def" << endl; 
							break;
					case DriverDescription::eps : 
							inFileStream << "/pstoedit.backendSupportsFileImages true def" << endl;
							inFileStream << "/pstoedit.withimages true def" << endl;
							inFileStream << "/pstoedit.imagedevicename (epswrite) def" << endl;
							inFileStream << "/pstoedit.imagefilesuffix (.eps)   def" << endl; 
							break;
					case DriverDescription::memoryeps : 
							inFileStream << "/pstoedit.withimages true def" << endl;
							break;
					default:
							break;
				}
				if ((currentDriverDesc->backendDesiredImageFormat != DriverDescription::noimage) &&
					(currentDriverDesc->backendDesiredImageFormat != DriverDescription::memoryeps) && 
					(!options.nameOfOutputFile) ) {
						errstream <<
							"Warning: some types of raster images in the input file cannot be converted if the output is sent to standard output"
							<< endl;
				}
				if ((!options.nocurves)
					&& currentDriverDesc->backendSupportsCurveto) {
					inFileStream << "/pstoedit.doflatten false def" << endl;
				} else {
					inFileStream << "/pstoedit.doflatten true def" << endl;
				}
				if (!options.noquit) {
					inFileStream << "/pstoedit.quitprog { quit } def" << endl;
				} else {
					inFileStream << "/pstoedit.quitprog { } def" << endl;
				}
				if (options.nobindversion) {
					inFileStream << "/pstoedit.delaybindversion  false def" << endl;
				} else {
					inFileStream << "/pstoedit.delaybindversion  true def" << endl;
				}
				delete[]nameOfOutputFilewithoutpercentD;
				if (outputdriver && (outputdriver->knownFontNames() != 0)) { 
					const char *const *fnames = outputdriver->knownFontNames();
					unsigned int size = 0;
					while (*fnames) {
						size++;
						fnames++;
					}
					inFileStream << "/pstoedit.knownFontNames " << size << " dict def" << endl;

					inFileStream << "pstoedit.knownFontNames begin" << endl;
					fnames = outputdriver->knownFontNames();
					while (*fnames) {
						inFileStream << "/" << *fnames << " true def" << endl;
						fnames++;
					}
					inFileStream << "end" << endl;
				}
				if (strcmp(options.drivername.value.value(), "ps2ai") == 0) {
					successstring = "%EOF";	// This is written by the ps2ai.ps 
					// showpage in ps2ai does quit !!!
					// ps2ai needs special headers
					inFileStream << "/pstoedit.preps2ai where not { \n"
						// first run through this file (before ps2ai.ps)
						"	/jout true def \n" "	/joutput pstoedit.outputfilename def \n";
					if (driveroptions && (strcmp(driveroptions, "-88") == 0)) {
						inFileStream << "	/jtxt3 false cdef\n";
					}
					inFileStream << "	/pstoedit.textastext where { pop pstoedit.textastext not {/joutln true def } if } if \n" "	/pstoedit.preps2ai false def \n" "}{ \n" "	pop\n"	// second run (after ps2ai.ps)
						"	pstoedit.inputfilename run \n"
						//  "   (\\" << successstring << "\\n) jp" 
						//  "   pstoedit.quitprog \n"
						"   showpage \n"
						"} ifelse \n" << endl;

				} else {
					successstring = "% normal end reached by pstoedit.pro";
#ifdef	USEPROLOGFROMFILE
					ifstream prologue("pstoedit.pro");
					//     errstream << " copying prologue file to " << gsin << endl;
					copy_file(prologue, inFileStream);
#else
					const char *const *prologueline = PS_prologue;
					while (prologueline && *prologueline) {
						inFileStream << *prologueline << '\n';
						prologueline++;
					}
#endif
				}
				inFileStream.close();
				// now call ghostscript

				Argv commandline;
				const char *gstocall = whichPI(errstream, options.verbose, 
				  options.gsregbase.value.value(),options.GSToUse.value.value());
				if (gstocall == 0) {
					return 3;
				}
				commandline.addarg(gstocall);
// rcw2: Current RiscOS port of gs can't find its fonts without this...
#ifdef riscos
				commandline.addarg("-I<GhostScript$Dir>");
#endif
#if 0
				// now handled by the next block (pioptions)
				char *gsargs = getRegistryValue(errstream, "common", "GS_LIB");
				if (gsargs) {
					char *inclDirective = new char[strlen(gsargs) + 3];
					strcpy(inclDirective, "-I");
					strcat(inclDirective, gsargs);
					commandline.addarg(inclDirective);
					delete[]inclDirective;
					delete[]gsargs;
				}
#endif
				const char *pioptions = defaultPIoptions(errstream, options.verbose);
				if (pioptions && (strlen(pioptions) > 0)) {
					commandline.addarg(pioptions);
				}
				if (!options.verbose)
					commandline.addarg("-q");
				if (strcmp(options.drivername.value.value(), "ps2ai") != 0) {	// not for ps2ai
					if (options.nobindversion) {
						// NOBIND disables bind in, e.g, gs_init.ps
						// these files are loaded before pstoedit.pro
						// so any already bound call to, e.g., show could
						// not be intercepted by pstoedit's show

						commandline.addarg("-dNOBIND");
					} else {
						commandline.addarg("-dDELAYBIND");
					}
					commandline.addarg("-dWRITESYSTEMDICT");
				}

				if (options.verbose) {
					commandline.addarg("-dESTACKPRINT");
				}
				if (options.withdisplay) {
					commandline.addarg("-dNOPAUSE");
				} else {
					commandline.addarg("-dNODISPLAY");
				}
				commandline.addarg("-dNOEPS"); // otherwise EPSF files create implicit showpages and a save/restore pair which disturbs the setPageSize handling
				for (unsigned int psi = 0; psi < options.psArgs().argc; psi++) {
					commandline.addarg(options.psArgs().argv[psi]);
				}
#if (defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined (NetBSD) ) && !defined(DJGPP)
				RSString gsinfilename = RSString("\"");
				gsinfilename += gsin ;
				gsinfilename += RSString("\"");
#else
				RSString gsinfilename = gsin;
#endif
				if (strcmp(options.drivername.value.value(), "ps2ai") == 0) {
					// ps2ai needs special headers
					commandline.addarg(gsinfilename.value());	// the first time to set the paramters for ps2ai.ps
					commandline.addarg("ps2ai.ps");
					commandline.addarg(gsinfilename.value());	// again, but this time it'll run the conversion
				} else {
					commandline.addarg(gsinfilename.value());
				}
				if (options.verbose)
					errstream << "now calling the interpreter via: " << commandline << endl;
				// gsresult = system(commandline);
				gsresult = call_PI(commandline.argc, commandline.argv);
				if (gsresult != 0) {
					errstream << "PostScript/PDF Interpreter finished. Return status " << gsresult 
					<< " executed command : " << commandline << endl;
				}
				// ghostscript seems to return always 0, so
				// check whether the normal end was reached by pstoedit.pro
				if (!options.keepinternalfiles)
					(void) remove(gsin.value());
				//wogl free(gsin);
				// if really returned !0 don't grep
				if (!gsresult) {
					if (options.verbose)
						errstream << "Now checking the temporary output" << endl;
					gsresult = grep(successstring, gsout.value(), errstream);
				}
			}
			if (gsresult != 0) {
				errstream << "The interpreter seems to have failed, cannot proceed !" << endl;
				if (!options.keepinternalfiles)
					(void) remove(gsout.value());
				//wogl free(gsoutName);
				return 1;
			} else {
				if (outputdriver->withbackend()) {

					if (options.backendonly && (strcmp(options.nameOfInputFile, stdinFileName) == 0)) {
						// we need to copy stdin because we need to scan it twice
						// the first time for the boundingboxes and than a second time for the
						// real processing.
						gsout = full_qualified_tempnam("pssi");
						if (options.verbose) {
							errstream << "copying stdin to " << gsout << endl;
						}
						ofstream copyofstdin(gsout.value());
						copy_file(cin, copyofstdin);
					}

					{
						// local scope to force delete before delete of driver
						outputdriver->setdefaultFontName(options.replacementfont.value.value());
						//      if (nosubpathes) ((DriverDescription*) outputdriver->Pdriverdesc)->backendSupportsSubPathes=false;
//						outputdriver->simulateSubPaths = (bool) options.simulateSubPaths;

						const char * bbfilename = 0;
						if (options.useBBfrominput) {
							// read BB from original input file
							bbfilename = options.nameOfInputFile;
						} else {
							bbfilename = gsout.value();
						}
                            
						yyin = fopen(bbfilename, "rb");	// ios::binary | ios::nocreate
						if (!yyin) {
								errstream << "Error opening file " << bbfilename << endl;
								return 1;
						}
						
						PSFrontEnd fe(outFile,
									  errstream,
									  options,
									  options.nameOfInputFile,
									  options.nameOfOutputFile,
									  currentDriverDesc, driveroptions, options.splitpages, outputdriver);
						if (options.verbose)
							errstream << "now reading BoundingBoxes from file " << bbfilename << endl;
						/* outputdriver-> */ drvbase::totalNumberOfPages =
						fe.readBBoxes( /* outputdriver-> */ drvbase::bboxes());
						fclose(yyin);
						if (options.verbose) {
							errstream << " got " <<	drvbase::totalNumberOfPages << " page(s)" << endl;
							for (unsigned int i = 0;  i < drvbase::totalNumberOfPages; i++) {
								errstream <<  drvbase::bboxes()[i].ll << " " <<  drvbase::bboxes()[i].ur << endl;
							}
						}

						if (options.verbose)
							errstream << "now postprocessing the interpreter output" << endl;
						
						yyin = fopen(gsout.value(), "rb");
						fe.run(options.mergelines);
						// now we can close it in any case - since we took a copy
						fclose(yyin);
					}
					if (options.verbose)
						errstream << "postprocessing the interpreter output finished" << endl;
// now delete is done in fe.run                 delete outputdriver;


					if (options.backendonly && (strcmp(options.nameOfInputFile, stdinFileName) == 0)) {
						(void) remove(gsout.value());
					}
				} else {
					// outputdriver has no backend
					// Debug or PostScript driver
					ifstream gsoutStream(gsout.value()); 
					if (options.verbose)
						errstream << "now copying  '" << gsout << "' to '"
							<< (options.nameOfOutputFile ? options.nameOfOutputFile : "standard output ") << "' ";
					copy_file(gsoutStream, *outputFilePtr);
					if (options.verbose)
						errstream << " done \n";
					delete outputdriver;
				}
				if (!options.backendonly) {
					if (!options.keepinternalfiles)
						(void) remove(gsout.value());
				}
			}
			//wogl free(gsoutName); 
		}
	}							// no backend specified
//  delete [] drivername;
//  delete [] nameOfInputFile;
//  delete [] nameOfOutputFile;
	return 0;
}

extern int callgs(int argc, const char *const argv[]);  //defined in callgs.cpp
extern const char *whichPI(ostream & errstream, int verbose, const char * gsToUse, const char *gsregbase);

static bool versioncheckOK = false;

extern "C" DLLEXPORT int pstoedit_checkversion(unsigned int callersversion)
{
	versioncheckOK = (callersversion == pstoeditdllversion) ||
		((callersversion == 300) && (pstoeditdllversion == 301));
	return versioncheckOK;
}

void ignoreVersionCheck()
{
	versioncheckOK = true;
}

extern "C" DLLEXPORT
	int pstoeditwithghostscript(int argc,
								const char *const argv[],
								ostream & errstream, 
								const DescriptionRegister * const pushinsPtr)
{
	if (!versioncheckOK) {
		errorMessage("wrong version of pstoedit");
		return -1;
	}
	return pstoedit(argc, argv, errstream, callgs, whichPI, pushinsPtr);
}

//
// the following functions provide the interface for gsview
//
static const char *givenPI = 0;
static const char *returngivenPI(ostream & errstream, int verbose, const char *gsregbase, const char * GSToUse)
{
	unused(&errstream);
	unused(&verbose);
	unused(&gsregbase);
	unused(&GSToUse);
	return givenPI;
}

extern "C" DLLEXPORT
	int pstoedit_plainC(int argc, const char *const argv[], const char *const psinterpreter)
{
	// this is called e.g. by gsview.
	if (!versioncheckOK) {
		errorMessage("wrong version of pstoedit");
		return -1;
	}
	if (psinterpreter != 0) { // gsview uses 0 here
		givenPI = psinterpreter;
		return pstoedit(argc, argv, cerr, callgs, returngivenPI, 0);
	} else {
		return pstoedit(argc, argv, cerr, callgs, whichPI, 0);
	}
}

static DriverDescription_S * getPstoeditDriverInfo_internal(bool withgsdrivers)
{
	if (!versioncheckOK) {
		errorMessage("wrong version of pstoedit");
		return 0;
	}
	loadpstoeditplugins("pstoedit", cerr, false );

	const int dCount = getglobalRp()->nrOfDescriptions();
	/* use malloc to be compatible with C */
	DriverDescription_S *result =
		(DriverDescription_S *) malloc((dCount + 1) * sizeof(DriverDescription_S));
	DriverDescription_S *curR = result;
	assert(curR);
	const DriverDescription *const *dd = getglobalRp()->rp;
	while (dd && (*dd)) {
		const DriverDescription *currentDD = *dd;
		assert(currentDD);
		if (currentDD->nativedriver || withgsdrivers) {
			curR->symbolicname =  currentDD->symbolicname;
			curR->explanation =  currentDD->short_explanation;
			curR->suffix =  currentDD->suffix;
			curR->additionalInfo =  currentDD->additionalInfo;
			curR->backendSupportsSubPathes = (int) currentDD->backendSupportsSubPathes;
			curR->backendSupportsCurveto = (int) currentDD->backendSupportsCurveto;
			curR->backendSupportsMerging = (int) currentDD->backendSupportsMerging;
			curR->backendSupportsText = (int) currentDD->backendSupportsText;
			curR->backendSupportsImages = (int) currentDD->backendDesiredImageFormat != DriverDescription::noimage;
			curR->backendSupportsMultiplePages = (int) currentDD->backendSupportsMultiplePages;
			curR++;
		}
		dd++;
	}
	curR++->symbolicname = 0;	// indicator for end

	return result;
}
extern "C" DLLEXPORT DriverDescription_S * getPstoeditDriverInfo_plainC(void)
{
	return getPstoeditDriverInfo_internal(true);
}

extern "C" DLLEXPORT DriverDescription_S* getPstoeditNativeDriverInfo_plainC(void)
 /* for the pstoedit native drivers - not the ones that are provided as short cuts to ghostscript */
{
	return getPstoeditDriverInfo_internal(false);
}


//
// function to clear the memory allocated by the getPstoeditDriverInfo_plainC
// function. This avoids conflicts with different heaps.
//
extern "C" DLLEXPORT void clearPstoeditDriverInfo_plainC(DriverDescription_S * ptr)
{
	free(ptr);
}

#if defined(_WIN32) || defined(__OS2__)

//
// under Windows we need to be able to switch between two modes of calling GS - 1 via DLL and 2 with EXE
// the EXE is needed when being called from gsview - whereas in all other cases the DLL is the better way
// So the useDLL is set to false - and the pstoedit stand-alone program sets it to true
// gsview uses the default which is false - hence the ghostscript is called via its exe
//

static int useDLL = false;
// need to use int instead of bool because of C mode
extern "C" DLLEXPORT void setPstoeditsetDLLUsage(int useDLL_p) 
{ useDLL = useDLL_p; }
extern "C" DLLEXPORT int getPstoeditsetDLLUsage() 
{ return useDLL; }

extern void set_gs_write_callback(write_callback_type * new_cb);	// defined in callgsdllviaiapi.cpp

extern "C" DLLEXPORT void setPstoeditOutputFunction(void *cbData, write_callback_type * cbFunction)
{
	if (!versioncheckOK) {
		errorMessage("wrong version of pstoedit");
		return;
	}

	static callbackBuffer cbBuffer(0, 0);// default /dev/null

	set_gs_write_callback(cbFunction);	// for the gswin.DLL
	(void) cbBuffer.set_callback(cbData, cbFunction);
#if defined(HAVESTL) || defined(__OS2__)
	(void)cerr.rdbuf(&cbBuffer);
#else
	cerr = &cbBuffer;
#endif
}

// END WINDOWS ONLY SECTION
#endif
