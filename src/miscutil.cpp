/*
   miscutil.cpp : This file is part of pstoedit
   misc utility functions

   Copyright (C) 1998 - 2009  Wolfgang Glunz, wglunz35_AT_pstoedit.net

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

#include I_stdio

// #ifdef _MSC_VER
// for getcwd ( at least for Visual C++)

#if defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined(__EMX__) || defined (NetBSD) || defined(__APPLE__) || defined(_AIX)
#include <unistd.h>

#ifdef __hpux
// HP-UX does not define getcwd in unistd.h
extern "C"  char *getcwd(char *, size_t);
#endif

#else
// not Unix like

#if HAVE_DIRENT_H
#include <dirent.h>
#elif HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#elif HAVE_SYS_DIR_H
#include <sys/dir.h>
#elif HAVE_NDIR_H
#include <ndir.h>
#else
// last chance
#include <direct.h>
#endif

#endif

#ifdef HAVE_MKSTEMP
// for using mkstemp we need errno
#include <errno.h>
#endif

#include I_stdlib

#include I_string_h


#if defined(_WIN32)
#include <windows.h>
#elif defined (__OS2__)
#define INCL_DOS
#define INCL_WINWINDOWMGR
#define INCL_WINSHELLDATA		/* Window Shell functions       */
#define INCL_WINERRORS
#include <os2.h>
#endif


#if defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined (NetBSD) 
//take this out (we may have backslashes w/ EMX(OS/2)):  || defined(__EMX__)
void convertBackSlashes(char *) { }
// nothing to do on systems with unix style file names ( / for directories)

#else
void convertBackSlashes(char *fileName)
{
 	char *c ;
	while ((c = strchr(fileName, '\\')) != NIL)
		*c = '/';
}
#endif

#ifndef HAVE_MKSTEMP
//
// if we do not have mkstemp we need to emulate it using old tempnam
// for some systems we even have to provide an own tempnam
//
#if defined(__STRICT_ANSI__) 
// normally this is in stdio.h except if __STRICT_ANSI__ is defined (GNU specific)
// but it is in again - at least under Linux with g++3.0
#if !( defined (__GNUG__)  && (__GNUC__>=3) )
	extern "C" char *tempnam(const char *, const char *pfx);
#endif
#endif

#if defined(riscos) 
// rcw2: tempnam doesn't seem to be defined in UnixLib 3.7b for RiscOS
static char *tempnam(const char *, const char *pfx)
{
	char tmp[1024];

	strcpy(tmp, "<Wimp$ScrapDir>.");
	strcat(tmp, pfx);
	return strdup(tmp);
}
#endif

#ifdef __WATCOMC__
// tempnam doesn't seem to be defined in Watcoms clibs
static char *tempnam(const char *, const char *pfx)
{
	const char *path;
	char tmp[1024];

	// simply take c: as temp dir, if environment not defined
	if ((path = getenv("TEMP")) == 0L && (path = getenv("TMP")) == 0L) {
		strcpy(tmp, "C:\\");
	} else {
		// take temp dir from environment
		strcpy(tmp, path);
		strcat(tmp, "\\");
	}

	strcat(tmp, pfx);

	// remove all backslashes
	convertBackSlashes(tmp);

	return strdup(tmp);
}
#endif

// #define BUGGYTEMPNAM

#if (defined(__mips) && defined(__sgi))
#define strdup cppstrdup
#endif

#if (defined(DJGPP) && defined(BUGGYTEMPNAM)) || (defined(__mips) && defined(__sgi))
// tempnam under DJGPP behaves different than on all other systems
// and Irix doesn't seem to have tempnam
char *tempnam(const char *, const char *pfx)
{
	return strdup(tmpnam(0));
}
#endif

// endif for not having mkstemp

#endif

