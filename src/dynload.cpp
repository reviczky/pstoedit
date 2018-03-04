/*
   dynload.h : This file is part of pstoedit
   declarations for dynamic loading of drivers

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
#include "cppcomp.h"
#include I_iostream

// we need __linux__ instead of just linux since the latter is not defined when -ansi is used.

#if defined(__sparc) || defined(__linux) || defined(__linux__) || defined(__CYGWIN32__) || defined(__FreeBSD__) || defined(_WIN32) || defined(__OS2__) || defined(_AIX) || (defined (HAVE_DLFCN_H) && (HAVE_DLFCN_H==1 ) )
#define HAVESHAREDLIBS
#endif

#ifdef HAVESHAREDLIBS

#ifndef LEANDYNLOAD
#include "drvbase.h"
#endif

#include "dynload.h"

#include I_stdio
#include I_stdlib
#include I_string_h

// for DriverDescription types.

#if defined(__hpux) && defined (__GNUG__)
typedef void (*initfunctype) ();
#endif

#if defined(__linux) || defined(__linux__) || defined(__CYGWIN32__) || defined(__FreeBSD__) || defined(__hpux) || defined(_AIX) || (defined (HAVE_DLFCN_H) && (HAVE_DLFCN_H==1 ) )
#include <dlfcn.h>
#elif defined(__sparc)
#if defined(__SVR4)
#include <dlfcn.h>
#else
				// prototypes under SunOS 4.1.x are not compatible with C++
extern "C" {
	void *dlopen(const char *pathname, int mode);
	 DynLoader::fptr dlsym(void *handle, const char *name);
	char *dlerror(void);
	int dlclose(void *handle);
}
#define RTLD_LAZY       1
#endif
#elif defined(__OS2__)
#include <sys/types.h>
#include <dlfcn.h>

#elif defined(_WIN32)
static char dlerror()
{
	return ' ';
}

#include <windows.h>
#define WINLOADLIB LoadLibrary
#define WINFREELIB FreeLibrary
	//  static const char * const libsuffix = ".dll";
#else
#error "system unsupported so far"
#endif

DynLoader::DynLoader(const char *libname_P, int verbose_p):libname(libname_P), handle(0),
verbose(verbose_p)
{
	if (libname) {
		if (verbose)
			cerr << "dlopening " << libname << endl;
		open(libname);
	}
}

void DynLoader::open(const char *libname_P)
{
	if (handle) {
		cerr << "error: DynLoader has already opened a library" << endl;
		exit(1);
	}
	const unsigned int size = strlen(libname_P) + 1; 
	char *fulllibname = new char[size];
	strcpy_s(fulllibname, size, libname_P);
	// not needed strcat(fulllibname_P,libsuffix);

#if defined(__linux) || defined(__linux__) || defined(__CYGWIN32__) || defined(__FreeBSD__) || defined(__hpux) || defined(__sparc) || defined(__OS2__) || defined(_AIX) || (defined (HAVE_DLFCN_H) && (HAVE_DLFCN_H==1 ) )
	int loadmode = RTLD_LAZY;	// RTLD_NOW
	handle = dlopen(fulllibname, loadmode);
#elif defined(_WIN32)
	handle = WINLOADLIB(fulllibname);
#else
#error "system unsupported so far"
#endif
	if (handle == 0) {
		cerr << "Problem during opening " << fulllibname << ":" << dlerror()
			<< endl;
		delete[]fulllibname;
		return;
	}
	{
		if (verbose)
			cerr << "loading dynamic library " << fulllibname << " completed successfully" << endl;
#if defined(__hpux) && defined (__GNUG__)
		// in HPUX g++ this is called "_GLOBAL__DI" - on other systems it may be something like "_init"
		// under HP-UX automatic initialization of dlopened shared libs doesn't work - at least not when compiled with g++
		initfunctype fptr = (initfunctype) dlsym(handle, "_GLOBAL__DI");
		if (fptr) {
			if (verbose)
				cerr << "Found an init function for the library, so execute it " << endl;
			fptr();
		}
#endif
	}
	delete[]fulllibname;
}

void DynLoader::close()
{
	if (handle) {
#if defined(__linux) || defined(__linux__) || defined(__CYGWIN32__) || defined(__FreeBSD__) || defined(__hpux) || defined(__sparc) || defined(__OS2__) || defined(_AIX) || (defined (HAVE_DLFCN_H) && (HAVE_DLFCN_H==1 ) )
		dlclose(handle);
#elif defined(_WIN32)
		(void) WINFREELIB((HINSTANCE) handle);
#else
#error "system unsupported so far"
#endif
		handle = 0;
	}
}

DynLoader::~DynLoader()
{
	close();
	libname=0;
}

int DynLoader::knownSymbol(const char *name) const
{
	return (getSymbol(name, 0) != 0);
}

DynLoader::fptr DynLoader::getSymbol(const char *name, int check) const
{
	//
	// see http://www.trilithium.com/johan/2004/12/problem-with-dlsym/ for a nice discussion
	// about the cast problem
	//
#if defined(__linux) || defined(__linux__) || defined(__CYGWIN32__) || defined(__FreeBSD__) || defined(__hpux) || defined(__sparc) || defined(__OS2__) || defined(_AIX) || (defined (HAVE_DLFCN_H) && (HAVE_DLFCN_H==1 ) )
	DynLoader::fptr rfptr = (DynLoader::fptr) dlsym(handle, name);	//lint !e611 //: Suspicious cast
#elif defined(_WIN32)
	DynLoader::fptr rfptr = (DynLoader::fptr) GetProcAddress((HINSTANCE) handle, name);	//lint !e611 //: Suspicious cast
#else
#error "system unsupported so far"
#endif
	if ((rfptr == 0) && check) {
		cerr << "error during getSymbol for " << name << ":" << dlerror()
			<< endl;
	}
	return rfptr;
}


#ifndef LEANDYNLOAD
// LEANDYNLOAD means just the DynLoader class

// now the stuff handling search for plugins and loading

//lint -sem(PluginVector::add, custodial(1))

// OK, I could take a vector<DynLoader *> but that is not supported with all compilers yet.
class PluginVector {
public:
	enum { maxPlugins = 100 };
	PluginVector():curindex(0u) {
		for (unsigned int i = 0; i < maxPlugins; i++)
			pluginPtr[i] = 0;
	} ~PluginVector() {
		for (unsigned int i = 0; i < maxPlugins; i++) {
			delete pluginPtr[i];
			pluginPtr[i] = 0;
		}
	}
	void add(DynLoader * newelem) {
		pluginPtr[curindex] = newelem;
		curindex++;
	}
	DynLoader *pluginPtr[maxPlugins];
	unsigned int curindex;

	NOCOPYANDASSIGN(PluginVector)
};

static PluginVector LoadedPlugins;

static void loadaPlugin(const char *filename, ostream & errstream, bool verbose)
{
	if (verbose && filename)
		errstream << "loading plugin: " << filename << endl;

	DriverDescription::currentfilename = filename;
	DynLoader *dynloader = new DynLoader(filename, verbose);
	if (!dynloader->valid()) {
		delete dynloader;
		errstream << "Problem during opening of pstoedit driver plugin: " << filename <<
			". This is no problem as long the driver in this library is not needed. Possibly you need to install further libraries and/or extend the LD_LIBRARY_PATH (*nix) or PATH (Windows) environment variables."
			<< endl;
		return;
	} else {
#if defined (__APPLE__)
// on MacOS it seems necessary to at least do a dlsym to an existing symbol 
// to get the global ctors beeing called
		void (*fp) () = ((void (*)()) dynloader->getSymbol("initlibrary"));
		if (fp) {
			if (verbose) {
				errstream << "getSymbol(\"initlibrary\") succeeded." << endl;
			}
		} else {
			errstream << "getSymbol(\"initlibrary\") failed." << endl;
		}
#endif
	}
	LoadedPlugins.add(dynloader);

// this seems to be obsolete. but not with .dll's
//          globalRp->explainformats(errstream);
	if (dynloader->knownSymbol("getglobalRp")) {
		getglobalRpFuncPtr dyngetglobalRpFunc =
			(getglobalRpFuncPtr) dynloader->getSymbol("getglobalRp");
		if (dyngetglobalRpFunc == 0) {
			errstream << "could not find getglobalRp " << endl;
			return;
		}
		DescriptionRegister *dynglobalRp = dyngetglobalRpFunc();
		if (dynglobalRp == 0) {
			errstream << " didn't find any registered Drivers " << endl;
		} else if (dynglobalRp != getglobalRp()) {
//                      globalRp->explainformats(errstream);
			getglobalRp()->mergeRegister(errstream, *dynglobalRp, filename);
		}
	} else {
		// didn't fint getglobalRp
		// so assume that the plug-in does not have
		// it's own copy of globalRp but uses the
		// one from the caller
	}
//          globalRp->explainformats(errstream);
}

// for directory search
#if (defined (HAVE_DLFCN_H) && (HAVE_DLFCN_H==1 ) )
#include <dirent.h>
#define DIR_VERSION 1
#elif (defined (HAVE_SYS_NDIR_H) && (HAVE_SYS_NDIR_H==1 ) )
#include <sys/ndir.h>
#define DIR_VERSION 2
#elif (defined (HAVE_SYS_DIR_H) && (HAVE_SYS_DIR_H==1 ) )
#include <sys/dir.h>
#define DIR_VERSION 3
#elif (defined (HAVE_NDIR_H) && (HAVE_NDIR_H==1 ) )
#include <ndir.h>
#define DIR_VERSION 4
#else
//
// last chance
#if defined(__linux) || defined(__linux__) || defined(__CYGWIN32__) || defined(__FreeBSD__) || defined(__hpux) || defined(__sparc) || defined(__OS2__) || defined(_AIX)
#include <direct.h>
#define DIR_VERSION 5
#else
// if not even direct.h is available we leave DIR_VERSION unset and later skip to Windows
#endif

#endif

#ifdef DIR_VERSION
// this is for all *nix like systems
void loadPlugInDrivers(const char *pluginDir, ostream & errstream, bool verbose)
{
	if (pluginDir) {
		DIR *dirp;
		struct dirent *direntp;
#if defined(__OS2__) || defined(__CYGWIN32__)
		const char *const suffix = ".dll";
#elif defined (__APPLE__)
		const char *const suffix = ".so";	// ".dylib";
		// Note: the suffix depends on the way the plugin libraries are built via libtool.
		// If the -module flag is set, then the output is a .so library,
		// if no -module is given, then the output is a .dylib library.
		// We use the .so since we use the -module option, since that is needed for other
		// systems such as cygwin.
#elif defined (__hpux)
		const char *const suffix = ".sl";
#else
		const char *const suffix = ".so";
#endif
		if (verbose && pluginDir)
			errstream << "loading plugins from " << pluginDir << " using suffix: " << suffix <<
				endl;

		dirp = opendir(pluginDir);
		if (!dirp) {
			if (verbose && pluginDir)
				errstream << "opendir failed on: " << pluginDir << endl;
			return;
		}

		while ((direntp = readdir(dirp)) != NIL) {
//      cout <<  direntp->d_name << endl;
			unsigned int flen = strlen(direntp->d_name);
			char *expectedpositionofsuffix = direntp->d_name + flen - strlen(suffix);
//              if ( local filename starts with drv or plugins and ends with .so)
#if defined (__CYGWIN32__)
			const char *const libprefix = "cygp2edrv";
#else
			const char *const libprefix = "libp2edrv";
#endif
			if (((strstr(direntp->d_name, libprefix) == direntp->d_name) ||
				 (strstr(direntp->d_name, "plugin") == direntp->d_name)
				) && (strstr(expectedpositionofsuffix, suffix) == expectedpositionofsuffix)) {
				char *fullname = new char[strlen(pluginDir) + flen + 2];
				strcpy(fullname, pluginDir);
				strcat(fullname, "/");
				strcat(fullname, direntp->d_name);
//          cout <<  direntp->d_name  << " " << fullname << endl;
				loadaPlugin(fullname, errstream, verbose);
				delete[]fullname;
			}

		}						// while
		closedir(dirp);
	} else {
		errstream << "Could not load plugins - parameter pluginDir is null " << endl;
		return;
	}
}
#elif defined(_WIN32)


void loadPlugInDrivers(const char *pluginDir, ostream & errstream, bool verbose)
{
	if (pluginDir != 0) {
		char szExePath[1000];
		szExePath[0] = '\0';
		(void) P_GetPathToMyself("pstoedit", szExePath, sizeof(szExePath));

		WIN32_FIND_DATA finddata;

		const char pattern[] = "/*.dll";
		const unsigned int size = strlen(pluginDir) + strlen(pattern) + 1; 
		char *searchpattern = new char[size];
		strcpy_s(searchpattern, size, pluginDir);
		strcat_s(searchpattern, size, pattern);
		HANDLE findHandle = FindFirstFile(searchpattern, &finddata);
		if (findHandle == INVALID_HANDLE_VALUE) {
			if (verbose)
				errstream << "Could not open plug-in directory (" << pluginDir
					<< " or didn't find any plugin there" << endl;
		} else {
			BOOL more = true;
			while (more) {
#ifdef xx
#define stricmp _stricmp
#endif
				// check for suffix beeing really .dll because FindFirstFile also matches
				// files such as e.g. .dllx
				const unsigned int len = strlen(finddata.cFileName);
				// -4 means go back the length of ".dll"
				if (stricmp(&finddata.cFileName[len - 4], ".dll") == 0) {
					// cout << &finddata.cFileName[len -4 ] << endl;
					const unsigned int size_2 = strlen(pluginDir) + len + 3; 
					char *fullname = new char[size_2];
					strcpy_s(fullname, size_2, pluginDir);
					strcat_s(fullname, size_2, "\\");
					strcat_s(fullname, size_2, finddata.cFileName);
//              errstream << "szExePath " << szExePath << endl;

					if ((stricmp(fullname, szExePath) != 0)
						&& (stricmp(finddata.cFileName, "pstoedit.dll") != 0)) {
						// avoid loading dll itself again
						//                 errstream << "loading " << fullname << endl;
						loadaPlugin(fullname, errstream, verbose);
					} else {
						//                 errstream << "ignoring myself " << finddata.cFileName << endl;
					}
					delete[]fullname;
				}
				more = FindNextFile(findHandle, &finddata);
			}
			(void) FindClose(findHandle);
		}
		delete[]searchpattern;
	} else {
		errstream << "Could not load plugins - parameter pluginDir is null " << endl;
		return;
	}
}
#else
#error "system unsupported so far (directory reading)"
#endif

#endif
// LEAN


#else
// no shared libs
#ifdef HAVESTL
#include <iosfwd>
using namespace std;
#else
class ostream;
#endif
void loadPlugInDrivers(const char *pluginDir, ostream & errstream, bool verbose)
{
	if (verbose) {
		errstream <<
			"dummy version of loadPlugInDrivers called since you system doesn't seem to support loading libraries at runtime - If you think that this is not correct, contact the author of this program (wglunz35_AT_pstoedit.net) "
			<< endl;
	}
}								// just a dummy Version
#endif
