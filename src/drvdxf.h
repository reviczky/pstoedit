#ifndef __drvDXF_h
#define __drvDXF_h

/* 
   drvDXF.h : This file is part of pstoedit
   Interface for new driver backends

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
#include "drvbase.h"

class drvDXF : public drvbase {

public:

        derivedConstructor(drvDXF);
	// (const char * driveroptions_P,ostream & theoutStream ,ostream & theerrStream ); // Constructor

        ~drvDXF(); // Destructor
	class DriverOptions : public ProgramOptions {
	public:
		Option < bool, BoolTrueExtractor > polyaslines ;
		Option < bool, BoolTrueExtractor > mm ;
		Option < bool, BoolTrueExtractor > colorsToLayers ;
		Option < bool, BoolTrueExtractor > splineaspolyline ;
		Option < bool, BoolTrueExtractor > splineasnurb ;
		Option < bool, BoolTrueExtractor > splineasbspline ;
		Option < bool, BoolTrueExtractor > splineassinglespline ;
		Option < bool, BoolTrueExtractor > splineasmultispline ;
		Option < bool, BoolTrueExtractor > splineasbezier ;
		Option < int, IntValueExtractor >  splineprecision ;
		Option < bool, BoolTrueExtractor > dumplayernames ;
		Option < RSString, RSStringValueExtractor > layerpositivfilter;
		Option < RSString, RSStringValueExtractor > layernegativfilter;

		DriverOptions():
			polyaslines(true,"-polyaslines",0,0,"use LINE instead of POLYLINE in DXF",0,false),
			mm(true,"-mm",0,0,"use mm coordinates instead of points in DXF (mm=pt/72*25.4)",0,false),
			colorsToLayers(true,"-ctl",0,0,"map colors to layers",0,false),
			splineaspolyline(true,"-splineaspolyline",0,0,"approximate splines with PolyLines (only for -f dxf_s)",0,false),
			splineasnurb(true,"-splineasnurb",0,0,"experimental (only for -f dxf_s)",0,false),
			splineasbspline(true,"-splineasbspline",0,0,"experimental (only for -f dxf_s)",0,false),
			splineassinglespline(true,"-splineassinglespline",0,0,"experimental (only for -f dxf_s)",0,false),
			splineasmultispline(true,"-splineasmultispline",0,0,"experimental (only for -f dxf_s)",0,false),
			splineasbezier(true,"-splineasbezier",0,0,"use Bezier splines in DXF format (only for -f dxf_s)",0,false),
			splineprecision(true,"-splineprecision","number",0,
				"number of samples to take from spline curve when doing approximation with -splineaspolyline or -splineasmultispline - should be >=2 (default 5)",0,5 ),
			dumplayernames(true,"-dumplayernames",0,0,"dump all layer names found to standard output",0,false),
			layerpositivfilter(true,"-layers","string",0,"layers to be shown (comma separated list of layer names, no space)",0,(const char *)""),
			layernegativfilter(true,"-layerfilter","string",0,"layers to be hidden (comma separated list of layer names, no space)",0,(const char *)"")

		{
			ADD(polyaslines);
			ADD(mm);
			ADD(colorsToLayers);
			ADD(splineaspolyline);
			ADD(splineasnurb);
			ADD(splineasbspline);
			ADD(splineassinglespline);
			ADD(splineasmultispline);
			ADD(splineasbezier);
			ADD(splineprecision);
			ADD(dumplayernames);
			ADD(layerpositivfilter);
			ADD(layernegativfilter);
		}
	}*options;

private:
		void drawVertex(const Point & p, bool withlinewidth, int val70 = 0);
		void drawLine(const Point & start,const Point & end);
		void curvetoAsOneSpline(const basedrawingelement & elem, const Point & currentpoint);
		void curvetoAsMultiSpline(const basedrawingelement & elem, const Point & currentpoint);
		void curvetoAsPolyLine(const basedrawingelement & elem, const Point & currentpoint);
		void curvetoAsNurb(const basedrawingelement & elem, const Point & currentpoint);
		void curvetoAsBSpline(const basedrawingelement & elem, const Point & currentpoint);
		void curvetoAsBezier(const basedrawingelement & elem, const Point & currentpoint);
		void writeHandle(ostream & outs) ;
		void writeLayer(float r, float g, float b,const RSString& colorName) ;
		void writelayerentry(ostream & outf, unsigned int color, const char * layername);
		bool wantedLayer(float r, float g, float b,const RSString& colorName)  ; // layer shall be written
		RSString calculateLayerString(float r, float g, float b,const RSString& colorName) ;
		
		
		void printPoint(const Point & p, unsigned short add );
		void writesplinetype(const unsigned short stype);

		enum dxfsplinetype {aspolyline, assinglespline, asmultispline, asnurb, asbspline, asbezier} splinemode;
//       	int polyaslines; // dump polygons as lines
// 		bool mm; //wether to use mm instead of points
		bool formatis14 ; // whether to use acad 14 format
//		unsigned int splineprecision;

//		bool colorsToLayers;
		const char * header_prelayer1;
		const char * header_prelayer2;
		const char * header_postlayer;
		const char * trailer;
		class DXFLayers * layers;
		long handleint;
		double scalefactor; 

		TempFile  tempFile;
		ofstream  &buffer;

#include "drvfuncs.h"

		void show_text(const TextInfo & textInfo);


		NOCOPYANDASSIGN(drvDXF)

};



#endif

 
 