RSString full_qualified_tempnam(const char *pref)
{
#ifdef HAVE_MKSTEMP
	const char *path = 0;
	char * filename = 0;
	const char XXXXXX[] = "XXXXXX" ; // needed for mkstemp template
	(void) ((path = getenv("TEMP"))   == 0L && 
		(path = getenv("TMP"))    == 0L &&
		(path = getenv("TMPDIR")) == 0L
		);
	const unsigned int needed = strlen(path ? path:"" ) + 1 + strlen(pref) + 1 + strlen(XXXXXX) + 2;
	filename = new char [ needed ];
	filename[0] = '\0';
// all getenvs returned 0
	if (path) {
		strncpy(filename, path, needed);
		strcat(filename, "/");
	}
	strcat(filename, pref);
	strcat(filename, XXXXXX);
	// cout << "using " << filename << " as template for mkstemp" << endl;
	const int fd = mkstemp(filename);
	// cout << "returned " << filename << " and " << fd << endl;
	if (fd == -1) {
		cerr << "error in mkstemp for " << filename << " " << errno << endl;
		exit(1);
	}

#else 
#if defined (__BCPLUSPLUS__) || defined (__TCPLUSPLUS__)
/* borland has a prototype that expects a char * as second arg */
	char *filename = tempnam(0, (char *) pref);
#else
	char *filename = tempnam(0, pref);
#endif
	// W95: Fkt. tempnam() erzeugt Filename+Pfad
	// W3.1: es wird nur der Name zurueckgegeben
#endif

// rcw2: work round weird RiscOS naming conventions
#ifdef riscos
	const RSString result(filename);
	free(filename);
	return result;
#else
	convertBackSlashes(filename);
	if ((strchr(filename, '\\') == 0) && (strchr(filename, '/') == 0)) {	// keine Pfadangaben..
		char cwd[400];
		(void) getcwd(cwd, 400);
		RSString result = cwd;
		result += "/";
		result += filename;
		free(filename);
		return result;
	} else {
		const RSString result(filename);
		free(filename);
		return result;
	}
#endif
}

unsigned short hextoint(const char hexchar) 
{
	char h = hexchar;
	if (h >= 'a' && h <='f') h += 'A' - 'a'; // normalize lowercase to uppercase
	unsigned short r = ( h <= '9' ) ? (h - '0') : (h + 10 - 'A' ) ; //lint !e732
	return r;
}

#if defined(_WIN32)
RSString tryregistry(HKEY hKey, LPCSTR subkeyn, LPCSTR key)
{
	HKEY subkey;
	const long ret = RegOpenKeyEx(hKey,	// HKEY_LOCAL_MACHINE, //HKEY hKey,
								  subkeyn,	// LPCSTR lpSubKey,
								  0L,	// DWORD ulOptions,
								  KEY_READ,	// REGSAM samDesired,
								  &subkey	//PHKEY phkResult
		);
	if (ret != ERROR_SUCCESS) {
		// errstream << "RegOpenKeyEx failed with error code " << ret << endl;
		return RSString( (char*) 0);
	} else {
		const int maxvaluelength = 1000;
		BYTE value[maxvaluelength];
		DWORD bufsize = maxvaluelength;
		DWORD valuetype;
		const long retv = RegQueryValueEx(subkey,	// HKEY_LOCAL_MACHINE, //HKEY hKey,
										  key,	// "SOFTWARE\\wglunz\\pstoedit\\plugindir", //LPCSTR lpValueName,
										  NIL,	// LPDWORD lpReserved,
										  &valuetype,	// LPDWORD lpType,
										  value,	// LPBYTE lpData,
										  &bufsize	// LPDWORD lpcbData
			);
		(void) RegCloseKey(subkey);
		if (retv != ERROR_SUCCESS) {
//          errstream << "RegQueryValueEx failed with error code " << retv << endl;
			return RSString( (char*) 0);
		} else {
//          errstream << "result is " << dirname << endl;
			return RSString( (char*) value);
		}
	}
}

#endif

#ifdef __OS2__
#include "getini.c"
#endif

