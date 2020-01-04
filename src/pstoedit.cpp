/*
   pstoedit.cpp : This file is part of pstoedit
   main control procedure

   Copyright (C) 1993 - 2020 Wolfgang Glunz, wglunz35_AT_pstoedit.net

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
#ifndef OS_WIN32_WCE
#include <sys/stat.h>
#endif

#include <memory>
#include <stddef.h>

#include "pstoeditoptions.h"
#include "pstoedit_config.h"

PsToEditOptions& PsToEditOptions::theOptions() // singleton
{
	static PsToEditOptions theInstance;
	return theInstance;
}


#include "pstoedit.h"

// for the DLL export data types (the description struct)
#include "pstoedll.h"

#include "drvbase.h"

#ifndef UPPVERSION
#include "dynload.h"
#endif

#ifndef USEPROLOGFROMFILE
#include "pstoedit.ph"
#endif

#include "psfront.h"

#ifdef HAVE_DIALOG
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
	const size_t len = strlen(filename);
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
	#ifdef HAVE_STL
	// at least MSVC does not know nocreate when using STL
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
		// old form auto buffer = new char[bufferlen];
		auto buffer = std::vector<char>(bufferlen);
		while ((void) inFile.get(buffer.data(), bufferlen, '\n'),
			   // debug            errstream << " read in grep :`" << buffer << "'" << inFile.gcount() << " " << matchlen << " eof:"<< inFile.eof() << endl,
			   !inFile.eof() /* && !inFile.fail() */ ) {
// debug                errstream << " read in grep :`" << buffer << "'" << inFile.gcount() << " " << matchlen << endl;
			// gcount returns int on MSVC
			// make a temp variable to avoid a compiler warning on MSVC
			// (signed/unsigned comparison)
			const std::streamsize inFile_gcountresult = inFile.gcount();
#ifdef HAVE_STL
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

			if ((inFile_gcountresult>0) && ((size_t)inFile_gcountresult == matchlen) && strequal(buffer.data(), matchstring) ) {
			//old	delete[]buffer;
				return 0;
			}
			if (inFile.peek() == '\n')
				(void) inFile.ignore();
		}
		//old delete[]buffer;
	}
	return 1;					// fail
}

// *INDENT-OFF*
class drvNOBACKEND : public drvbase {	// not really needed - just as template argument
public:
	derivedConstructor(drvNOBACKEND);	// Constructor
	~drvNOBACKEND() override {
		options=nullptr;
	} // Destructor

	class DriverOptions : public ProgramOptions {
	public:
		DriverOptions()
		{
		}
	}*options;

	bool withbackend() const override {
		return false;
	}
private:						// yes these are private, library users should use the public interface
	// provided via drvbase
	void open_page() override {
	}
	void close_page() override {
	}
	void show_text(const TextInfo & /* textInfo */ ) override {
	}
	void show_path() override {
	}
	void show_rectangle(const float /* llx */ , const float /* lly */ , const float /* urx */ ,
						const float /* ury */ ) override {
	}
};
// *INDENT-ON*

drvNOBACKEND::derivedConstructor(drvNOBACKEND):constructBase
{
}

