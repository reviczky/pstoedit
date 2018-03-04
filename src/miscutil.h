#ifndef miscutil_h
#define miscutil_h
/*
   miscutil.h : This file is part of pstoedit
   header declaring misc utility functions

   Copyright (C) 1998 - 2014 Wolfgang Glunz, wglunz35_AT_pstoedit.net

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
#ifndef cppcomp_h
#include "cppcomp.h"
#endif

#include I_iostream
#include I_ostream
#include I_istream
#include I_fstream
#include I_string_h

//lint -efile(451,math.h)
#include <math.h>

USESTD

#ifndef assert
#include <assert.h>
#endif

// used to eliminate compiler warnings about unused parameters
inline void unused(const void * const) { }
//lint -esym(522,unused)


#if defined(_WIN32) || defined(__OS2__)
const char directoryDelimiter = '\\';
#else
const char directoryDelimiter = '/';
#endif



// NOTE: The following two dup functions are made inline to solve the problem
// of allocation and deallocation in different .dlls. 
// a strdup which uses new instead of malloc

inline char * cppstrndup(const char * const src, const size_t length, const size_t addon = 0 )
{
	const size_t lp1 = length+1;
	char * const ret = new char[lp1 + addon];
	for (unsigned int i = 0 ; i < lp1; i++)
	{
			ret[i] = src[i];
	}
	return ret;

}
inline char *cppstrdup(const char * const src, const size_t addon = 0)
{
	return cppstrndup(src,strlen(src),addon);
}

// DLLEXPORT char * cppstrdup(const char * src, unsigned int addon = 0);
// DLLEXPORT char * cppstrndup(const char * src, unsigned int length, unsigned int addon = 0);
DLLEXPORT unsigned short hextoint(const char hexchar) ;

// A temporary file, that is automatically removed after usage
class  TempFile  {
public:
	DLLEXPORT TempFile()  ;
	DLLEXPORT ~TempFile() ;
	DLLEXPORT ofstream & asOutput();
	DLLEXPORT ifstream & asInput();
private:
	void close() ;
	char * tempFileName;
	ofstream outFileStream;
	ifstream inFileStream;

	NOCOPYANDASSIGN(TempFile)
};

#ifdef HAVE_AUTOPTR 
#include <memory>
#else
template <class T> 
class auto_ptr {
public:
// something that can be attached to a normal pointer
// but which deletes the object when the Deleter goes
// out of scope. Sort of very simple std::auto_ptr
//
	auto_ptr(T* ptr,bool isArray = false) : m_ptr(ptr), m_isArray(isArray) {}
	~auto_ptr() { if (m_isArray) delete [] m_ptr; else delete m_ptr; m_ptr=0;}
	T* operator->() const { return m_ptr; }
private:
	T* m_ptr; //lint !e1725
	bool m_isArray;

	NOCOPYANDASSIGN(auto_ptr<T>)

//	auto_ptr(const auto_ptr<T> &); // no copy ctor please
//	const auto_ptr<T>& operator =(const auto_ptr<T> &); // no assignment please
};


#endif

#if defined (_MSC_VER) 
#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
// non dll-interface class 'Mapper<class KeyValuePair<class RSString,class RSString> >' used as base for dll-interface class 'FontMapper'
// miscutil.h(259) : see declaration of 'FontMapper'
// Same applies to string
#endif

//#ifdef NOTYETFULLYBASEDONSTL
#ifdef HAVE_STL
#include <string>
typedef std::string RSString;
inline bool string_contains(const RSString & s, const RSString & substr) { return s.find(substr) !=RSString::npos; }

#define DONOTIMPLEMENTRSSTRING

#else
// a very very simple resizing string
// since STL is not yet available on all systems / compilers
class DLLEXPORT RSString  {
public:
		
	RSString(const char * arg = 0);
	RSString(const char arg );
	RSString(const char * arg , const size_t len);
	RSString(const RSString & s);
	virtual ~RSString();
	const char *c_str() const { return content; }
	char * value_to_change() { return content; }
	size_t length() const { return stringlength; }
	void assign(const char *src,const size_t len) ;
	void assign(const char *src);
	bool contains(const RSString & s) const;
	// coverity[assign_returning_const]
	const RSString & operator = (const RSString & rs) {
		if (&rs != this) {
			assign(rs.c_str(),rs.length());
		}
		return *this;
	}
	const RSString & operator = (const char * rs) {
		assign(rs,strlen(rs));
		return *this;
	}

 	RSString& operator+= (const RSString &rs);
	
	RSString& operator+= (const char * rs);
	friend bool operator==(const RSString & ls,const RSString & rs);
	friend bool operator!=(const RSString & ls,const RSString & rs); 

//	bool operator<(const RSString & rs) const
//	{	return strncmp(content,rs.content) < 0; }
	char operator[](const size_t i) const
	{	return content[i]; }
	friend ostream & operator<<(ostream & out,const RSString &outstring)
	{	if (outstring.content) out << outstring.content; return out; }

private:
	//
	// we need to make the memory allocation/deallocation
	// virtual in order to force that these are executed
	// always in the same context (either main EXE or the
	// loaded DLL)
	// Since the plugins are not and MFC extension DLL memory
	// allocation and deallocation cannot be mixed across the EXE
	// and the DLL. It must be done in a symmetric way - if the memory
	// is allocated by the DLL, it must be destroyed there as well.
	//
	virtual void clearContent();
	// tell Flexelint: clearContent does cleanup
	//lint -sem(RSString::clearContent,cleanup)
	virtual char * newContent(size_t size);
	char * content;
	size_t allocatedLength;
	size_t stringlength; // needed for storing binary strings including \0 
};


inline RSString  operator+ (const RSString & ls, const RSString &rs) 
	{ RSString result(ls); result += rs; return result; }

inline bool operator==(const RSString & ls, const RSString & rs) 
	{ 	return ( (rs.stringlength == ls.stringlength ) && (strncmp(ls.content,rs.content,ls.stringlength) == 0) );	}
inline bool operator!=(const RSString & ls, const RSString & rs)  
	{ 	return !(ls==rs); }
inline bool string_contains(const RSString & s, const RSString & substr) { return s.contains(substr);}
#endif


inline bool strequal(const char * const s1, const char * const s2) { return (strcmp(s1,s2) == 0);}

class Argv {
	enum { maxargs=1000 };
public:
	unsigned int argc;
// #define USE_RSSTRING 1
#ifdef USE_RSSTRING
	RSString argv[maxargs];
#else
	char * argv[maxargs];
#endif

	Argv() : argc(0) { for (unsigned int i = 0; i< (unsigned) maxargs; i++)  { argv[i] = 0; } }
	~Argv() { clear(); }

	void addarg(const char * const arg) { 
		assert(argc<maxargs); //lint !e1776
		if (argc < maxargs) {
#ifdef USE_RSSTRING
		   argv[argc] = RSString(arg);
#else
		   argv[argc] = cppstrdup(arg); 
#endif
		   argc++; 
		}
	}
	void addarg(const RSString & arg) { 
		assert(argc<maxargs); //lint !e1776
		if (argc < maxargs) {
#ifdef USE_RSSTRING
		   argv[argc] = arg;
#else
		   argv[argc] = cppstrdup(arg.c_str()); 
#endif
		   argc++; 
		}
	}

	unsigned int parseFromString(const char * const argstring);
	
	void clear() {
#ifdef USE_RSSTRING
#else
		for (unsigned int i = 0; i< (unsigned) argc &&  i< (unsigned) maxargs ; i++) {

			delete [] argv[i] ; argv[i]= 0; 
		}
#endif
		argc = 0;
	}
	
	NOCOPYANDASSIGN(Argv)
};
DLLEXPORT ostream & operator <<(ostream & out, const Argv & a);


DLLEXPORT bool fileExists (const char * filename);
DLLEXPORT RSString full_qualified_tempnam(const char * pref);
DLLEXPORT void convertBackSlashes(char* string);


#ifndef BUGGYGPP
template <class T> 
#else /* BUGGYGPP */
template <class T,class K_Type,class V_Type> 
#endif /* BUGGYGPP */
class DLLEXPORT Mapper {
public:
	Mapper() : firstEntry(0) {};
	virtual ~Mapper() {
		while (firstEntry != 0) {
			T * nextEntry = firstEntry->nextEntry;
			delete firstEntry;
			firstEntry=nextEntry;
		}
	}
public:
#ifndef BUGGYGPP
// define BUGGYGPP if your compiler complains about syntax error here.
// see above
	void insert(const typename T::K_Type & key, const  typename T::V_Type& value) {
#else /* BUGGYGPP */
	void insert(const K_Type & key, const  V_Type & value) {
#endif /* BUGGYGPP */
		firstEntry = new T(key,value,firstEntry);
	}
#ifndef BUGGYGPP
	const  typename T::V_Type* getValue(const  typename T::K_Type & key) const {
#else /* BUGGYGPP */
	const  V_Type* getValue(const  K_Type & key) const {
#endif /* BUGGYGPP */
		T * curEntry = firstEntry;
		while (curEntry != 0) {
			if (curEntry->key() == key ) {
				return &curEntry->value();
			}
			curEntry=curEntry->nextEntry;
		}
		return 0;
	}
	T * firstEntry;

private: 
#ifndef BUGGYGPP
	NOCOPYANDASSIGN(Mapper<T>)
#else /* BUGGYGPP */

#define COMMA ,
	NOCOPYANDASSIGN(Mapper<T COMMA K_Type COMMA V_Type>)
#undef COMMA

#endif /* BUGGYGPP */

		
};

//lint -esym(1712,KeyValuePair) // no default ctor
template <class K, class V> 
class DLLEXPORT KeyValuePair
{
public:
		typedef K K_Type;
		typedef V V_Type;
		KeyValuePair(const K_Type & k,const V_Type & v, KeyValuePair<K,V> * nextE = 0):
			key_(k), value_(v), nextEntry(nextE) {}
		const K_Type & key()	const { return key_;}
		const V_Type & value()	const { return value_;}
private:
		K_Type key_;
		V_Type value_;
public:
		KeyValuePair<K,V> * nextEntry;

private: 
#define COMMA ,
		NOCOPYANDASSIGN(KeyValuePair<K COMMA V>) 
#undef COMMA
};


typedef KeyValuePair<RSString,RSString> FontMapping ;



//lint -esym(1790,Mapper*)
#ifndef BUGGYGPP
class DLLEXPORT FontMapper: public Mapper<FontMapping>
#else /* BUGGYGPP */
class DLLEXPORT FontMapper: public Mapper<FontMapping,RSString,RSString>
#endif /* BUGGYGPP */
{
public:
  void readMappingTable(ostream & errstream,const char * filename);
  const char * mapFont(const RSString & fontname) const;
};
#if defined (_MSC_VER) 
#pragma warning(default: 4275)
#endif



DLLEXPORT RSString getRegistryValue(ostream& errstream, const char * typekey, const char * key);
DLLEXPORT unsigned long P_GetPathToMyself(const char *name, char * returnbuffer, unsigned long buflen);
DLLEXPORT size_t searchinpath(const char* EnvPath,const char* name, char *returnbuffer,unsigned long buflen);
DLLEXPORT void errorMessage(const char * text); // display an error message (cerr or msgbox)
DLLEXPORT void copy_file(istream& infile,ostream& outfile) ;
DLLEXPORT RSString getOutputFileNameFromPageNumber(const char * const outputFileTemplate, const RSString & pagenumberformatOption, unsigned int pagenumber);

inline float pythagoras(const float x, const float y) { return sqrt( x*x + y*y); }


#endif
 
 
 
 
 