//lint -save -esym(715,errstream) -esym(1764,errstream)
RSString getRegistryValue(ostream & errstream, const char *typekey, const char *key)
{
	
#if defined(_WIN32)
	unused(&errstream);

//  CString subkey = CString("SOFTWARE\\wglunz\\") + CString(product);
	RSString subkeyn("SOFTWARE\\wglunz\\"); // char subkeyn[1000];
	// subkeyn[0] = '\0';
	// strcat_s(subkeyn,1000, "SOFTWARE\\wglunz\\");
	subkeyn += typekey; // strcat_s(subkeyn,1000, typekey);
	RSString result = tryregistry(HKEY_CURRENT_USER, subkeyn.value(), key);
	if (!result.value() )
		result = tryregistry(HKEY_LOCAL_MACHINE, subkeyn.value(), key);
	return result;
#elif defined (__OS2__)
	//query a "real" OS/2 profile pstoedit.ini
	HINI hini;
	HAB hab;
	char pszFileName[1000];
	const char inifilename[] = "pstoedit.ini";
	APIRET rc;
	char buffer[1000];

	hab = WinInitialize(0);

	getini(0, errstream, pszFileName, inifilename, (int) sizeof(pszFileName));
	hini = PrfOpenProfile(hab, pszFileName);
	rc = PrfQueryProfileString(hini, typekey, key, NIL, (PVOID) buffer, (LONG) sizeof(buffer));
	PrfCloseProfile(hini);
	WinTerminate(hab);

	if (rc) {
		// char *r = cppstrdup(buffer);
		return buffer;
	} else
		return RSString( (char*) 0);
#else
//
// UNIX version
// Just ask the environment
//
#if 0
//getenv version
	char envname[1000];
	envname[0] = '\0';
	strcat(envname, typekey);
	strcat(envname, "_");
	strcat(envname, key);
//  cout << "checking " << envname << endl;
	char *envvalue = getenv(envname);
	if (envvalue != 0) {
		char *r = cppstrdup(envvalue);
//      cout << "found " << r << endl;
		return r;
	} else
		return 0;
#else

	char pathbuffer[255];
	// First look in HOME for .pstoedit.reg. If not found there, then look in PATH anywhere
	unsigned int result = searchinpath(getenv("HOME"), ".pstoedit.reg", pathbuffer,
									   sizeof(pathbuffer));
	if (!result)
		result = searchinpath(getenv("PATH"), ".pstoedit.reg", pathbuffer, sizeof(pathbuffer));
	if (!result)
		return RSString((char*) 0);

#if 0
	const char *homedir = getenv("HOME");
	if (!homedir)
		return 0;
	const int len = strlen(homedir) + 20;
	char *obuf = new char[len];
	strcpy(obuf, homedir);
	strcat(obuf, "/.pstoedit.reg");
	ifstream regfile(obuf);
//  cout << "opened " << obuf << endl;
	delete[]obuf;
#endif

	ifstream regfile(pathbuffer);
	if (!regfile)
		return RSString((char*)0);

	char envname[1000];
	envname[0] = '\0';
	strcat(envname, typekey);
	strcat(envname, "/");
	strcat(envname, key);
//  cout << "checking " << envname << endl;
	char line[1000];
	while (!regfile.eof()) {
		regfile.getline(line, 1000);
//      cout << line << endl;
		if (strstr(line, envname)) {
			char *r = cppstrdup(line + strlen(envname) + 1);
			char *cr = strrchr(r, '\r');
			if (cr)
				*cr = 0;
//          cout << "found " << r << endl;
			const RSString result(r);
			delete [] r;
			return result;
		}
	}
	return RSString((char*) 0);
#endif
#endif
}
//lint -restore

void copy_file(istream & infile, ostream & outfile)
{
#if 1
	if ( infile.peek() != EOF ) {
		// Calling the copy operator in case the input file is empty results in the error state
		// of the target stream to be set to fail which then blocks any further IO on the output
		// This is not want we want, hence we have to check in advance whether there is something
		// available in the input buffer.
		// Alternatively I tried if (infile.rdbuf())->in_avail() ) 
		// but - in_avail seems to be compiler dependent. E.g. GNU seems to return
		// the number of available chars even without prior peek. MSVC seems to return 0
		// up to the first peek (or other operation).
		// 
#if defined(__SUNPRO_CC) && (__SUNPRO_CC < 0x500)
		// for SUN CC 4.2 rdbuf is non const - i guess this is an error since all other compilers have it const
		outfile << ((istream&)infile).rdbuf();
#else
		outfile << infile.rdbuf();
#endif
	}
#else
// long version. should do the same as above
	unsigned char c;
	while (infile.get(c)) {
		outfile.put(c);
	}
#endif
}

ostream & operator << (ostream & out, const Argv & a)
{
	for (unsigned int i = 0; i < (unsigned int) a.argc; i++) {
		out << a.argv[i];
		if (i < (unsigned int) ( a.argc -1 )) out << ' ';// out << endl;
	}
	return out;
}