static DriverDescriptionT < drvNOBACKEND > D_psf("psf", "Flattened PostScript (no curves)",
												 "","fps", true, false, true, true,  DriverDescription::memoryeps,
												 DriverDescription::normalopen, true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_ps("ps", "Simplified PostScript with curves", "","spsc", true, true, true, true,
												DriverDescription::memoryeps, DriverDescription::normalopen, true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_debug("debug", "for test purposes", "","dbg", true, true,
												   true, true, DriverDescription::memoryeps, DriverDescription::normalopen,
												   true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_dump("dump", "for test purposes (same as debug)",
												  "","dbg", true, true, true, true, DriverDescription::memoryeps,
												  DriverDescription::normalopen, true, true,false);
static DriverDescriptionT < drvNOBACKEND > D_gs("gs",
												"any device that Ghostscript provides - use gs:format, e.g. gs:pdfwrite",
												"","gs", true, true, true, true, DriverDescription::noimage,
												DriverDescription::normalopen, true, true,false);
#if 0
// -88 no longer supported - too lazy to create an own class for the moment.
static const OptionDescription driveroptionsAI[] = {
	OptionDescription("-88",0,"generate AI88 format"),
	endofoptions};
#endif
static DriverDescriptionT < drvNOBACKEND > D_ps2ai("ps2ai",
												   "Adobe Illustrator via ps2ai.ps of Ghostscript",
												   "","ai", true, true, true, true, DriverDescription::noimage,
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

#ifndef UPPVERSION
static void loadpstoeditplugins(const char *progname, ostream & errstream, bool verbose)
{
	static bool pluginsloaded = false;
	if (pluginsloaded)
		return;
	RSString plugindir = getRegistryValue(errstream, "common", "plugindir");
	if (plugindir.length()) {
		loadPlugInDrivers(plugindir.c_str(), errstream, verbose);	// load the driver plugins
		pluginsloaded = true;
	}
#ifdef PSTOEDITLIBDIR
	struct stat s;
	if (!pluginsloaded &&
	    !stat(PSTOEDITLIBDIR, &s) &&
	    S_ISDIR(s.st_mode)) {
  	  // also try to load drivers from the PSTOEDITLIBDIR
	  loadPlugInDrivers(PSTOEDITLIBDIR, errstream,verbose);
	  pluginsloaded = true;
	}
#endif
	// If the above failed, also look in the directory where the pstoedit .exe/dll was found
	if (!pluginsloaded) {
	  char szExePath[1000];
	  szExePath[0] = '\0';
	  const unsigned long r = P_GetPathToMyself(progname, szExePath, sizeof(szExePath));
	  if (verbose)  errstream << "pstoedit : path to myself:" << progname << " " << r << " " << szExePath<< endl;
	  char *p = nullptr;
	  if (r && (p = strrchr(szExePath, directoryDelimiter)) != nullptr) {
		*p = '\0';
		loadPlugInDrivers(szExePath, errstream,verbose);
	  }
	  // now try also $exepath/../lib/pstoedit
          if (szExePath[0]) {
            // it is not an empty string
#if COMPILEDFOR64BIT 
	    strcat_s(szExePath,1000,"/../lib64/pstoedit");
#else
	    strcat_s(szExePath,1000,"/../lib/pstoedit");
#endif
	    if (!strequal(szExePath, plugindir.c_str())) {
	      loadPlugInDrivers(szExePath, errstream,verbose);
	    }
	  }
	}

	// delete[]plugindir;
}
#endif

extern FILE *yyin;				// used by lexer
						// This has to be declared here because of the extern "C"
						// otherwise we could declare it locally where it is used


static void usage(ostream & outstream, bool forTeX, bool withdetails, bool withcategories = false)
{
	if (withcategories) {
		// PropSheetEnum {g_t, t_t, d_t, a_t, b_t, h_t };
		for (unsigned int sheet = PsToEditOptions::g_t ; sheet < PsToEditOptions::h_t ; sheet++ ) {
	        // if (sheet == PsToEditOptions::a_t) continue; // skip "about"
			outstream << "\\subsection{"  << PsToEditOptions::propSheetName((PsToEditOptions::PropSheetEnum) sheet) << "}" << endl;
			PsToEditOptions::theOptions().showhelp(outstream,forTeX,withdetails,sheet);
		}
		outstream << "\\subsection{Input and outfile file arguments}" << endl;
		outstream << "[ inputfile [outputfile] ]" << endl;
	} else {
		PsToEditOptions::theOptions().showhelp(outstream,forTeX,withdetails);
		outstream << "[ inputfile [outputfile] ]" << endl;
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
void useCoutForDiag(int flag) { f_useCoutForDiag = (flag != 0); }


extern "C" DLLEXPORT
	int pstoedit(int argc, const char *const argv[], ostream & errstream,
				 execute_interpreter_function call_PI,
				 whichPI_type whichPI, const DescriptionRegister * const pushinsPtr)
{

	Closer closerObject;

	PsToEditOptions& options = PsToEditOptions::theOptions(); // just short hand notation
	ostream& diag = f_useCoutForDiag ? cout : errstream;

	drvbase::SetVerbose( false );	// init
	const unsigned int remaining = options.parseoptions(errstream,argc,argv);


#ifdef __VERSION__
	#if defined(_LP64) && (_LP64)
		#define COMPILEDFORWHICHARCH "64-bit"
	        #define COMPILEDFOR64BIT 1
	#else
		#define COMPILEDFORWHICHARCH "32-bit"
	        #define COMPILEDFOR64BIT 0
	#endif
        #ifdef __GNUG__
                const char compversion [] = "g++ " __VERSION__ " - " COMPILEDFORWHICHARCH;
        #else
                const char compversion [] = "unknown compiler " __VERSION__ " - " COMPILEDFORWHICHARCH;
        #endif

#else
	#ifdef _MSC_VER
		#define XNUMTOSTRING(x) NUMTOSTRING(x)
		#define NUMTOSTRING(x) #x
		#ifdef _WIN64
			#define COMPILEDFORWHICHARCH "64-bit"
	        	#define COMPILEDFOR64BIT 1
		#else
			#define COMPILEDFORWHICHARCH "32-bit"
	        	#define COMPILEDFOR64BIT 0
		#endif
		const char compversion [] = "MS VC++ " XNUMTOSTRING(_MSC_VER) " - " COMPILEDFORWHICHARCH;
	#elif defined(__SUNPRO_CC)
		#define XNUMTOSTRING(x) NUMTOSTRING(x)
		#define NUMTOSTRING(x) #x
		const char compversion [] = "SUN Studio " XNUMTOSTRING(__SUNPRO_CC);
	        #define COMPILEDFOR64BIT 0
	#else
		const char compversion [] = "unknown compiler ";
	        #define COMPILEDFOR64BIT 0
	#endif
#endif

	if (!options.quiet) {

#ifdef _DEBUG
	const char buildtype [] = "debug build";
#else
	const char buildtype [] = "release build";
#endif
		errstream << "pstoedit: version " << PACKAGE_VERSION << " / DLL interface " <<
		drvbaseVersion << " (built: " << __DATE__ << " - " << buildtype << " - " << compversion << ")"
		" : Copyright (C) 1993 - 2020 Wolfgang Glunz\n";
	}

	//  handling of derived parameters
	drvbase::SetVerbose(options.verbose() > 1); // verbose just for level > 1
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
		errstream << "more than two file arguments " << endl;
		shortusage(diag);
		return 1;
			}
	}

#ifdef HAVE_DIALOG
	// setPstoeditDialogFunction(rundialog);

	if (options.showdialog) {
		static bool initdone = false;
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
		options.nameOfInputFile = nullptr;
	}
	if (options.nameOfOutputFile && strequal(options.nameOfOutputFile, "-")) {
		delete[]options.nameOfOutputFile;
		options.nameOfOutputFile = nullptr;
	}

#ifndef UPPVERSION
// not needed for importps
	if (!options.dontloadplugins) {
		loadpstoeditplugins(argv[0], errstream, options.verbose());	// load the driver plugins
	}
#endif

	if ((pushinsPtr != nullptr) && (pushinsPtr != getglobalRp())) {
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
	if (options.listdrivers) {
		// show driver specific options
		getglobalRp()->listdrivers(diag);
		return 1;
	}
	if (options.showdrvhelp) {
		usage(diag,false,false);
		const char *gstocall = whichPI(diag, options.verbose(), options.gsregbase.value.c_str(),options.GSToUse.value.c_str());
		if (gstocall != nullptr) {
			errstream << "Default interpreter is " << gstocall << endl;
		}
		getglobalRp()->explainformats(diag);
		return 1;
	}

	if (options.justgstest) {
		const char *gstocall = whichPI(errstream, options.verbose(), options.gsregbase.value.c_str(),options.GSToUse.value.c_str());
		if (gstocall == nullptr) {
			return 3;
		}
		Argv commandline;
		commandline.addarg(gstocall);
// rcw2: Current RiscOS port of gs can't find its fonts without this...
#ifdef riscos
		commandline.addarg("-I<GhostScript$Dir>");
#endif
		const char *pioptions = defaultPIoptions(errstream, options.verbose());
		if (pioptions && (strlen(pioptions) > 0)) {
			commandline.addarg(pioptions);
		}

		if (!options.verbose())
			commandline.addarg("-q");

		if (options.nobindversion) {
			// NOBIND disables bind in, e.g, gs_init.ps
			// these files are loaded before pstoedit.pro
			// so any already bound call to, e.g., show could
			// not be intercepted by pstoedit's show

			commandline.addarg("-dNOBIND");
#if WITHRDB
		} else if (options.reallydelaybindversion) {
			commandline.addarg("-dREALLYDELAYBIND");
#endif
		} else {
			commandline.addarg("-dDELAYBIND");
		}
		commandline.addarg("-dWRITESYSTEMDICT");


		if (options.verbose()) {
			commandline.addarg("-dESTACKPRINT");
			// commandline.addarg("-sDEBUG=true");
		}
		if (options.withdisplay) {
			commandline.addarg("-dNOPAUSE");
		} else {
			commandline.addarg("-dNODISPLAY");
		}
		commandline.addarg("-dDELAYSAFER");
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
		if (!options.noquit) {
			commandline.addarg("-c");
			commandline.addarg("quit");
                }
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
	if (options.drivername.value.length() == 0) {
		// try to find driver according to suffix of input file
		if (!options.nameOfOutputFile) {
			errstream << "No output format specified (-f option) and format could not be deduced from suffix of output file since no output file name was given" << endl;
			shortusage(diag);
			return 1;
		} else {
			const char * suffixOfInputFile = strrchr(options.nameOfOutputFile,'.');
			if (!suffixOfInputFile) {
				errstream << "No output format specified (-f option) and format could not be deduced from suffix of output file since no suffix was found" << endl;
				shortusage(diag);
				return 1;
			} else {
				const DriverDescription *suffixDriverDesc = getglobalRp()->getDriverDescForSuffix((suffixOfInputFile+1)); // +1 == skip "."
				if (suffixDriverDesc) {
					errstream << "No explicit output format specified - using " << suffixDriverDesc->symbolicname << " as derived from suffix of output file" << endl;
					options.drivername = suffixDriverDesc->symbolicname;
				} else {
					errstream << "No output format specified (-f option) and format could not be uniquely deduced from suffix " << suffixOfInputFile << " of output file" << endl;
					// usage(errstream);
					getglobalRp()->explainformats(diag); // ,true);
					return 1;
				}
			}
		}
	}
	// from here on options.drivername is != 0
	assert(options.drivername.value.length());

	{
		RSString driveroptionscopy (options.drivername.value);
		char *driveroptions = strchr(const_cast<char*>(driveroptionscopy.c_str()), ':'); //lint !e1773
		if (driveroptions) {
			*driveroptions = '\0';	// replace : with 0 to separate drivername
			options.drivername = driveroptionscopy;
			driveroptions++;
		}
		const DriverDescription *currentDriverDesc = getglobalRp()->getDriverDescForName(options.drivername.value.c_str());
		if (currentDriverDesc == nullptr) {
			errstream << "Unsupported output format " << options.drivername.value.c_str() << endl;
			getglobalRp()->explainformats(diag);
			return 1;
		}

		if ( currentDriverDesc->backendFileOpenType!=DriverDescription::normalopen && !options.nameOfOutputFile ) {
			errstream << "The driver for the selected format cannot write to standard output because it writes binary data" << endl;
			return 1;
		}


		if (driveroptions && strequal(driveroptions, "-help") ) {
			ProgramOptions* dummy = currentDriverDesc->createDriverOptions();
			if (dummy->numberOfOptions() ) {
				errstream << "The driver for this output format supports the following additional options: (specify using -f \"format:-option1 -option2\")" << endl;
				dummy->showhelp(diag,false,false);
			}
			delete dummy;
			return 1;
		}
		if (strequal(options.drivername.value.c_str(), "gs")) {
// TODO:
			// Check for input file (exists, or stdin) stdout handling
			if (!options.nameOfInputFile) {
				errstream << "Cannot read from standard input if GS drivers are selected" << endl;
				return 1;
			}
			// an input file was given as argument

			// just test whether InputFile is readable.
			// The file will be read directly from the PostScript
			// interpreter later on
			convertBackSlashes(options.nameOfInputFile);

			if (!fileExists(options.nameOfInputFile)) {
				errstream << "Could not open file " << options.nameOfInputFile << " for input" << endl;
				return 1;
			}

			if (!options.nameOfOutputFile) {
				errstream <<
					"Cannot write to standard output if GS drivers are selected" << endl;
				return 1;
			}

			if (options.backendonly) {
				errstream << "The -bo option cannot be used if GS drivers are selected " << endl;
				return 1;
			}

			if (!driveroptions) {
				errstream <<
					"The gs output driver needs a gs-device as argument, e.g. gs:pdfwrite" << endl;
				return 1;
			}
			// special handling of direct Ghostscript drivers
			Argv commandline;
//          char commandline[1000];
			// TODO check for overflow
//          commandline[0]= '\0';
			const char *gstocall = whichPI(errstream, options.verbose(), options.gsregbase.value.c_str(),options.GSToUse.value.c_str());
			if (gstocall == nullptr) {
				return 3;
			}
			commandline.addarg(gstocall);
// rcw2: Current RiscOS port of gs can't find its fonts without this...
#ifdef riscos
			commandline.addarg("-I<GhostScript$Dir>");
#endif
			if (!options.verbose())
				commandline.addarg("-q");

			const char *pioptions = defaultPIoptions(errstream, options.verbose());
			if (pioptions && (strlen(pioptions) > 0)) {
				commandline.addarg(pioptions);
			}
			commandline.addarg("-dNOPAUSE");
			commandline.addarg("-dBATCH");
			RSString tempbuffer("-sDEVICE=") ; // char tempbuffer[1000];
			// tempbuffer[0] = '\0';
			// tempbuffer += "-sDEVICE="; // strcat_s(tempbuffer, 1000, "-sDEVICE=");
			tempbuffer += driveroptions; // strcat_s(tempbuffer, 1000,driveroptions);	// e.g., pdfwrite ;
			commandline.addarg(tempbuffer.c_str() );
			for (unsigned int psi = 0; psi < options.psArgs().argc; psi++) {
				commandline.addarg(options.psArgs().argv[psi]);
			}
			tempbuffer = "-sOutputFile=";
			tempbuffer +=  options.nameOfOutputFile;
			// strcat_s(tempbuffer,1000, "-sOutputFile=");
			// strcat_s(tempbuffer,1000, options.nameOfOutputFile);
			commandline.addarg(tempbuffer.c_str());
			commandline.addarg("-c");
			commandline.addarg("save");
			commandline.addarg("pop");
			commandline.addarg("-f");
			commandline.addarg(options.nameOfInputFile);
			if (options.verbose())
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
			ostream *outputFilePtr = nullptr;
			ofstream outFile;
			drvbase *outputdriver = nullptr;
			char *nameOfOutputFilewithoutpercentD = nullptr;

			{
				// setup pstoeditHome directory
				char szExePath[1000];
				szExePath[0] = '\0';
				const int r = P_GetPathToMyself(argv[0], szExePath, sizeof(szExePath));
				if (r && options.verbose()) {
					errstream << "path to myself: " << szExePath << endl;
				}
				char *p = nullptr;
				if (r && (p = strrchr(szExePath, directoryDelimiter)) != nullptr) {
					*p = '\0';
					drvbase::pstoeditHomeDir() = RSString(szExePath);
				} else {
					drvbase::pstoeditHomeDir() = "";
				}
				if (options.verbose())  errstream << "pstoedit home directory : " << drvbase::pstoeditHomeDir().c_str() << endl;

				drvbase::pstoeditDataDir() = drvbase::pstoeditHomeDir();

#if (defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined (NetBSD) ) && !defined(DJGPP)
#ifdef PSTOEDITDATADIR
		// usually something like /usr/share/pstoedit
				drvbase::pstoeditDataDir() = PSTOEDITDATADIR;
#else
				drvbase::pstoeditDataDir() +=  "/../share/pstoedit";
#endif
#endif

				if (options.verbose())  errstream << "pstoedit data directory : " << drvbase::pstoeditDataDir().c_str() << endl;
			}


			if (options.nameOfOutputFile) {
				options.splitpages = options.splitpages || (!currentDriverDesc->backendSupportsMultiplePages);
				// if -split is given or if backend does not support multiple pages
				// BUT, do this only if a real output file is given, not when writing to stdout.
				outputFilePtr = &outFile;
				convertBackSlashes(options.nameOfOutputFile);
#if 0
				nameOfOutputFilewithoutpercentD = cppstrdup(options.nameOfOutputFile, 20);	// reserve 20 chars for page number
				sprintf_s(TARGETWITHLEN(nameOfOutputFilewithoutpercentD, strlen(options.nameOfOutputFile)+ 20) , options.nameOfOutputFile, 1);	//first page is page 1
#endif
				RSString newFileName = getOutputFileNameFromPageNumber(options.nameOfOutputFile, options.pagenumberformat, 1);
				nameOfOutputFilewithoutpercentD = cppstrdup(newFileName.c_str());

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
														 nullptr,
														 options);
				}
			}

			if ((!outputdriver) || (!outputdriver->driverOK())) {
				errstream << "Creation of driver failed " << endl;
				return 1;
			}


			if (options.explicitFontMapFile.value.length()) {
				if (fileExists(options.explicitFontMapFile.value.c_str())) {
					if (options.verbose()) {
						errstream << "Loading fontmap from " << options.explicitFontMapFile.value << endl;
					}
					drvbase::theFontMapper().readMappingTable(errstream, options.explicitFontMapFile.value.c_str());
				} else {
					RSString extendedFontMapFile = options.explicitFontMapFile.value;
					extendedFontMapFile += ".fmp";
					if (fileExists(extendedFontMapFile.c_str())) {
						if (options.verbose()) {
							errstream << "Loading fontmap from " << extendedFontMapFile << endl;
						}
						drvbase::theFontMapper().readMappingTable(errstream, options.explicitFontMapFile.value.c_str());
					} else {
						errstream << "Warning: Couldn't open fontmap file. Neither " <<
						options.explicitFontMapFile.value << " nor " << extendedFontMapFile << ". Option ignored." << endl;
					}
				}
			} else {
				// look for a driver specific fontmap
				// also look in the directory where the pstoedit .exe/dll was found

				if (drvbase::pstoeditDataDir() != "") {
					RSString test(drvbase::pstoeditDataDir());
					test += directoryDelimiter;
					test += options.drivername.value;
					test += ".fmp";
					if (fileExists(test.c_str())) {
						if (options.verbose()) {
							errstream <<
								"loading driver specific fontmap from " << test.c_str() << endl;
						}
						drvbase::theFontMapper().readMappingTable(errstream, test.c_str());
					}
				}
			}

			{
				// load a system specific font map after the driver fontmap file
				// (idea provided by Scott Pakin)
				// So driver fontmap files can use generic names and redirect them to specific names via the system
				// map file.
				if (drvbase::pstoeditDataDir() != "") {
					RSString test(drvbase::pstoeditDataDir());
					test += directoryDelimiter;
#if defined(PSTOEDIT_UNIXLIKE)
					test += "unix";
#else
					test += "windows";
#endif
					test += ".fmp";
					if (fileExists(test.c_str())) {
						if (options.verbose()) {
							errstream <<
								"loading system specific fontmap from " << test.c_str() << endl;
						}
						drvbase::theFontMapper().readMappingTable(errstream, test.c_str());
					} else  {
						if (options.verbose()) {
							errstream <<
								"Warning: System specific fontmap not loaded - " << test.c_str() << " not found in pstoedit data directory." << endl;
						}
					}
				} else {
					if (options.verbose()) {
							errstream <<
								"Warning: System specific fontmap not loaded - no pstoedit data directory found" << endl;
					}
				}
			}

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
				ofstream inFileStream(gsin.c_str());
				inFileStream << "/pstoedit.pagetoextract " << options.pagetoextract << " def" << endl;
				inFileStream << "/pstoedit.versioninfo (" << PACKAGE_VERSION << " " << compversion << ") def" << endl;
				if (options.nomaptoisolatin1) {
					inFileStream << "/pstoedit.maptoisolatin1 false def" << endl;
				}
				if (options.psLanguageLevel != 3 ) {
					inFileStream << "/.setlanguagelevel where { pop " << options.psLanguageLevel << " .setlanguagelevel } if" << endl;
				}
				if (options.usePlainStrings) {
					inFileStream << "/pstoedit.ashexstring false def" << endl;
				}
				if (options.t2fontsast1) {
					inFileStream << "/pstoedit.t2fontsast1 true def" << endl;
				} else {
					inFileStream << "/pstoedit.t2fontsast1 false def" << endl;
				}
				if (options.fontdebug) {
					inFileStream << "/pstoedit.fontdebugging true def" << endl;
				} else {
					inFileStream << "/pstoedit.fontdebugging false def" << endl;
				}

				if (options.useRGBcolors) {
					inFileStream << "/pstoedit.useRGBcolors true def" << endl;
				}
#if WITHCLIPSUPPORT
				if (currentDriverDesc->backendSupportsClipping && !(options.noclip) ) {
					inFileStream << "/pstoedit.dumpclippath true def" << endl;
				}
#endif
				if (options.verbose() > 1) {
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
				if (options.DrawGlyphBitmaps) {
					inFileStream << "/pstoedit.DrawGlyphBitmaps true def" << endl;
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
				if (options.unmappablecharstring.value.length()) {
					inFileStream << "/pstoedit.globalunmappablecharacter (" <<
						options.unmappablecharstring.value.c_str()[0] << ") def" << endl;
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

				if (strequal(options.drivername.value.c_str(), "ps")
					|| strequal(options.drivername.value.c_str(), "psf")) {
					inFileStream << "/pstoedit.escapetext true def" << endl;
				}
				if ((strequal(options.drivername.value.c_str(), "debug")) || (strequal(options.drivername.value.c_str(), "pdf"))) {
					inFileStream << "/pstoedit.usepdfmark true def" << endl;
				}

				inFileStream << "/pstoedit.replacementfont (" << options.replacementfont.value << ") def" << endl;
				gsout = full_qualified_tempnam("psout");
				// assert((gsin != gsout) && ("You seem to have a buggy version of tempnam" != 0));
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
				writeFileName(inFileStream, gsout.c_str());
				inFileStream << ") def" << endl;

				inFileStream << "/pstoedit.inputfilename  (";
				writeFileName(inFileStream, options.nameOfInputFile);
				inFileStream << ") def" << endl;

				if (options.nameOfIncludeFile.value.length()) {
					ifstream filetest(options.nameOfIncludeFile.value.c_str());
					if (!filetest) {
						errstream << "Could not open file " <<
							options.nameOfIncludeFile.value << " for inclusion" << endl;
						return 1;
					}
					filetest.close();
					inFileStream << "/pstoedit.nameOfIncludeFile  (";
					writeFileName(inFileStream, options.nameOfIncludeFile.value.c_str());
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
				if (options.useBBfrominput) {
					inFileStream << "/pstoedit.useBBfrominput true def" << endl;
				}
				if (options.fake_date_and_version) {
					// for regression testing
					drvbase::use_fake_version_and_date = true;
				}
				if (options.nobindversion) {
					inFileStream << "/pstoedit.delaybindversion  false def" << endl;
				} else {
					// we use this for both versions of delaybind (also reallydelaybind)
					inFileStream << "/pstoedit.delaybindversion  true def" << endl;
				}
				delete[]nameOfOutputFilewithoutpercentD;
				if (outputdriver && (outputdriver->knownFontNames() != nullptr)) {
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
				const char* successstring = nullptr;	// string that indicated success of .pro
				if (strequal(options.drivername.value.c_str(), "ps2ai")) {
					successstring = "%EOF";	// This is written by the ps2ai.ps
					// showpage in ps2ai does quit !!!
					// ps2ai needs special headers
					inFileStream << "/pstoedit.preps2ai where not { \n"
						// first run through this file (before ps2ai.ps)
						"	/jout true def \n" "	/joutput pstoedit.outputfilename def \n";
					if (driveroptions && strequal(driveroptions, "-88")) {
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
				// now call Ghostscript

				Argv commandline;
				const char *gstocall = whichPI(errstream, options.verbose(),
				  options.gsregbase.value.c_str(),options.GSToUse.value.c_str());
				if (gstocall == nullptr) {
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
				const char *pioptions = defaultPIoptions(errstream, options.verbose());
				if (pioptions && (strlen(pioptions) > 0)) {
					commandline.addarg(pioptions);
				}
				if (!options.verbose())
					commandline.addarg("-q");
				if (!strequal(options.drivername.value.c_str(), "ps2ai") ) {	// not for ps2ai
					if (options.nobindversion) {
						// NOBIND disables bind in, e.g, gs_init.ps
						// these files are loaded before pstoedit.pro
						// so any already bound call to, e.g., show could
						// not be intercepted by pstoedit's show
						commandline.addarg("-dNOBIND");
#if WITHRDB
					}
					else if (options.reallydelaybindversion) {
						commandline.addarg("-dREALLYDELAYBIND");
#endif
					}
					else {
						commandline.addarg("-dDELAYBIND");
					}
					commandline.addarg("-dWRITESYSTEMDICT");
				}

				if (options.verbose()) {
					commandline.addarg("-dESTACKPRINT");
				}

				if (options.withdisplay) {
					commandline.addarg("-dNOPAUSE");
				} else {
					if (options.pngimage.value.length()) {
						commandline.addarg("-dNOPAUSE");
						commandline.addarg("-dBATCH");
						commandline.addarg("-sDEVICE=png16m");
						RSString tempbuffer = "-sOutputFile=";
						tempbuffer +=  options.pngimage.value;
						commandline.addarg(tempbuffer.c_str());
					} else {
						commandline.addarg("-dNODISPLAY");
					}
				}
				commandline.addarg("-dDELAYSAFER");
				commandline.addarg("-dNOEPS"); // otherwise EPSF files create implicit showpages and a save/restore pair which disturbs the setPageSize handling

				//layer commandline.addarg("-dDEBUG");
				//layer commandline.addarg("pdf_cslayer.ps");


				for (unsigned int psi = 0; psi < options.psArgs().argc; psi++) {
					commandline.addarg(options.psArgs().argv[psi]);
				}
#if (defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined (NetBSD) ) && !defined(DJGPP)
				RSString gsinfilename = RSString("\"");
				gsinfilename += gsin ;
				gsinfilename += RSString("\"");
#else
				const RSString gsinfilename = gsin;
#endif
				if (strequal(options.drivername.value.c_str(), "ps2ai")) {
					// ps2ai needs special headers
					commandline.addarg(gsinfilename.c_str());	// the first time to set the paramters for ps2ai.ps
					commandline.addarg("ps2ai.ps");
					commandline.addarg(gsinfilename.c_str());	// again, but this time it'll run the conversion
				} else {
					commandline.addarg(gsinfilename.c_str());
				}
				if (options.verbose())
					errstream << "now calling the interpreter via: " << commandline << endl;
				// gsresult = system(commandline);
				gsresult = call_PI(commandline.argc, commandline.argv);
				if (gsresult != 0) {
					errstream << "PostScript/PDF Interpreter finished. Return status " << gsresult
					<< " executed command : " << commandline << endl;
				}
				// Ghostscript seems to return always 0, so
				// check whether the normal end was reached by pstoedit.pro
				if (!options.keepinternalfiles) {
                                        // coverity[tainted_string]
					(void) remove(gsin.c_str());
                                }
				// if really returned !0 don't grep
				if (!gsresult) {
					if (options.verbose())
						errstream << "Now checking the temporary output" << endl;
					gsresult = grep(successstring, gsout.c_str(), errstream);
				}
			}
			if (gsresult != 0) {
				errstream << "The interpreter seems to have failed, cannot proceed !" << endl;
				if (!options.keepinternalfiles)
					(void) remove(gsout.c_str());
				return 1;
			} else {
				if (outputdriver->withbackend()) {

					if (options.backendonly && (strequal(options.nameOfInputFile, stdinFileName) )) {
						// we need to copy stdin because we need to scan it twice
						// the first time for the boundingboxes and than a second time for the
						// real processing.
						gsout = full_qualified_tempnam("pssi");
						if (options.verbose()) {
							errstream << "copying stdin to " << gsout << endl;
						}
						ofstream copyofstdin(gsout.c_str());
						copy_file(cin, copyofstdin);
					}

					{
						// local scope to force delete before delete of driver
						outputdriver->setdefaultFontName(options.replacementfont.value.c_str());
						//      if (nosubpaths) ((DriverDescription*) outputdriver->Pdriverdesc)->backendSupportsSubPaths=false;
//						outputdriver->simulateSubPaths = (bool) options.simulateSubPaths;

						const char * bbfilename = nullptr;
						if (options.useBBfrominput) {
							// read BB from original input file
							bbfilename = options.nameOfInputFile;
						} else {
							bbfilename = gsout.c_str();
						}
						const auto err_bb = fopen_s(&yyin, bbfilename, "rb"); 
								// ios::binary | ios::nocreate
						if (err_bb) {
						    errstream << "Error opening file " << bbfilename << endl;
						    return 1;
						}
						assert(yyin);

						PSFrontEnd fe(outFile,
									  errstream,
									  options,
									  options.nameOfInputFile,
									  options.nameOfOutputFile,
									  currentDriverDesc, driveroptions, options.splitpages, outputdriver);
					//	if (options.verbose())
					//		errstream << "now reading BoundingBoxes from file " << bbfilename << endl;
						/* outputdriver-> */ 
						int pagesfound= fe.readBBoxes( /* outputdriver-> */ drvbase::bboxes());
						fclose(yyin);
						if (options.useBBfrominput && !pagesfound) {
							// in case we did not find a BB in input, e.g. when reading binary PDF,
							// then we use the BB from interpreter output.
							// In case of PDF, we dumped the BB there anyway.
							bbfilename = gsout.c_str();
							const auto err_bb2 = fopen_s(&yyin, bbfilename, "rb");
							if (err_bb2) {
								errstream << "Error opening file " << bbfilename << endl;
								return 1;
							}
							assert(yyin);
							pagesfound = fe.readBBoxes( /* outputdriver-> */ drvbase::bboxes());
							fclose(yyin);
						}
						drvbase::totalNumberOfPages() = pagesfound;
						if (options.verbose()) {
							errstream << " got " <<	drvbase::totalNumberOfPages() << " page(s) from " << bbfilename << endl;
							for (size_t i = 0;  i < drvbase::totalNumberOfPages(); i++) {
								errstream <<  drvbase::bboxes()[i].ll << " " <<  drvbase::bboxes()[i].ur << endl;
							}
							errstream << "now postprocessing the interpreter output" << endl;
						}
						const auto err_gs = fopen_s(&yyin, gsout.c_str(), "rb");
						if (err_gs) {
							errstream << "Error opening file " << gsout.c_str() << endl;
							return 1;
						}
						assert(yyin);
						fe.run(options.mergelines);
						// now we can close it in any case - since we took a copy
						fclose(yyin);
					}
					if (options.verbose())
						errstream << "postprocessing the interpreter output finished" << endl;
// now delete is done in fe.run                 delete outputdriver;


					if (options.backendonly && (strequal(options.nameOfInputFile, stdinFileName))) {
						(void) remove(gsout.c_str());
					}
				} else {
					// outputdriver has no backend
					// Debug or PostScript driver
					ifstream gsoutStream(gsout.c_str());
					if (options.verbose())
						errstream << "now copying  '" << gsout << "' to '"
							<< (options.nameOfOutputFile ? options.nameOfOutputFile : "standard output ") << "' ";
					copy_file(gsoutStream, *outputFilePtr);
					if (options.verbose())
						errstream << " done \n";
					delete outputdriver;
				}
				if (!options.backendonly) {
					if (!options.keepinternalfiles)
						(void) remove(gsout.c_str());
				}
			}
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
static const char *givenPI = nullptr;
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
	if (psinterpreter != nullptr) { // gsview uses 0 here
		givenPI = psinterpreter;
		return pstoedit(argc, argv, cerr, callgs, returngivenPI, nullptr);
	} else {
		return pstoedit(argc, argv, cerr, callgs, whichPI, nullptr);
	}
}

static DriverDescription_S * getPstoeditDriverInfo_internal(bool withgsdrivers)
{
	if (!versioncheckOK) {
		errorMessage("wrong version of pstoedit");
		return nullptr;
	}
#ifndef UPPVERSION
// not needed for importps
	loadpstoeditplugins("pstoedit", cerr, false );
#endif

	const size_t dCount = (size_t) getglobalRp()->nrOfDescriptions() + 1;
	/* use malloc to be compatible with C */
	DriverDescription_S *result =
		static_cast<DriverDescription_S *>( malloc(dCount * sizeof(DriverDescription_S)));
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
			curR->additionalInfo =  currentDD->additionalInfo();
			curR->backendSupportsSubPaths = (int) currentDD->backendSupportsSubPaths;
			curR->backendSupportsCurveto = (int) currentDD->backendSupportsCurveto;
			curR->backendSupportsMerging = (int) currentDD->backendSupportsMerging;
			curR->backendSupportsText = (int) currentDD->backendSupportsText;
			curR->backendSupportsImages = (int) currentDD->backendDesiredImageFormat != DriverDescription::noimage;
			curR->backendSupportsMultiplePages = (int) currentDD->backendSupportsMultiplePages;
			curR++;
		}
		dd++;
	}
	curR++->symbolicname = nullptr;	// indicator for end

	return result;
}
extern "C" DLLEXPORT DriverDescription_S * getPstoeditDriverInfo_plainC(void)
{
	return getPstoeditDriverInfo_internal(true);
}

extern "C" DLLEXPORT DriverDescription_S* getPstoeditNativeDriverInfo_plainC(void)
 /* for the pstoedit native drivers - not the ones that are provided as short cuts to Ghostscript */
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
// gsview uses the default which is false - hence the Ghostscript is called via its exe
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

	static callbackBuffer cbBuffer(nullptr, nullptr);// default /dev/null

	set_gs_write_callback(cbFunction);	// for the gswin.DLL
	(void) cbBuffer.set_callback(cbData, cbFunction);
#if defined(HAVE_STL) || defined(__OS2__)
	(void)cerr.rdbuf(&cbBuffer);
#else
	cerr = &cbBuffer;
#endif
}

// END WINDOWS ONLY SECTION
#endif
