#ifndef __drvsvm_h__
#define __drvsvm_h__

/*
   drvsvm.h : This file is part of pstoedit
   Class declaration for the SVM (StarView metafile) output driver.
   The implementation can be found in drvsvm.cpp

   Copyright (C) 2005 Thorsten Behrens (tbehrens at acm.org)

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
#include "genericints.h"

#include <utility>
#include <vector> 


class drvSVM : public drvbase {

public:

    // lifetime
	derivedConstructor(drvSVM);		// macrofied Constructor
	~drvSVM();

	class DriverOptions : public ProgramOptions { 
	public:
		Option < bool, BoolTrueExtractor> mapToArial;
		Option < bool, BoolTrueExtractor> emulateNarrowFonts;

		DriverOptions() :
		mapToArial(true,"-m",0,0,"map to Arial",0,false),
		emulateNarrowFonts(true,"-nf",0,0,"emulate narrow fonts",0,false)
		{
			ADD(mapToArial);
			ADD(emulateNarrowFonts);
		}
	
	} * options;

    // overriding virtual base class methods
    // -------------------------------------

	virtual void ClipPath(cliptype clipmode);
	virtual void Save();
	virtual void Restore();

	virtual void show_image(const PSImage & imageinfo); 
	virtual bool driverOK() const { return isDriverOk; }
 
	typedef GenericInts::Int<32>::signedtype   Int32;
	typedef GenericInts::Int<32>::unsignedtype uInt32;
	typedef GenericInts::Int<16>::signedtype   Int16;
	typedef GenericInts::Int<16>::unsignedtype uInt16;
	typedef GenericInts::Int< 8>::signedtype   Int8;
	typedef GenericInts::Int< 8>::unsignedtype uInt8;

    // include _essential_ overrides (also adds a 'private' declaration)
    // -----------------------------------------------------------------
#include "drvfuncs.h"

	void show_text(const TextInfo & textInfo);

private:

    typedef std::pair<Int32,Int32>			IntPoint;
    typedef std::vector<IntPoint> 			VectorOfPoints;
    typedef std::vector<VectorOfPoints>		VectorOfVectorOfPoints;
    typedef std::vector<uInt8> 				VectorOfFlags;
    typedef std::vector<VectorOfFlags>		VectorOfVectorOfFlags;

    enum LineColorAction
    {
        lineColor,
        noLineColor
    };
    enum FillColorAction
    {
        fillColor,
        noFillColor
    };

    void setAttrs( LineColorAction eLineAction, 
                   FillColorAction eFillAction );
    void write_path( VectorOfVectorOfPoints const& polyPolygon,
                     VectorOfVectorOfFlags const&  polyPolygonFlags );
    void write_polyline( VectorOfVectorOfPoints const& polyPolygon,
                         VectorOfVectorOfFlags const&  polyPolygonFlags );

	// private data of drvsvm
    // ----------------------

    std::ostream::pos_type  headerPos;
    long int				actionCount;
    bool 					isDriverOk;

};

#endif /* #ifndef __drvsvm_h__ */