TempFile::TempFile()
{
	tempFileName = tempnam(0, "pstmp");
	// cout << "constructed " << tempFileName << endl; 
}

TempFile::~TempFile()
{
	// cout << "detructed " << tempFileName << endl; 
	close();
	(void) remove(tempFileName);
	free(tempFileName);
}

ofstream & TempFile::asOutput()
{
	close();
	outFileStream.open(tempFileName);
	if (outFileStream.fail())
		cerr << "openening " << tempFileName << "failed " << endl;
	return outFileStream; //lint !e1536 //exposing low access member
}

ifstream & TempFile::asInput()
{
	close();
	inFileStream.open(tempFileName);
	if (outFileStream.fail()) {
		cerr << "openening " << tempFileName << "failed " << endl;
	}
	return inFileStream; //lint !e1536 //exposing low access member
}

void TempFile::close()
{
//#ifdef HAVE_IS_OPEN
//#define IS_OPEN(file) file.is_open()
//#else
//#define IS_OPEN(file) file
//#endif
// commented, since it also works portably if we call clear()

//  if (IS_OPEN(inFileStream))  {
	inFileStream.close();
//      if (inFileStream.fail()) cerr << "closing inFileStream failed " << endl;
//  }
	inFileStream.clear();
//  if (IS_OPEN(outFileStream))  {
	outFileStream.close();
//      if (outFileStream.fail()) cerr << "closing outFileStream  failed " << endl;
//  }
	outFileStream.clear();
}


#if 0
void freeconst(const void *ptr)
{
	free((char *) ptr);			// just to have the warning about casting away constness
	// once (here)
}
#endif

unsigned long searchinpath(const char *EnvPath, const char *name,
						   char *returnbuffer, unsigned long buflen)
{
//      const char * EnvPath = getenv("PATH");
	if (!EnvPath)
		return 0;
#if defined(unix) || defined(__unix__) || defined(_unix) || defined(__unix) || defined(__EMX__) || defined (NetBSD) 
	const char separator = ':';
#else
	const char separator = ';';
#endif
	char *path = cppstrdup(EnvPath, 2);
	// append a separator at the end to make the loop below easier
	unsigned int pathlen = strlen(path);
	path[pathlen] = separator;
	path[pathlen + 1] = '\0';	// remember, we reserved one char more
	char *colon = path;
	char *lastbegin = path;

//      cout << "looking for " << name << " in " << EnvPath << " sep is " << separator << endl;
	while (*colon) {
		while (*colon && (*colon != separator))
			colon++;
//          cout << "lb " << lastbegin << " *colon" << *colon << endl;
		if (*colon) {
			*colon = '\0';
			RSString test(lastbegin);
			test += "/";
			test += name;
			//cout << "checking " << test.value() << endl;
			if (fileExists(test.value())) {
				strcpy_s(returnbuffer,buflen, test.value());
				delete[]path;
				//cout << " FOUND !! " << endl;
				return strlen(returnbuffer);
			}
			colon++;
			lastbegin = colon;
		}
	}
	delete[]path;
	unused(&buflen);
	return 0;
}

unsigned long P_GetPathToMyself(const char *name, char *returnbuffer, unsigned long buflen)
{
#if defined(_WIN32)
	return GetModuleFileName(GetModuleHandle(name), returnbuffer, buflen);
#elif defined (__OS2__)
	PTIB pptib;
	PPIB pppib;
	APIRET rc;
	if ((rc = DosGetInfoBlocks(&pptib, &pppib)) == 0)
		if ((rc = DosQueryModuleName(pppib->pib_hmte, buflen, returnbuffer)) == 0)
			return strlen(returnbuffer);
		else
			return 0;
	else
		return 0;
#else
	if ( (*name == '/')  || (*name == '.') ) {			// starts with / or .
		strcpy(returnbuffer, name);
		return strlen(returnbuffer);
	} else {
		return searchinpath(getenv("PATH"), name, returnbuffer, buflen);

	}
	return 0;					// not found
#endif
}

