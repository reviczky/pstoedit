#ifndef _POPTIONS_H
#define _POPTIONS_H
/*
   poptions.h : This file is part of pstoedit
   program option handling 

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

#ifndef cppcomp_h
#include "cppcomp.h"
#endif

#include I_iostream
#include I_ostream
#include I_istream

#include "miscutil.h"

USESTD

enum baseTypeIDs { int_ty, double_ty, bool_ty, char_ty };

class DLLEXPORT IntValueExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, int &result);
	static const char *gettypename() ;
	static unsigned int gettypeID();
};

class DLLEXPORT DoubleValueExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, double &result) ;
	static const char *gettypename() ;
	static unsigned int gettypeID();
};


class DLLEXPORT CharacterValueExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, char &result) ;
	static const char *gettypename() ;
	static unsigned int gettypeID();
};

class DLLEXPORT BoolBaseExtractor {
public:
	static const char *gettypename();
	static unsigned int gettypeID();
};

//lint -esym(1790,BoolBaseExtractor)
class DLLEXPORT BoolInvertingExtractor : public BoolBaseExtractor {
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, bool &result) ;
};

class DLLEXPORT BoolFalseExtractor : public BoolBaseExtractor{
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, bool &result) ;
};

class DLLEXPORT BoolTrueExtractor : public BoolBaseExtractor{
public:
	static bool getvalue(const char *optname, const char *instring, unsigned int &currentarg, bool &result);
};



class DLLEXPORT OptionBase {
public:
	OptionBase(bool optional_p,const char *flag_p, const char *argname_p, int propsheet_p, const char *description_p, const char * TeXhelp_p):
	  flag(flag_p),
	  argname(argname_p),
	  propsheet(propsheet_p),
	  description(description_p),
	  TeXhelp(TeXhelp_p),
	  optional(optional_p),
	  membername(""){ // membername is set during "add" because it was simpler to grab the membername during the add
	};
	virtual ~OptionBase() { membername = 0; }
	virtual ostream & writevalue(ostream & out) const = 0;
#if 0
	void toString(RSString &) const;  
#endif
	virtual bool copyvalue(const char *optname, const char *valuestring, unsigned int &currentarg) = 0;
	virtual bool copyvalue_simple(const char *valuestring) = 0;
//	virtual bool copyvalue_simple(bool boolvalue) = 0; 
	virtual const char *gettypename() const = 0;
	virtual unsigned int gettypeID() const = 0;
	virtual void * GetAddrOfValue() = 0; // will return real type instead of void

	//lint -esym(1540,OptionBase::flag) // not freed
	//lint -esym(1540,OptionBase::description) // not freed
	const char * const flag;		// -bf
	const char * const argname;     // a meaningfull name of the argument (if not a boolean option)
	int propsheet;					// the number of the propertysheet to place this option on
	const char * const description;	// help text
	const char * const TeXhelp;
	bool optional;
	const char * membername;

private:
	OptionBase(); // disabled
	OptionBase(const OptionBase&); // disabled
	const OptionBase& operator=(const OptionBase&); // disabled
};

template <class ValueType, class ExtractorType >
class OptionT : public OptionBase {
public:
	OptionT < ValueType, ExtractorType > (bool optional_p, const char *flag_p, const char *argname_p, int propsheet_p, const char *description_p, const char * TeXhelp_p, const ValueType & initialvalue)	:
		OptionBase(optional_p, flag_p, argname_p, propsheet_p, description_p, TeXhelp_p),
		value(initialvalue) {
	};
	OptionT < ValueType, ExtractorType > (bool optional_p, const char *flag_p, const char *argname_p, int propsheet_p, const char *description_p, const char * TeXhelp_p )	:
		OptionBase(optional_p, flag_p, argname_p, propsheet_p, description_p, TeXhelp_p),
        value() // use default init for value
	{
			//lint -esym(1401,*::value) // not initialized - we use the default ctor here
	};
	virtual ostream & writevalue(ostream & out) const;


	virtual bool copyvalue(const char *optname, const char *valuestring, unsigned int &currentarg) {
		return ExtractorType::getvalue(optname, valuestring, currentarg, value);
	}
	bool copyvalue_simple(const char *valuestring) {
		unsigned int num = 0;
		return copyvalue("no name because of copyvalue_simple",valuestring,num);
	}
#if 0
	bool copyvalue_simple(bool boolvalue  ) {
//		value = boolvalue;
	//	unsigned int num = 0;
		// we cannot just use the "extractor" since that works like the option has given on the commandline
		// but copyvalue_simple is called for all options - but that does not mean that all options are
		// "put on the virtual commandline" - clear ?
	//	return copyvalue("no name because of copyvalue_simple","",num);
		value = boolvalue;
	 	return true; // wogl FIXME
	}
#endif
	virtual const char *gettypename() const {
		return ExtractorType::gettypename();
	}
	virtual unsigned int gettypeID() const {
		return ExtractorType::gettypeID();
	}
	//lint -save -esym(1539,OptionBase::optional) // not assigned a value
	//lint -e(1763)
  	ValueType & operator()() { return value; }
	const ValueType & operator()() const { return value; }
	operator ValueType () const { return value; }
	/* const ValueType & */ void operator =(const ValueType & arg) {
		/* return */ value = arg; 
		//lint -esym(1539,OptionBase::propsheet)  // not assigned in op=
		//lint -esym(1539,OptionBase::membername) // not assigned in op=
	} //  cannot return a reference, because char*::operator= does not
	//lint -restore
	bool operator !=(const ValueType & arg) const { return value != arg; }
	bool operator ==(const ValueType & arg) const { return value == arg; }
	bool operator !() const { return !value ; }
	virtual void * GetAddrOfValue() { return &value;}


	ValueType value;

private:
	OptionT();// disabled
	OptionT(const OptionT&);// disabled
	const OptionT& operator=(const OptionT&); // disabled
};


template <class ValueType, class ExtractorType >
ostream & OptionT<ValueType, ExtractorType>::writevalue(ostream & out) const {
		out << value;
		return out;
}

class DLLEXPORT ProgramOptions {
public:
	ProgramOptions(bool expectUnhandled_p = false) : expectUnhandled(expectUnhandled_p), unhandledCounter(0), optcount(0)   { unhandledOptions[0]=0;alloptions[0]=0; };

	virtual ~ProgramOptions() {}
	unsigned int parseoptions(ostream & outstr, unsigned int argc, const char * const*argv) ;
	// unsigned int sheet: -1 indicates "all"
	void showhelp(ostream & outstr, bool forTeX, bool withdescription, int sheet = -1) const ;
	void dumpunhandled(ostream & outstr) const ;	
#if 0
	void showvalues(ostream & outstr, bool withdescription = true) const ;
#endif
	const OptionBase * const * getOptionConstIterator() const { return &alloptions[0]; }
	OptionBase * const * getOptionIterator() const { return &alloptions[0]; }
	unsigned int numberOfOptions() const { return optcount; }
	virtual bool hideFromDoku(const OptionBase& /* opt */ ) const { return false; } // some options may be hidden, i.e. debug only options


  protected:
	void add(OptionBase * op, const char * const membername) ;

  public:
	bool expectUnhandled; // whether to expect unhandled arguments
	unsigned int unhandledCounter;
	const char *unhandledOptions[100];

  private:
	unsigned int optcount;
	OptionBase *alloptions[100];

  private:
	ProgramOptions(const ProgramOptions&); // disabled
	const ProgramOptions& operator =(const ProgramOptions&); // disabled
};

#endif
 
