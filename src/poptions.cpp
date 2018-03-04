/*
   poptions.cpp : This file is part of pstoedit
   program option handling 

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
#include "poptions.h"

#include I_string_h
#include I_stdlib
#include I_strstream

#include <miscutil.h>

USESTD
#define UNUSEDARG(p)
bool IntValueExtractor::getvalue(const char *optname, const char *instring,
								 unsigned int &currentarg, int &result)
{
	if (instring) {
		result = atoi(instring);
		currentarg++;
		return true;
	} else {
		cout << "missing integer argument for " << optname << " option:" << endl;
		return false;
	}
}
const char *IntValueExtractor::gettypename()
{
	return "integer";
}

unsigned int IntValueExtractor::gettypeID()
{
	return int_ty;
}


bool DoubleValueExtractor::getvalue(const char *optname, const char *instring,
									unsigned int &currentarg, double &result)
{
	if (instring) {
		result = atof(instring);
		currentarg++;
		return true;
	} else {
		cout << "missing double argument for " << optname << " option:" << endl;
		return false;
	}
}
const char *DoubleValueExtractor::gettypename()
{
	return "double";
}

unsigned int DoubleValueExtractor::gettypeID()
{
	return double_ty;
}



bool CharacterValueExtractor::getvalue(const char *optname, const char *instring,
									   unsigned int &currentarg, char &result)
{
	if (instring) {
		result = instring[0];
		currentarg++;
		return true;
	} else {
		cout << "missing character argument for " << optname << " option" << endl;
		return false;
	}
}

const char *CharacterValueExtractor::gettypename()
{
	return "character";
}

unsigned int CharacterValueExtractor::gettypeID()
{
	return char_ty;
}

const char *BoolBaseExtractor::gettypename()
{
	return "boolean";
}

unsigned int BoolBaseExtractor::gettypeID()
{
	return bool_ty;
}

bool BoolInvertingExtractor::getvalue(const char *UNUSEDARG(optname),
									  const char *UNUSEDARG(instring),
									  unsigned int &UNUSEDARG(currentarg), bool &result)
{
	result = !result;			// just invert
	return true;
}


bool BoolFalseExtractor::getvalue(const char *UNUSEDARG(optname), const char *UNUSEDARG(instring),
								  unsigned int &UNUSEDARG(currentarg), bool &result)
{
	result = false;
	return true;
}


bool BoolTrueExtractor::getvalue(const char *UNUSEDARG(optname), const char *UNUSEDARG(instring),
								 unsigned int &UNUSEDARG(currentarg), bool &result)
{
	result = true;
	return true;
}


void ProgramOptions::showvalues(ostream & outstr, bool withdescription) const
{
	for (unsigned int i = 0; i < optcount; i++) {
		(void) outstr.width(20);
		outstr << alloptions[i]->flag << "\t : " << alloptions[i]->gettypename() << "\t : ";
		if (withdescription)
			outstr << alloptions[i]->description << "\t : ";
		(void) alloptions[i]->writevalue(outstr);
		outstr << endl;
	}
}

void OptionBase::toString(RSString & result) const
{
	C_ostrstream tempstream;
	(void) writevalue(tempstream);
	tempstream << ends;
#ifdef  USE_NEWSTRSTREAM
	const string str = tempstream.str();
	// no freeze / delete needed since ostringstream::str returns a string and not char*
	result = str.data();
#else
	result = tempstream.str();
	tempstream.rdbuf()->freeze(0);
#endif
}

unsigned int ProgramOptions::parseoptions(ostream & outstr, unsigned int argc,
										  const char *const *argv)
{
	unsigned int i = 1;			// argv[0] is not of interest
	//debug outstr << "parsing options: argc : " << argc << endl;
	while (i < argc) {
		bool found = false;
		for (unsigned int j = 0; j < optcount; j++) {
			const char *optid = alloptions[j]->flag;
			if (strcmp(optid, argv[i]) == 0) {
				//debug outstr << " found arg:" << i << " " << argv[i] << endl;
				const char *nextarg = (i < argc) ? argv[i + 1] : (const char *) 0;
				if (!alloptions[j]->copyvalue(argv[i], nextarg, i)) {
					outstr << "Error in option " << argv[i] << endl;
				}
				found = true;
				//debug outstr << " after found arg:" << i << " " << argv[i] << endl;
				break;
			}
		}
		if (!found) {
			if ((strlen(argv[i]) > 1) && (argv[i])[0] == '-') {
				outstr << "unknown option " << argv[i] << endl;
			} else {
				unhandledOptions[unhandledCounter] = argv[i];
				++unhandledCounter;
			}
		}
		i++;
	}
	return unhandledCounter;
}

static void TeXescapedOutput(ostream & outstr, const char *const st)
{
	const char *s = st;
	while (s && *s) {
		switch (*s) {
		case '[':
			outstr << "\\Lbr";
			break;
		case ']':
			outstr << "\\Rbr";
			break;
		default:
			outstr << *s;
			break;
		}
		s++;
	}
}

void ProgramOptions::showhelp(ostream & outstr, bool forTeX, bool withdescription, int sheet) const
{
	if (optcount && forTeX && withdescription) {
		outstr << "\\begin{description}" << endl;
	}
	const char *const terminator = withdescription ? "] " : " ";
	for (unsigned int i = 0; i < optcount; i++) {
		if (forTeX) {
			if ((!hideFromDoku(*(alloptions[i])) && (sheet == -1))
				// -1 means : show all sheets (except the hidden ones)
				|| (alloptions[i]->propsheet == sheet)) {
				if (withdescription)
					outstr << "\\item[";
				if (alloptions[i]->gettypeID() == bool_ty) {
					if (alloptions[i]->optional)
						outstr << "\\oOpt{";
					else
						outstr << "\\Opt{";
					TeXescapedOutput(outstr, alloptions[i]->flag);
					outstr << "}" << terminator << endl;
				} else {
					if (alloptions[i]->optional)
						outstr << "\\oOptArg{";
					else
						outstr << "\\OptArg{";
					TeXescapedOutput(outstr, alloptions[i]->flag);
					outstr << "}";
					const char *aname =
						alloptions[i]->argname ? alloptions[i]->argname : "missing arg name";
					outstr << "{~";
					TeXescapedOutput(outstr, aname);
					outstr << "}" << terminator << endl;
				}
				if (withdescription) {
					const char *help =
						alloptions[i]->TeXhelp ? alloptions[i]->TeXhelp : alloptions[i]->
						description;
					outstr << help << endl << endl;
				}
#if 0
				(void) outstr.width(20);
				outstr << alloptions[i]->flag << "\t : " << alloptions[i]->
					gettypename() << "\t : " << alloptions[i]->description;
				if (alloptions[i]->optional)
					outstr << "]";
#endif
				if (withdescription)
					outstr << endl;
			}

		} else {
			if (alloptions[i]->optional)
				outstr << "[";
			(void) outstr.width(20);
			outstr << alloptions[i]->flag << "\t : " << alloptions[i]->
				gettypename() << "\t : " << alloptions[i]->description;
			if (alloptions[i]->optional)
				outstr << "]";
			outstr << endl;
		}
	}

	if (forTeX && withdescription) {
		if (optcount) {
			outstr << "\\end{description}" << endl;
		} else {
			// this happens only in the context of driver options
			outstr << "No driver specific options" << endl;
		}
	}

}

void ProgramOptions::dumpunhandled(ostream & outstr) const
{
	if (unhandledCounter) {
		outstr << "unhandled parameters " << endl;
		for (unsigned int i = 0; i < unhandledCounter; i++) {
			outstr << '\t' << unhandledOptions[i] << endl;
		}
	} else {
		outstr << "no unhandled parameters" << endl;
	}
}

void ProgramOptions::add(OptionBase * op, const char *const membername_p)
{
	alloptions[optcount] = op;
	op->membername = membername_p;
	alloptions[++optcount] = 0;
}