void errorMessage(const char *errortext)
{
#if 0 // defined(_WIN32)
	MessageBox(NIL, errortext, "pstoedit", MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
#else
	cerr << errortext << endl;
#endif
}

// a very very simple resizing string
RSString::RSString(const char *arg) : content(0), allocatedLength(0), stringlength(0)
{
	if (arg) {
		this->copy(arg,strlen(arg));
	}
	// cerr << "{ constructed" << (void*) this << endl;
}

RSString::RSString(const char arg) : content(0), allocatedLength(0), stringlength(0)
{
	char tmp[2];
	tmp[0] = arg; tmp[1] = '\0';
	this->copy(tmp,1);
}

RSString::RSString(const char * arg  , const size_t len) :content(0), allocatedLength(0), stringlength(len)
{
	if (arg) {
		this->copy(arg,stringlength);
	}
}


RSString::RSString(const RSString & s):content(0), allocatedLength(0), stringlength(0)
{
	assert(this != &s);
	this->copy(s.value(),s.stringlength);
}

char *RSString::newContent(size_t size)
{
	return new char[size];
}

void RSString::clearContent()
{
	delete[]content;
	content = 0;
	stringlength = 0;
}

RSString::~RSString()
{
	// cerr << (void*) this << "deleted }" << endl;
	clearContent();				//lint !e1506 // call to virtual function in dtor
	allocatedLength = 0;
}

bool RSString::contains(const RSString & s) const
{
	return strstr(value(),s.value()) != 0;
}

RSString & RSString::operator += (const char* rs)
{
	assert(rs != 0);
	assert(content != 0);
	const unsigned int rslength = strlen(rs);
	unsigned int newlen = stringlength + rslength  + 1;
	char *newstring = newContent(newlen);
//cout << ":" << content << ":" << stringlength << endl;
//cout << ":" << rs.content << ":" << rs.stringlength << endl;
	for (unsigned int i = 0 ; i < stringlength; i++)
	{
		newstring[i] = content[i];
	}
//	strncpy(newstring, content, stringlength);
	for (unsigned int j = 0 ; j < rslength ; j++)
	{
		newstring[stringlength+j] = rs[j];
	}
//	strncat(newstring, rs.content, rs.stringlength);
	newstring[newlen-1] = '\0';
	clearContent();
	content = newstring;
	allocatedLength = newlen;
	stringlength= newlen-1;
	return *this;
}

RSString & RSString::operator += (const RSString & rs)
{
	return (*this) += rs.content;
}


void RSString::copy(const char *src)
{
	copy(src,strlen(src));

}
//      const char * value() const { return content; }
void RSString::copy(const char *src, const size_t len )
{
//          cerr << "copy " << src << " to " << (void *) this << endl;
	if (src == 0) {
		// nothing to copy - just clear
		clearContent();
	} else if (content && ((len + 1) <= allocatedLength)) {
		// we have enough space
		for (unsigned int i = 0 ; i < len+1; i++)
		{
			content[i] = src[i];
		}
		//::strncpy(content, src, len+1);
		
	} else {
		// resize
//              cerr << "content before delete is " << (void *) content << endl;
		clearContent();
		allocatedLength = len + 1;
		content = newContent(allocatedLength);
//              cerr << "content after new is " << (void *) content << endl;
		for (unsigned int i = 0 ; i < len+1; i++)
		{
			content[i] = src[i]; 
		}
		//::strncpy(content, src, len+1);
	}
	stringlength = len;
}

bool fileExists(const char *filename)
{
#ifdef HAVESTL
	std::ifstream test(filename);
	return test.is_open();
#else
#if defined (__GNUG__)  && (__GNUC__>=3)
	ifstream test(filename, ios::in );	//lint !e655
#else
	ifstream test(filename, ios::in | ios::nocreate);	//lint !e655
	// MSVC needs ios::nocreate if used for testing whether file exists
	// also others except the g++ > 3.0 accept this
#endif
	return (test ? true : false);
#endif
}


//FontMapper::~FontMapper() { 
	// cerr << "destroying FontMapper" << endl;

//}

static void skipws(char *&lineptr)
{
	while ( (*lineptr != '\0') && ((*lineptr == ' ') || (*lineptr == '\t')))
		lineptr++;
	return;
}

static char *readword(char *&lineptr)
{
	char *result = 0;
	if (*lineptr == '"') {
		result = strtok(lineptr, "\"");
	} else {
		result = strtok(lineptr, "\t ");
	}
	if (result) {
		lineptr = result + strlen(result) + 1;
	}
	return result;
}

void FontMapper::readMappingTable(ostream & errstream, const char *filename)
{
	if (!fileExists(filename)) {
		errstream << "Could not open font map file " << filename << endl;
		return;
	}
	ifstream inFile(filename);
	const int linesize = 255;
	char line[linesize];
	char save[linesize];
	unsigned int linenr = 0;
	while (!inFile.getline(line, linesize).eof()) {
		linenr++;
		strcpy_s(save,linesize, line);
#ifdef HAVESTL
		// Notes regarding ANSI C++ version (from KB)
		// istream::get( char* pch, int nCount, char delim ) is different in three ways: 
		// When nothing is read, failbit is set.
		// An eos is always stored after characters extracted (this happens regardless of the outcome).
		// A value of -1 for nCount is an error.

		// If the line contains just a \n then the failbit
		// is set in the ANSI version
		if (inFile.gcount() == 0) {
			inFile.clear();
			continue;
		}
#endif
		if (line[0] == '%')
			continue;
		char *lineptr = line;
		//skip initial spaces
		skipws(lineptr);
//		unsigned int linelength = strlen(line);
		if ((*lineptr) == '\0' ) {
			// empty line - ignore
		//	errstream << "empty line in fontmap - ignored " << endl;
			continue;
		}
		char *original = readword(lineptr);
		skipws(lineptr);
		char *replacement = readword(lineptr);
		if (original && replacement) {
			// errstream << "\"" << original << "\" \"" << replacement <<"\""<< endl;
			insert(original, replacement);
		} else {
			errstream << "unexpected line (" << linenr <<
				") found in fontmap: " << save << endl;
		}
	}
}

const char *FontMapper::mapFont(const RSString & fontname)
{
#if 0
	FontMapping *curEntry = firstEntry;
	// cerr << "Trying to remap font" << fontname << endl;
	while (curEntry != 0) {
		// cerr << "comparing with" << curEntry->original << endl;
		if (curEntry->original == RSString(fontname)) {
			// cerr << "mapped to " << curEntry->replacement.value() << endl;
			return curEntry->replacement.value();
		}
		curEntry = curEntry->nextEntry;
	}
	return 0;
#endif
	const RSString *r = getValue(fontname);
	if (r) {
		return r->value();
	} else {
		for (unsigned int i=0; i<fontname.length(); i++) {
			// patch from Scott Pakin
			// I've noticed that Microsoft's PostScript printer driver, pscript5.dll, 
			// produces EPS files with subsetted fonts and names the subsetted fonts 
			// using a random (?) prefix.  For example, in the attached EPS file, the 
			// subset of Officina Serif Bold is not called OfficinaSerif-Bold but rather 
			// HJCBAA+OfficinaSerif-Bold.  When pstoedit tries to map that to a TeX name 
			// (e.g., for the mpost driver) it fails because mpost.fmp knows only 
			// OfficinaSerif-Bold.
			//
			// The attached patch adds a fallback case to FontMapper::mapFont().  
			// If the specified font can't be mapped and it contains a "+" character, 
			// the method tries a second time starting from after the "+".
	        if (fontname[i] == '+') {
		        const RSString altfontname(fontname.value() + i + 1);
				r = getValue(altfontname);
				if (r)
			        return r->value();
				else 
					return 0;
			}
		}
		return 0;
    }
}

unsigned int Argv::parseFromString(const char * const argstring) {
	unsigned int nrOfNewArgs = 0;
	const char * cp = argstring;
	while (cp && *cp) { // for all args
		while (cp && *cp && (*cp == ' ')) cp++; // skip leading space
		RSString result("");
		if (*cp == '"')	{ // handle string arg - read everything until closing "
				cp++; // skip leading "
				while (cp && *cp && (*cp != '"')) {
					result += *cp; 
					cp++;
				}
				if (*cp) cp++; // skip trailing "
		} else {
				while (cp && *cp && (*cp != ' ')) {
					result += *cp; 
					cp++;
				}
		}
		addarg(result);
		nrOfNewArgs++;
	}
	return nrOfNewArgs;
}
