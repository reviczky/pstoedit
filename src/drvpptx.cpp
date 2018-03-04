/*
   drvPPTX.cpp : This file is part of pstoedit
   Backend for Office Open XML files
   Contributed by: Scott Pakin <scott+ps2ed_AT_pakin.org>

   Copyright (C) 1993 - 2013 Wolfgang Glunz, wglunz35_AT_pstoedit.net

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

#ifdef _MSC_VER
// avoid this warning
// this macro needs to be define before all the includes
// warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "drvpptx.h"
#include I_fstream
#include I_stdio
#include I_stdlib
#include I_iomanip
#include <cfloat>
#include <time.h>

#include <errno.h>

#ifdef _MSC_VER
// MS VC++ Windows
// _USE_MATH_DEFINES needed on Windows to enable the define M_PI
#define _USE_MATH_DEFINES
#define srandom srand
#define random rand
#include <process.h>

// work-around - missing on WIndows.
long lroundf(float f) {
        return (long)(floor(f +0.5f));
}

#else
#include <unistd.h>
#endif

#include <math.h>

// when linking against static library - otherwise it means declspec(dllextern)
#define ZIP_EXTERN extern

#include <zip.h>

#ifdef _MSC_VER
// MS VC++ Windows
// handle this warning: 
// 'xxxx': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _xxxx. See online help for details.
#define getpid _getpid
#define strdup _strdup
#define unlink _unlink

#endif

// #include "version.h"

/*
  The following are some things to know about the Office Open XML
  (OOXML) DrawingML format (a.k.a. PowerPoint or pptx) that should
  help understand the code in this file:

    - A pptx file is really a zip archive that contains a bunch of XML
      files (plus embedded-image files).

    - These XML files are cross-linked to each other via
      "relationships" (indirections through ID-to-filename mappings).

    - Describing graphics -- or anything else -- requires lots of XML
      verbosity and a substantial amount of boilerplate text.

    - The DrawingML coordinate system places the origin in the
      upper-left corner, in contrast to PostScript, which puts it in
      the lower-left corner.

    - The main unit of measurement is the English Metric Unit (EMU),
      of which there are exactly 914,400 per inch, 360,000 per
      centimeter, and 12,700 per PostScript point.  Note that all of
      those are integers; DrawingML works exclusively with integers.
      As an example, a distance of 10cm (approximately 3.9" or 283
      pt.) is written as "3600000" in OOXML.

    - Angles are specified in 60,000ths of a degree.  Positive numbers
      represent clockwise rotations, and negative numbers represent
      counterclockwise rotations.  For example a 30-degree clockwise
      rotation is written as "1800000" in OOXML.

    - Percentages are expressed in 1000ths and with no trailing "%"
      character.  For example, 75% is written as "75000" in OOXML.

    - Positions are specified as a distance from the upper-left corner
      of the object's bounding box.

    - Coordinates within a shape are specified with the origin in the
      upper-left corner of their bounding box.

    - Scaling is performed relative to the upper-left corner of the
      object's bounding box.  That is, the upper-left corner is
      invariant with respect to scaling.

    - Rotation is performed relative to the center of the object's
      bounding box.

    - The order of object transformations is (1) translate the
      upper-left corner, (2) scale from the upper-left corner, (3)
      flip horizontally/vertically around the image's center, (4)
      rotate around the image's center.

    - Although DrawingML supports horizontal and vertical flipping for
      text, PowerPoint flips only the shape in which the text is
      embedded.  (This box is always an invisible rectangle in the
      case of drvpptx-generated DrawingML.)  The text itself is in
      fact *rotated*, not flipped.  Specifically, horizontally flipped
      text negates the sign of the rotation angle, and vertically
      flipped text subtracts the rotation angle from 180 degrees.  For
      example, if a piece of text is rotated clockwise by 30 degrees
      (angle="1800000"), flipping it horizontally (flipH="1") is
      equivalent to rotating it instead by -30 degrees; flipping it
      vertically (flipV="1") is equivalent to rotating it instead by
      150 degrees.

    - Coordinates are always specified in an unrotated coordinate
      space.

    - DrawingML supports a fairly large subset of the PostScript that
      pstoedit understands.  Omissions include winding-number fills
      (only even-odd fills are supported), dash patterns with nonzero
      offsets, arbitrarily transformed (e.g., skewed) text and images,
      and vertical text kerning produced by PostScript's awidthshow.
      I believe that vertical kerning can probably be faked using sub-
      and superscripts, but that's probably not worth the effort.

  Office Open XML has been ratified as an Ecma International standard
  (ECMA-376).  See
  http://www.ecma-international.org/publications/standards/Ecma-376.htm
  for the complete specification.  Note that Microsoft PowerPoint is
  not always consistent with the ECMA-376 specification and that not
  all of the information listed above appears in the
  specification.
*/

const char * const drvPPTX::xml_rels =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
  "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"ppt/presentation.xml\"/>\n"
  "</Relationships>\n";

const char * const  drvPPTX::xml_slideLayout1_xml =
  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
  "<p:sldLayout preserve=\"1\" type=\"blank\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">\n"
  "  <p:cSld name=\"Blank Slide\">\n"
  "    <p:spTree>\n"
  "      <p:nvGrpSpPr>\n"
  "        <p:cNvPr id=\"1\" name=\"\"/>\n"
  "        <p:cNvGrpSpPr/>\n"
  "        <p:nvPr/>\n"
  "      </p:nvGrpSpPr>\n"
  "      <p:grpSpPr>\n"
  "        <a:xfrm>\n"
  "          <a:off x=\"0\" y=\"0\"/>\n"
  "          <a:ext cx=\"0\" cy=\"0\"/>\n"
  "          <a:chOff x=\"0\" y=\"0\"/>\n"
  "          <a:chExt cx=\"0\" cy=\"0\"/>\n"
  "        </a:xfrm>\n"
  "      </p:grpSpPr>\n"
  "    </p:spTree>\n"
  "  </p:cSld>\n"
  "</p:sldLayout>\n";

const char * const drvPPTX::xml_slideLayout1_xml_rels =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
  "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster\" Target=\"../slideMasters/slideMaster1.xml\"/>\n"
  "</Relationships>\n";

const char * const drvPPTX::xml_slideMaster1_xml =
  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
  "<p:sldMaster xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">\n"
  "  <p:cSld>\n"
  "    <p:spTree>\n"
  "      <p:nvGrpSpPr>\n"
  "        <p:cNvPr id=\"1\" name=\"\"/>\n"
  "        <p:cNvGrpSpPr/>\n"
  "        <p:nvPr/>\n"
  "      </p:nvGrpSpPr>\n"
  "      <p:grpSpPr>\n"
  "        <a:xfrm>\n"
  "          <a:off x=\"0\" y=\"0\"/>\n"
  "          <a:ext cx=\"0\" cy=\"0\"/>\n"
  "          <a:chOff x=\"0\" y=\"0\"/>\n"
  "          <a:chExt cx=\"0\" cy=\"0\"/>\n"
  "        </a:xfrm>\n"
  "      </p:grpSpPr>\n"
  "    </p:spTree>\n"
  "  </p:cSld>\n"
  "  <p:clrMap accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" bg1=\"lt1\" bg2=\"lt2\" folHlink=\"folHlink\" hlink=\"hlink\" tx1=\"dk1\" tx2=\"dk2\"/>\n"
  "  <p:sldLayoutIdLst>\n"
  "    <p:sldLayoutId id=\"2147483649\" r:id=\"rId2\"/>\n"
  "  </p:sldLayoutIdLst>\n"
  "</p:sldMaster>\n";

const char * const drvPPTX::xml_slideMaster1_xml_rels =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
  "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme\" Target=\"../theme/theme1.xml\"/>\n"
  "  <Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout\" Target=\"../slideLayouts/slideLayout1.xml\"/>\n"
  "</Relationships>\n";

const char * const drvPPTX::xml_theme1_xml =
  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
  "<a:theme name=\"Office Theme\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">\n"
  "  <a:themeElements>\n"
  "    <a:clrScheme name=\"Office\">\n"
  "      <a:dk1>\n"
  "        <a:sysClr val=\"windowText\" lastClr=\"000000\"/>\n"
  "      </a:dk1>\n"
  "      <a:lt1>\n"
  "        <a:sysClr val=\"window\" lastClr=\"FFFFFF\"/>\n"
  "      </a:lt1>\n"
  "      <a:dk2>\n"
  "        <a:srgbClr val=\"1F497D\"/>\n"
  "      </a:dk2>\n"
  "      <a:lt2>\n"
  "        <a:srgbClr val=\"EEECE1\"/>\n"
  "      </a:lt2>\n"
  "      <a:accent1>\n"
  "        <a:srgbClr val=\"4F81BD\"/>\n"
  "      </a:accent1>\n"
  "      <a:accent2>\n"
  "        <a:srgbClr val=\"C0504D\"/>\n"
  "      </a:accent2>\n"
  "      <a:accent3>\n"
  "        <a:srgbClr val=\"9BBB59\"/>\n"
  "      </a:accent3>\n"
  "      <a:accent4>\n"
  "        <a:srgbClr val=\"8064A2\"/>\n"
  "      </a:accent4>\n"
  "      <a:accent5>\n"
  "        <a:srgbClr val=\"4BACC6\"/>\n"
  "      </a:accent5>\n"
  "      <a:accent6>\n"
  "        <a:srgbClr val=\"F79646\"/>\n"
  "      </a:accent6>\n"
  "      <a:hlink>\n"
  "        <a:srgbClr val=\"0000FF\"/>\n"
  "      </a:hlink>\n"
  "      <a:folHlink>\n"
  "        <a:srgbClr val=\"800080\"/>\n"
  "      </a:folHlink>\n"
  "    </a:clrScheme>\n"
  "    <a:fontScheme name=\"Office\">\n"
  "      <a:majorFont>\n"
  "        <a:latin typeface=\"Arial\"/>\n"
  "        <a:ea typeface=\"DejaVu Sans\"/>\n"
  "        <a:cs typeface=\"DejaVu Sans\"/>\n"
  "      </a:majorFont>\n"
  "      <a:minorFont>\n"
  "        <a:latin typeface=\"Arial\"/>\n"
  "        <a:ea typeface=\"DejaVu Sans\"/>\n"
  "        <a:cs typeface=\"DejaVu Sans\"/>\n"
  "      </a:minorFont>\n"
  "    </a:fontScheme>\n"
  "    <a:fmtScheme name=\"Office\">\n"
  "      <a:fillStyleLst>\n"
  "        <a:solidFill>\n"
  "          <a:schemeClr val=\"phClr\"/>\n"
  "        </a:solidFill>\n"
  "        <a:gradFill rotWithShape=\"1\">\n"
  "          <a:gsLst>\n"
  "            <a:gs pos=\"0\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:tint val=\"50000\"/>\n"
  "                <a:satMod val=\"300000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"35000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:tint val=\"37000\"/>\n"
  "                <a:satMod val=\"300000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"100000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:tint val=\"15000\"/>\n"
  "                <a:satMod val=\"350000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "          </a:gsLst>\n"
  "          <a:lin ang=\"16200000\" scaled=\"1\"/>\n"
  "        </a:gradFill>\n"
  "        <a:gradFill rotWithShape=\"1\">\n"
  "          <a:gsLst>\n"
  "            <a:gs pos=\"0\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:shade val=\"51000\"/>\n"
  "                <a:satMod val=\"130000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"80000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:shade val=\"93000\"/>\n"
  "                <a:satMod val=\"130000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"100000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:shade val=\"94000\"/>\n"
  "                <a:satMod val=\"135000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "          </a:gsLst>\n"
  "          <a:lin ang=\"16200000\" scaled=\"0\"/>\n"
  "        </a:gradFill>\n"
  "      </a:fillStyleLst>\n"
  "      <a:lnStyleLst>\n"
  "        <a:ln w=\"9525\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\n"
  "          <a:solidFill>\n"
  "            <a:schemeClr val=\"phClr\">\n"
  "              <a:shade val=\"95000\"/>\n"
  "              <a:satMod val=\"105000\"/>\n"
  "            </a:schemeClr>\n"
  "          </a:solidFill>\n"
  "          <a:prstDash val=\"solid\"/>\n"
  "        </a:ln>\n"
  "        <a:ln w=\"25400\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\n"
  "          <a:solidFill>\n"
  "            <a:schemeClr val=\"phClr\"/>\n"
  "          </a:solidFill>\n"
  "          <a:prstDash val=\"solid\"/>\n"
  "        </a:ln>\n"
  "        <a:ln w=\"38100\" cap=\"flat\" cmpd=\"sng\" algn=\"ctr\">\n"
  "          <a:solidFill>\n"
  "            <a:schemeClr val=\"phClr\"/>\n"
  "          </a:solidFill>\n"
  "          <a:prstDash val=\"solid\"/>\n"
  "        </a:ln>\n"
  "      </a:lnStyleLst>\n"
  "      <a:effectStyleLst>\n"
  "        <a:effectStyle>\n"
  "          <a:effectLst>\n"
  "            <a:outerShdw blurRad=\"40000\" dist=\"20000\" dir=\"5400000\" rotWithShape=\"0\">\n"
  "              <a:srgbClr val=\"000000\">\n"
  "                <a:alpha val=\"38000\"/>\n"
  "              </a:srgbClr>\n"
  "            </a:outerShdw>\n"
  "          </a:effectLst>\n"
  "        </a:effectStyle>\n"
  "        <a:effectStyle>\n"
  "          <a:effectLst>\n"
  "            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\n"
  "              <a:srgbClr val=\"000000\">\n"
  "                <a:alpha val=\"35000\"/>\n"
  "              </a:srgbClr>\n"
  "            </a:outerShdw>\n"
  "          </a:effectLst>\n"
  "        </a:effectStyle>\n"
  "        <a:effectStyle>\n"
  "          <a:effectLst>\n"
  "            <a:outerShdw blurRad=\"40000\" dist=\"23000\" dir=\"5400000\" rotWithShape=\"0\">\n"
  "              <a:srgbClr val=\"000000\">\n"
  "                <a:alpha val=\"35000\"/>\n"
  "              </a:srgbClr>\n"
  "            </a:outerShdw>\n"
  "          </a:effectLst>\n"
  "          <a:scene3d>\n"
  "            <a:camera prst=\"orthographicFront\">\n"
  "              <a:rot lat=\"0\" lon=\"0\" rev=\"0\"/>\n"
  "            </a:camera>\n"
  "            <a:lightRig rig=\"threePt\" dir=\"t\">\n"
  "              <a:rot lat=\"0\" lon=\"0\" rev=\"1200000\"/>\n"
  "            </a:lightRig>\n"
  "          </a:scene3d>\n"
  "          <a:sp3d>\n"
  "            <a:bevelT w=\"63500\" h=\"25400\"/>\n"
  "          </a:sp3d>\n"
  "        </a:effectStyle>\n"
  "      </a:effectStyleLst>\n"
  "      <a:bgFillStyleLst>\n"
  "        <a:solidFill>\n"
  "          <a:schemeClr val=\"phClr\"/>\n"
  "        </a:solidFill>\n"
  "        <a:gradFill rotWithShape=\"1\">\n"
  "          <a:gsLst>\n"
  "            <a:gs pos=\"0\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:tint val=\"40000\"/>\n"
  "                <a:satMod val=\"350000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"40000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:tint val=\"45000\"/>\n"
  "                <a:shade val=\"99000\"/>\n"
  "                <a:satMod val=\"350000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"100000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:shade val=\"20000\"/>\n"
  "                <a:satMod val=\"255000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "          </a:gsLst>\n"
  "          <a:path path=\"circle\">\n"
  "            <a:fillToRect l=\"50000\" t=\"-80000\" r=\"50000\" b=\"180000\"/>\n"
  "          </a:path>\n"
  "        </a:gradFill>\n"
  "        <a:gradFill rotWithShape=\"1\">\n"
  "          <a:gsLst>\n"
  "            <a:gs pos=\"0\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:tint val=\"80000\"/>\n"
  "                <a:satMod val=\"300000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "            <a:gs pos=\"100000\">\n"
  "              <a:schemeClr val=\"phClr\">\n"
  "                <a:shade val=\"30000\"/>\n"
  "                <a:satMod val=\"200000\"/>\n"
  "              </a:schemeClr>\n"
  "            </a:gs>\n"
  "          </a:gsLst>\n"
  "          <a:path path=\"circle\">\n"
  "            <a:fillToRect l=\"50000\" t=\"50000\" r=\"50000\" b=\"50000\"/>\n"
  "          </a:path>\n"
  "        </a:gradFill>\n"
  "      </a:bgFillStyleLst>\n"
  "    </a:fmtScheme>\n"
  "  </a:themeElements>\n"
  "</a:theme>\n";



drvPPTX::derivedConstructor(drvPPTX):
//(const char * driveroptions_p,ostream & theoutStream,ostream & theerrStream): // Constructor
constructBase,
outzip(NIL)
{
  // Parse our command-line options.
  if (options->colortype == "original")
    color_type = C_ORIGINAL;
  else if (options->colortype == "theme")
    color_type = C_THEME;
  else if (options->colortype == "theme-pure")
    color_type = C_THEME_PURE;
  else {
    errorMessage("ERROR: -colors must be either \"original\", \"theme\", or \"theme-pure\"");
    abort();
  }
  if (options->fonttype == "windows")
    font_type = F_WINDOWS;
  else if (options->fonttype == "native")
    font_type = F_NATIVE;
  else if (options->fonttype == "theme")
    font_type = F_THEME;
  else {
    errorMessage("ERROR: -fonts must be one of \"windows\", \"native\", or \"theme\"");
    abort();
  }
  if (options->embeddedfonts != "") {
    // Loop over each EOT filename.
    stringstream embed_stream(RSString(options->embeddedfonts).value());
    string efile;
    while (getline(embed_stream, efile, ',')) {
      // Ensure the file exists, then add it to the list.
      const char * const efile_cstr = efile.c_str();
      if (!fileExists(efile_cstr)) {
        errorMessage((RSString("ERROR: Cannot open file ") + efile_cstr).value());
        abort();
      }
      eotlist.insert(efile);
    }
  }

  // Map PostScript core font names to their Windows + PANOSE replacements.
  if (font_type == F_WINDOWS) {
    ps2win.insert("Courier",                 "Courier New,Courier New,02070309020205020404");
    ps2win.insert("Courier-Bold",            "Courier New Bold,Courier New,02070609020205020404");
    ps2win.insert("Courier-BoldOblique",     "Courier New Bold Italic,Courier New,02070609020205090404");
    ps2win.insert("Courier-Oblique",         "Courier New Italic,Courier New,02070409020205090404");
    ps2win.insert("Helvetica",               "Arial,Arial,020b0604020202020204");
    ps2win.insert("Helvetica-Bold",          "Arial Bold,Arial,020b0704020202020204");
    ps2win.insert("Helvetica-BoldOblique",   "Arial Bold Italic,Arial,020b0704020202090204");
    ps2win.insert("Helvetica-Oblique",       "Arial Italic,Arial,020b0604020202090204");
    ps2win.insert("Times-Bold",              "Times New Roman Bold,Times New Roman,02020803070505020304");
    ps2win.insert("Times-BoldItalic",        "Times New Roman Bold Italic,Times New Roman,02020703060505090304");
    ps2win.insert("Times-Italic",            "Times New Roman Italic,Times New Roman,02020503050405090304");
    ps2win.insert("Times-Roman",             "Times New Roman,Times New Roman,02020603050405020304");
  }

  // Output all floating-point numbers as integers.
  slidef << fixed << setprecision(0);

  // Seed the random-number generator.
  srandom((unsigned int) time(NULL)*getpid());

  // Create a zip archive for holding PresentationML data.
  create_pptx();

  // Specify the slide dimensions (must match ppt/presentation.xml's
  // <p:sldSz> tag).
  slideBBox.ll = Point(0, 0);
  slideBBox.ur = Point(10080625/12700.0, 7559675/12700.0);

  // Number IDs from 1 and images from 0.
  next_id = 1;
  total_images = 0;
  page_images = 0;
}

drvPPTX::~drvPPTX()
{
  // Embed fonts in the PPTX file if asked to do so.
  if (!eotlist.empty()) {
    unsigned int fontNum = 1;
    for (set<string>::iterator iter = eotlist.begin();
         iter != eotlist.end();
         ++iter) {
      const char * const eotFileName = iter->c_str();
      struct zip_source *font_file = zip_source_file(outzip, eotFileName, 0, -1);
      if (font_file == NULL) {
        errorMessage((RSString("ERROR: Failed to embed font file ") +
                      eotFileName + " (" + zip_strerror(outzip) + ")").value());
        abort();
      }
      ostringstream full_eot_filename;
      full_eot_filename << "ppt/fonts/font" << fontNum << ".fntdata";
      if (zip_add(outzip, full_eot_filename.str().c_str(), font_file) == -1) {
        errorMessage((RSString("ERROR: Failed to embed font file ") +
                      eotFileName + " as " + full_eot_filename.str().c_str() +
                      " (" + zip_strerror(outzip) + ")").value());
        abort();
      }
      fontNum++;
    }
  }

  // Create the presentation file.
  ostringstream xml_presentation_xml;
  xml_presentation_xml <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
    "<p:presentation xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
                       << (eotlist.empty() ? ">\n" : " embedTrueTypeFonts=\"1\">\n");
  xml_presentation_xml <<
    "  <p:sldMasterIdLst>\n"
    "    <p:sldMasterId id=\"2147483648\" r:id=\"rId2\"/>\n"
    "  </p:sldMasterIdLst>\n"
    "  <p:sldIdLst>\n";
  for (unsigned int p = 0; p < totalNumberOfPages(); p++)
    xml_presentation_xml << "    <p:sldId id=\"" << p+256
                         << "\" r:id=\"rId" << p+3 << "\"/>\n";
  xml_presentation_xml << "  </p:sldIdLst>\n"
    "  <p:sldSz cx=\"10080625\" cy=\"7559675\"/>\n"
    "  <p:notesSz cx=\"7772400\" cy=\"10058400\"/>\n";
  if (!eotlist.empty()) {
    unsigned int rId = totalNumberOfPages() + 3;
    xml_presentation_xml << "  <p:embeddedFontLst>\n";
    for (set<string>::iterator iter = eotlist.begin();
         iter != eotlist.end();
         ++iter) {
      // Get information about the font.
      TextInfo * textinfo = eot2texinfo(*iter);
      RSString typeface;
      RSString panose;
      bool isBold;
      bool isItalic;
      unsigned char pitchFamily;
      get_font_props(*textinfo, &typeface, &panose, &isBold, &isItalic, &pitchFamily);

      // Describe the font to be embedded.
      xml_presentation_xml << "    <p:embeddedFont>\n"
                           << "      <p:font typeface=\"" << typeface
                           << "\" pitchFamily=\"" << (unsigned int)pitchFamily
                           << "\" charset=\"0\"/>\n";
      switch (int(isBold)*2 + int(isItalic)) {
      case 0:
        xml_presentation_xml << "      <p:regular r:id=\"rId" << rId << "\"/>\n";
        break;

      case 1:
        xml_presentation_xml << "      <p:italic r:id=\"rId" << rId << "\"/>\n";
        break;

      case 2:
        xml_presentation_xml << "      <p:bold r:id=\"rId" << rId << "\"/>\n";
        break;

      case 3:
        xml_presentation_xml << "      <p:boldItalic r:id=\"rId" << rId << "\"/>\n";
        break;

      default:
        errf << "\t\tERROR: unexpected case in drvpptx " << endl;
        abort();
        break;
      }
      xml_presentation_xml << "    </p:embeddedFont>\n";
      rId++;
    }
    xml_presentation_xml << "  </p:embeddedFontLst>\n";
  }
  xml_presentation_xml << "</p:presentation>\n";
  create_pptx_file("ppt/presentation.xml", xml_presentation_xml.str().c_str());

  // Create the presentation relationships file.
  ostringstream xml_presentation_xml_rels;
  xml_presentation_xml_rels <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme\" Target=\"theme/theme1.xml\"/>\n"
    "  <Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster\" Target=\"slideMasters/slideMaster1.xml\"/>\n";
  for (unsigned int p = 0; p < totalNumberOfPages(); p++)
    xml_presentation_xml_rels << "  <Relationship Id=\"rId" << p+3
                              << "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide\" Target=\"slides/slide" << p+1
                              << ".xml\"/>\n";
  if (!eotlist.empty()) {
    unsigned int fontNum = 1;
    unsigned int rId = totalNumberOfPages() + 3;
    for (set<string>::iterator iter = eotlist.begin();
         iter != eotlist.end();
         ++iter) {
      xml_presentation_xml_rels << "  <Relationship Id=\"rId" << rId
                                << "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/font\""
                                << " Target=\"fonts/font" << fontNum << ".fntdata\"/>\n";
      fontNum++;
      rId++;
    }
  }
  xml_presentation_xml_rels << "</Relationships>\n";
  create_pptx_file("ppt/_rels/presentation.xml.rels", xml_presentation_xml_rels.str().c_str());

  // Create the content-types file.
  ostringstream xml_content_types;
  xml_content_types <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n"
    "  <Override PartName=\"/_rels/.rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
    "  <Override PartName=\"/ppt/_rels/presentation.xml.rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
    "  <Override PartName=\"/ppt/slideLayouts/_rels/slideLayout1.xml.rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
    "  <Override PartName=\"/ppt/slideLayouts/slideLayout1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml\"/>\n"
    "  <Override PartName=\"/ppt/theme/theme1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.theme+xml\"/>\n"
    "  <Override PartName=\"/ppt/slideMasters/_rels/slideMaster1.xml.rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
    "  <Override PartName=\"/ppt/slideMasters/slideMaster1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml\"/>\n"
    "  <Override PartName=\"/ppt/presentation.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml\"/>\n";
  for (unsigned int p = 0; p < totalNumberOfPages(); p++)
    xml_content_types << "  <Override PartName=\"/ppt/slides/_rels/slide" << p+1
                      << ".xml.rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
                      << "  <Override PartName=\"/ppt/slides/slide" << p+1
                      << ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slide+xml\"/>\n";
  if (total_images > 0)
    xml_content_types << "  <Default Extension=\"png\" ContentType=\"image/png\"/>\n";
  if (!eotlist.empty())
    xml_content_types << "  <Default Extension=\"fntdata\" ContentType=\"application/x-fontdata\"/>\n";
  xml_content_types << "</Types>\n";
  create_pptx_file("[Content_Types].xml", xml_content_types.str().c_str());

  // Write the PPTX file to disk.
  if (zip_close(outzip) == -1) {
      errorMessage((RSString("ERROR: Failed to generate ") + outFileName +
                    " (" + zip_strerror(outzip) + ")").value());
      abort();
  }
}

// Create a file in a PPTX package from given contents.
void drvPPTX::create_pptx_file(const char * relname, const char * contents)
{
  // Convert the file contents into a data source.
  struct zip_source * file_source = zip_source_buffer(outzip, strdup(contents), strlen(contents), 1);
  if (file_source == NULL) {
    errorMessage((RSString("ERROR: Failed to create data for ") + relname +
                  " (" + zip_strerror(outzip) + ")").value());
    abort();
  }

  // Add the data source to the PPTX file.
  if (zip_add(outzip, relname, file_source) == -1) {
    errorMessage((RSString("ERROR: Failed to insert ") + relname +
                  " into " + outFileName + " (" +
                  zip_strerror(outzip) + ")").value());
    abort();
  }
}

// Create a PPTX file from scratch and add some of the boilerplate.
void drvPPTX::create_pptx()
{
  // Create a PPTX file for writing.
  unlink(outFileName.value());
  int ziperr;
  outzip = zip_open(outFileName.value(), ZIP_CREATE, &ziperr);
  if (outzip == NULL) {
    char reason[101];
    zip_error_to_str(reason, 100, ziperr, errno);
    errorMessage((RSString("ERROR: Failed to create ") + outFileName + " (" + reason + ")").value());
    abort();
  }
  RSString comment = RSString("Created by pstoedit's pptx driver from PostScript input ") + inFileName;
  zip_set_archive_comment(outzip, comment.value(), (zip_uint16_t)comment.length());

  // Insert boilerplate files into the PPTX archive.
  create_pptx_file("_rels/.rels", xml_rels);
  create_pptx_file("ppt/slideLayouts/slideLayout1.xml", xml_slideLayout1_xml);
  create_pptx_file("ppt/slideLayouts/_rels/slideLayout1.xml.rels", xml_slideLayout1_xml_rels);
  create_pptx_file("ppt/slideMasters/slideMaster1.xml", xml_slideMaster1_xml);
  create_pptx_file("ppt/slideMasters/_rels/slideMaster1.xml.rels", xml_slideMaster1_xml_rels);
  create_pptx_file("ppt/theme/theme1.xml", xml_theme1_xml);
}

void drvPPTX::print_coords(const BBox & pathBBox)
{
  // Output a list of coordinates in the shape's coordinate system.
  long int xshift_emu = -xtrans(pathBBox.ll.x_);
  long int yshift_emu = -ytrans(pathBBox.ur.y_);
  for (unsigned int n = 0; n < numberOfElementsInPath(); n++) {
    const basedrawingelement & elem = pathElement(n);
    switch (elem.getType()) {
    case moveto:
      {
        const Point & p = elem.getPoint(0);
        slidef << "                <a:moveTo>\n"
               << "                  <a:pt "
               << pt2emu(p.x_, p.y_, xshift_emu, yshift_emu) << "/>\n"
               << "                </a:moveTo>\n";
      }
      break;
    case lineto:
      {
        const Point & p = elem.getPoint(0);
        slidef << "                <a:lnTo>\n"
               << "                  <a:pt "
               << pt2emu(p.x_, p.y_, xshift_emu, yshift_emu) << "/>\n"
               << "                </a:lnTo>\n";
      }
      break;
    case curveto:
      {
        slidef << "                <a:cubicBezTo>\n";
        for (unsigned int cp = 0; cp < 3; cp++) {
          const Point & p = elem.getPoint(cp);
          slidef << "                  <a:pt "
                 << pt2emu(p.x_, p.y_, xshift_emu, yshift_emu) << "/>\n";
        }
        slidef << "                </a:cubicBezTo>\n";
      }
      break;
    case closepath:
      slidef << "                <a:close/>\n";
      break;
    default:
      errf << "\t\tERROR: unexpected case in drvpptx " << endl;
      abort();
      break;
    }
  }
}

void drvPPTX::open_page()
{
  // Determine how much to offset the current page to center its
  // graphics within the slide.
  BBox pageBBox = getCurrentBBox();
  center_offset.x_ = (slideBBox.ur.x_ - slideBBox.ll.x_ - (pageBBox.ur.x_ - pageBBox.ll.x_)) / 2.0f;
  center_offset.y_ = (slideBBox.ur.y_ - slideBBox.ll.y_ - (pageBBox.ur.y_ - pageBBox.ll.y_)) / 2.0f;

  // Output OOXML header boilerplate.
  slidef << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
         << "<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"\n"
         << "       xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"\n"
         << "       xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">\n"
         << "  <p:cSld>\n"
         << "    <p:spTree>\n"
         << "      <p:nvGrpSpPr>\n"
         << "        <p:cNvPr id=\"1\" name=\"\"/>\n"
         << "        <p:cNvGrpSpPr/>\n"
         << "        <p:nvPr/>\n"
         << "      </p:nvGrpSpPr>\n"
         << "      <p:grpSpPr>\n"
         << "        <a:xfrm>\n"
         << "          <a:off x=\"0\" y=\"0\"/>\n"
         << "          <a:ext cx=\"0\" cy=\"0\"/>\n"
         << "          <a:chOff x=\"0\" y=\"0\"/>\n"
         << "          <a:chExt cx=\"0\" cy=\"0\"/>\n"
         << "        </a:xfrm>\n"
         << "      </p:grpSpPr>\n";

  // Reset the image count.
  page_images = 0;
}

void drvPPTX::close_page()
{
  // Output OOXML trailer boilerplate.
  slidef << "    </p:spTree>\n"
         << "  </p:cSld>\n"
         << "</p:sld>\n";

  // Add the current slide to the PPTX file.
  const char * const slideContents_c = strdup(slidef.str().c_str());
  struct zip_source * slideContents = zip_source_buffer(outzip, slideContents_c, strlen(slideContents_c), 1);
  ostringstream slideFileName;
  slideFileName << "ppt/slides/slide" << currentPageNumber << ".xml";
  const char * const slideFileName_c = strdup(slideFileName.str().c_str());  // libzip seems to store a pointer to this.
  if (zip_add(outzip, slideFileName_c, slideContents) == -1) {
    errorMessage((RSString("ERROR: Failed to store ") + slideFileName_c + " in " + outFileName + " (" + zip_strerror(outzip) + ")").value());
    abort();
  }

  // Clear the slide contents in preparation for the next page.
  slidef.str("");
  slidef.clear();

  // Create a relationships file for the current slide.
  ostringstream slideRelName;
  slideRelName << "ppt/slides/_rels/slide" << currentPageNumber << ".xml.rels";
  ostringstream slideRelContents;
  slideRelContents <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
    "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout\" Target=\"../slideLayouts/slideLayout1.xml\"/>\n";
  for (unsigned int i = 0; i < page_images; i++)
    slideRelContents << "  <Relationship Id=\"rId" << i + 2
                     << "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"../media/image"
                     << total_images - page_images + i + 1 << ".png\"/>\n";
  slideRelContents  << "</Relationships>\n";
  create_pptx_file(slideRelName.str().c_str(), slideRelContents.str().c_str());
}

unsigned char drvPPTX::panose2pitch  (const unsigned int * panose_vals)
{
  // Convert a PANOSE characterization to an OOXML pitch family.
  unsigned char pitchFamily = 0;
  switch (panose_vals[0]) {
  case 3:
    // "Latin script"
    pitchFamily = 0x40;  // "Script"
    break;
  case 4:
    // "Latin decorative"
    pitchFamily = 0x50;  // "Decorative"
    break;
  default:
    // "Latin text" or other
    pitchFamily = (panose_vals[1] >= 11 && panose_vals[1] <= 13 ? 0x20 : 0x10);  // "Swiss" or "Roman"
    break;
  }
  pitchFamily |= (panose_vals[3] == 9 ? 0x01 : 0x02);  // "Fixed" or "Variable"
  return pitchFamily;
}

void drvPPTX::get_font_props(const TextInfo & textinfo,
                             RSString * typeface, RSString * panose,
                             bool * isBold, bool * isItalic,
                             unsigned char * pitchFamily)
{
  // Replace PostScript core fonts with Windows fonts.
  RSString currentFontName(textinfo.currentFontName);
  if (font_type == F_WINDOWS) {
    const RSString * winFont = ps2win.getValue(currentFontName);
    if (winFont != NULL)
      currentFontName = *winFont;
  }

  // Determine properties of the given font.
  unsigned int panose_vals[10];
  if (currentFontName.contains(',')) {
    // Split the font name at commas into <full name>,<family name>,<panose>.
    stringstream fontname_stream(currentFontName.value());
    string fullname;
    string familyname;
    string panose_str;
    if (getline(fontname_stream, fullname, ',') &&
        getline(fontname_stream, familyname, ',') &&
        getline(fontname_stream, panose_str, ',') &&
        sscanf(panose_str.c_str(),
               "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               &panose_vals[0], &panose_vals[1], &panose_vals[2],
               &panose_vals[3], &panose_vals[4], &panose_vals[5],
               &panose_vals[6], &panose_vals[7], &panose_vals[8],
               &panose_vals[9]) == 10) {
      // Compute everything from the given PANOSE categorization.
      typeface->copy(familyname.c_str());
      panose->copy(panose_str.c_str());
      *isBold = panose_vals[2] >= 7;      // "Demi" and up
      *isItalic = panose_vals[7] >= 9;    // "Oblique" letterform
      *pitchFamily = panose2pitch(panose_vals);
      return;
    }
  }

  // If we're here, then either the user didn't specify a font map
  // that includes PANOSE data or the current font was not found in
  // the map.  Do the best we can heuristically.
  *typeface = textinfo.currentFontFamilyName;
  if (*typeface == "Courier" || *typeface == "unknown"
      || typeface->length() < currentFontName.length())
    *typeface = currentFontName;
  for (int i = 0; i < 10; i++)
    panose_vals[i] = 0;
  if (currentFontName.contains("Sans") || textinfo.currentFontFullName.contains("Sans")) {
    panose_vals[0] = 2;   // "Latin text"
    panose_vals[1] = 11;  // "Normal sans"
  }
  else {
    if (currentFontName.contains("Script") || textinfo.currentFontFullName.contains("Script")
        || currentFontName.contains("Hand") || textinfo.currentFontFullName.contains("Hand"))
      panose_vals[0] = 3;   // "Latin script"
    else {
      panose_vals[0] = 2;   // "Latin text"
      panose_vals[1] = 2;   // "Cove"
    }
  }
  if (currentFontName.contains("Bold") || textinfo.currentFontFullName.contains("Bold")) {
    *isBold = true;
    panose_vals[2] = 8;   // "Bold"
  }
  else {
    *isBold = false;
    panose_vals[2] = 5;   // "Book"
  }
  if (currentFontName.contains("Italic") || textinfo.currentFontFullName.contains("Italic")
      || currentFontName.contains("Oblique") || textinfo.currentFontFullName.contains("Oblique")) {
    *isItalic = true;
    panose_vals[9] = 9;   // "Contact/oblique"
  }
  else {
    *isItalic = false;
    panose_vals[9] = 2;   // "Contact/normal"
  }
  if (currentFontName.contains("Mono") || textinfo.currentFontFullName.contains("Mono"))
    panose_vals[4] = 9;   // "Monospaced"
  else
    panose_vals[4] = 3;   // "Modern"
  char panose_str[21];
  sprintf(panose_str, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
          panose_vals[0], panose_vals[1], panose_vals[2], panose_vals[3],
          panose_vals[4], panose_vals[5], panose_vals[6], panose_vals[7],
          panose_vals[8], panose_vals[9]);
  panose->copy(panose_str);
  *pitchFamily = panose2pitch(panose_vals);
}

// Fabricate a TextInfo structure from an EOT file header.
drvbase::TextInfo * drvPPTX::eot2texinfo(string eotfilename)
{
  TextInfo * textinfo = new TextInfo();
  unsigned char panose_vals[10];
  unsigned char charvals[4];

  // Parse the EOT header.
  ifstream eotfile(eotfilename.c_str());
  eotfile.ignore(4+4+4+4);                  // Size, font data size, version, flags
  eotfile.read((char *)panose_vals, 10);    // PANOSE values
  eotfile.ignore(1+1+4);                    // Character set, italic, weight
  eotfile.read((char *)charvals, 2);        // Embedding restrictions
  short fstype = charvals[1]<<8 | charvals[0];
  eotfile.read((char *)charvals, 2);        // Magic number
  unsigned short magicnum = charvals[1]<<8 | charvals[0];
  if (magicnum != 0x504c) {
    errorMessage((RSString("ERROR: ") + eotfilename.c_str() + " is not a valid Embedded OpenType (EOT) font file").value());
    abort();
  }
  eotfile.ignore(4+4+4+4+4+4);              // Unicode ranges 1-4 and code page ranges 1-2
  eotfile.ignore(4+4+4+4+4+2);              // Checksum adjustment, reserved 1-4, padding
  eotfile.read((char *)charvals, 2);        // Family-name length
  unsigned short namesize = charvals[1]<<8 | charvals[0];
  char *familyname = new char[namesize];
  eotfile.read(familyname, namesize);       // Family name
  for (unsigned short i = 0; i < namesize/2; i++)
    // Cheesy conversion from Unicode to ASCII
    familyname[i] = familyname[i*2];
  textinfo->currentFontFamilyName = RSString(familyname, namesize/2);
  delete[] familyname;
  eotfile.ignore(2);                        // Padding
  eotfile.read((char *)charvals, 2);        // Style-name length
  namesize = charvals[1]<<8 | charvals[0];
  eotfile.ignore(namesize);                 // Style name
  eotfile.ignore(2);                        // Padding
  eotfile.read((char *)charvals, 2);        // Version-name length
  namesize = charvals[1]<<8 | charvals[0];
  eotfile.ignore(namesize);                 // Version name
  eotfile.ignore(2);                        // Padding
  eotfile.read((char *)charvals, 2);        // Full-name length
  namesize = charvals[1]<<8 | charvals[0];
  char *fullname = new char[namesize];
  eotfile.read(fullname, namesize);         // Full name
  for (unsigned short i = 0; i < namesize/2; i++)
    // Cheesy conversion from Unicode to ASCII
    fullname[i] = fullname[i*2];
  textinfo->currentFontFullName = RSString(fullname, namesize/2);
  delete[] fullname;
  eotfile.close();

  // Warn the user if the font has embedding restrictions.
  if (fstype == 0x0002)
    errf << "WARNING: Font " << textinfo->currentFontFullName << " ("
         << eotfilename << ") indicates that it must not be modified,"
         << " embedded, or exchanged in any manner without first obtaining"
         << " permission from the legal owner.  Do not embed this font"
         << " unless you have obtained such permission.\n";

  // Concatenate the PANOSE data to the font name so get_font_props()
  // can extract and process it.
  char panose_str[22];
  sprintf(panose_str, ",%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
          panose_vals[0], panose_vals[1], panose_vals[2],
          panose_vals[3], panose_vals[4], panose_vals[5],
          panose_vals[6], panose_vals[7], panose_vals[8],
          panose_vals[9]);
  textinfo->currentFontName = textinfo->currentFontFullName + ',' + textinfo->currentFontFamilyName + panose_str;
  return textinfo;
}

// Given two 2-D vectors (represented as Points), return the angle
// from the first to the second.
// http://stackoverflow.com/questions/5188561/signed-angle-between-two-3d-vectors-with-same-origin-within-the-same-plane-reci
// was useful here.
float drvPPTX::angle_between(Point first, Point second)
{
  // Normalize each vector.
  float scale = pythagoras(first.x_, first.y_);
  first.x_ /= scale;
  first.y_ /= scale;
  scale = pythagoras(second.x_, second.y_);
  second.x_ /= scale;
  second.y_ /= scale;

  // Determine the direction of the rotation.
  float direction = first.x_*second.y_ - first.y_*second.x_;

  // Determine the rotation itself.
  float angle = acos(first.x_*second.x_ + first.y_*second.y_) * 180.0f/(float)M_PI;
  if (direction < 0)
    angle = -angle;
  return angle;
}

// Given a six-element PostScript transformation matrix, determine the
// components of the transformation it represents.
void drvPPTX::parse_xform_matrix(const float * origMatrix,
                                 bool * mirrored,
                                 float * xscale, float * yscale,
                                 float * rotation,
                                 float * xtrans, float * ytrans)
{
  // Return the translation then remove it from the matrix.
  float matrix[6];
  for (int i = 0; i < 6; i++)
    matrix[i] = origMatrix[i];
  *xtrans = matrix[4];
  *ytrans = matrix[5];
  matrix[4] = 0.0f;
  matrix[5] = 0.0f;

  // Determine whether the transformation includes mirroring.
  Point xunit(1.0f, 0.0f);
  Point xunit_xform = xunit.transform(matrix);
  Point yunit(0.0f, 1.0f);
  Point yunit_xform = yunit.transform(matrix);
  float rot90 = angle_between(xunit_xform, yunit_xform);
  *mirrored = rot90 < 0;

  // Compute the rotation angle.
  *rotation = angle_between(xunit, xunit_xform);
  if (*mirrored)
    *rotation = fmodf(*rotation + 180.0f, 360.0f);

  // Compute the scaling.
  *xscale = pythagoras(xunit_xform.x_, xunit_xform.y_);
  *yscale = pythagoras(yunit_xform.x_, yunit_xform.y_);
}

void drvPPTX::show_text(const TextInfo & textinfo)
{
  // Output the non-visual shape properties.
  slidef << "      <p:sp>\n"
         << "        <p:nvSpPr>\n"
         << "          <p:cNvPr id=\"" << next_id << "\" name=\"pstoedit " << next_id << "\"/>\n"
         << "          <p:cNvSpPr/>\n"
         << "          <p:nvPr/>\n"
         << "        </p:nvSpPr>\n";
  next_id++;

  // Compute the unrotated text width and height.
  float text_width =                                 // Unrotated width
    pythagoras(textinfo.x_end - textinfo.x, textinfo.y_end - textinfo.y);
  float text_height = textinfo.currentFontSize;      // Unrotated height

  // Determine if the text is flipped horizontally.  We don't test for
  // vertical flipping because this is isomorphic to a horizontal flip
  // plus a rotation.
  bool flipH;
  float xscale, yscale, angle, xtrans, ytrans;
  parse_xform_matrix(textinfo.FontMatrix, &flipH, &xscale, &yscale,
                     &angle, &xtrans, &ytrans);
  if (flipH)
    angle = -angle;

  // Compute the upper-left corner of the rotated text.
  Point text_pivot(textinfo.x, textinfo.y);   // Unrotated lower left
  Point text_ul(textinfo.x, textinfo.y + text_height);   // Unrotated upper left
  Point text_c = text_pivot + Point(text_width/2.0f, text_height/2.0f);   // Unrotated center
  if (flipH) {
    text_ul.x_ -= text_width;
    text_c.x_ -= text_width;
  }

  // Rotate the upper-left corner and center around the original
  // lower-left corner, then unrotate the upper-left corner around the
  // new center.
  Point text_ul_rot = rotate_pt_around(text_ul, angle, text_pivot);
  Point text_c_rot = rotate_pt_around(text_c, angle, text_pivot);
  Point text_ofs = rotate_pt_around(text_ul_rot, -angle, text_c_rot);

  // Output the visual shape properties.
  slidef << "        <p:spPr>\n";
  slidef << "          <a:xfrm";
  if (angle != 0.0f)
    slidef << " rot=\"" << -angle*60000 << '"';
  if (flipH)
    slidef << " flipH=\"1\"";
  slidef << ">\n";
  slidef << "            <a:off " << pt2emu(text_ofs.x_, text_ofs.y_) << "/>\n";
  slidef << "            <a:ext " << pt2emu(text_width, text_height,
                                            0, 0, "cx", "cy", true) << "/>\n"
         << "          </a:xfrm>\n"
         << "          <a:prstGeom prst=\"rect\"/>\n"
         << "        </p:spPr>\n";

  // Get information about the current font.
  RSString typeface;
  RSString panose;
  bool isBold;
  bool isItalic;
  unsigned char pitchFamily;
  get_font_props(textinfo, &typeface, &panose, &isBold, &isItalic, &pitchFamily);

  // Output the text itself.
  slidef << "        <p:txBody>\n"
         << "          <a:bodyPr wrap=\"none\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" rtlCol=\"0\">\n"
         << "            <a:spAutoFit/>\n"
         << "          </a:bodyPr>\n"
         << "          <a:p>\n"
         << "            <a:r>\n"
         << "              <a:rPr dirty=\"1\" smtClean=\"0\" sz=\""
         << textinfo.currentFontSize*100.0 << '"'
         << (isBold ? " b=\"1\"" : "")
         << (isItalic ? " i=\"1\"" : "");
  if (textinfo.ax != 0)
    slidef << " spc=\"" << textinfo.ax*100.0 << '"';
  slidef << ">\n";
  print_color(16, textinfo.currentR, textinfo.currentG, textinfo.currentB);
  switch (font_type) {
  case F_WINDOWS:
  case F_NATIVE:
    slidef << "                <a:latin typeface=\"" << typeface
           << "\" pitchFamily=\"" << (unsigned int)pitchFamily
           << "\" panose=\"" << panose
           << "\" charset=\"0\"/>\n";
    break;
  case F_THEME:
    // Use the theme's default font.
    break;
  default:
    errorMessage("ERROR: Unknown font type");
    abort();
    break;
  }
  slidef << "              </a:rPr>\n"
         << "              <a:t>";
  static bool warned_invalid_char = false;  // true=already issued an invalid-character warning
  for (size_t c = 0; c < textinfo.thetext.length(); c++) {
    unsigned char onechar = textinfo.thetext[c];
    if (onechar < 32 || (onechar >= 128 && onechar < 192)) {
      if (!warned_invalid_char)
        errf << "Warning: Character " << (unsigned int)onechar << " is not allowed in OOXML text; ignoring\n";
        warned_invalid_char = true;
    }
    else
      switch (onechar) {
      case '<':
        slidef << "&lt;";
        break;
      case '>':
        slidef << "&gt;";
        break;
      case '&':
        slidef << "&amp;";
        break;
      default:
        if (onechar < 128)
          slidef << onechar;
        else
          slidef << char(0xC0 | (onechar>>6)) << char(0x80 | (onechar&0x3F));
        break;
      }
  }
  slidef << "</a:t>\n"
         << "            </a:r>\n"
         << "            <a:endParaRPr dirty=\"1\">\n";
  print_color(14, textinfo.currentR, textinfo.currentG, textinfo.currentB);
  slidef << "            </a:endParaRPr>\n"
         << "          </a:p>\n"
         << "        </p:txBody>\n"
         << "      </p:sp>\n";
}



long int drvPPTX::bp2emu (float bp) {
          return lroundf(bp * 12700.0f);
}


const char * drvPPTX::pt2emu(float x_bp, float y_bp,
                             long int xshift_emu, long int yshift_emu,
                             RSString x_name, RSString y_name,
                             bool scaleOnly) const
{
  // Convert a PostScript (x, y) coordinate (in big points with its
  // origin in the lower left) to an XML string (in EMUs with its
  // origin in the upper left).  If scaleOnly is true, then scale the
  // units without translating the coordinates (useful for widths and
  // heights).  Note that the result will get overwritten on the
  // subsequent call.
  static char emuString[100];   // Should be more than enough in practice

  if (scaleOnly)
    sprintf(emuString, "%s=\"%ld\" %s=\"%ld\"",
            x_name.value(), bp2emu(x_bp),
            y_name.value(), bp2emu(y_bp));
  else
    sprintf(emuString, "%s=\"%ld\" %s=\"%ld\"",
            x_name.value(), xtrans(x_bp) + xshift_emu,
            y_name.value(), ytrans(y_bp) + yshift_emu);
  return emuString;
}



// Compute the centroid of a polygon.  Note that we might not have
// been given a polygon, but we'll at least return *something*.
Point drvPPTX::pathCentroid()
{
  // We start by finding a cycle of knots.
  unsigned int numElts = numberOfElementsInPath();
  Point * allKnots = new Point[numElts + 1];
  unsigned int numKnots = 0;
  unsigned int movetos = 0;
  for (unsigned int n = 0; n < numElts; n++) {
    const basedrawingelement & elem = pathElement(n);
    if (elem.getType() == moveto)
      movetos++;
    if (elem.getNrOfPoints() == 0)
      continue;
    allKnots[numKnots++] = elem.getPoint(elem.getNrOfPoints() - 1);
  }
  if (allKnots[numKnots - 1] == allKnots[0])
    numKnots--;
  else
    allKnots[numKnots] = allKnots[0];

  // Otherwise, we compute the area bounded by the knots.
  float area = 0.0f;
  for (unsigned int n = 0; n < numKnots; n++)
    area += allKnots[n].x_*allKnots[n+1].y_ - allKnots[n+1].x_*allKnots[n].y_;
  area /= 2.0f;

  // If we were given a disjoint path or the area is zero, simply
  // average all of the knot coordinates and return that.
  if (movetos > 1 || area == 0.0f) {
    Point centroid;
    for (unsigned int n = 0; n < numKnots; n++)
      centroid += allKnots[n];
    centroid.x_ /= numKnots;
    centroid.y_ /= numKnots;
    delete[] allKnots;
    return centroid;
  }

  // Finally, we compute the centroid of the polygon.
  Point p;
  for (unsigned int n = 0; n < numKnots; n++) {
    float partial = allKnots[n].x_*allKnots[n+1].y_ - allKnots[n+1].x_*allKnots[n].y_;
    p.x_ += (allKnots[n].x_ + allKnots[n+1].x_)*partial;
    p.y_ += (allKnots[n].y_ + allKnots[n+1].y_)*partial;
  }
  p.x_ /= area*6.0f;
  p.y_ /= area*6.0f;
  delete[] allKnots;
  return p;
}

// Rotate point PT by ANGLE degrees around point PIVOT.
Point drvPPTX::rotate_pt_around  (const Point & pt, float angle, const Point & pivot)
{
  Point shiftedPt = pt;
  shiftedPt.x_ -= pivot.x_;   // Shift the pivot to the origin.
  shiftedPt.y_ -= pivot.y_;
  float angle_rad = angle * (float)M_PI / 180.0f;
  Point rotatedPt(shiftedPt.x_*cosf(angle_rad) - shiftedPt.y_*sinf(angle_rad),  // Rotate the shifted point.
                  shiftedPt.x_*sinf(angle_rad) + shiftedPt.y_*cosf(angle_rad));
  return rotatedPt + pivot;   // Shift back to the original pivot.
}

void drvPPTX::print_connections(const BBox & pathBBox)
{
  // Output shape connection sites (knots and centroid).
  Point centroid = pathCentroid();
  long int xshift_emu = -xtrans(pathBBox.ll.x_);
  long int yshift_emu = -ytrans(pathBBox.ur.y_);
  slidef << "            <a:cxnLst>\n"
         << "              <a:cxn ang=\"0\">\n"
         << "                <a:pos " << pt2emu(centroid.x_, centroid.y_,
                                                xshift_emu, yshift_emu) << "/>\n"
         << "              </a:cxn>\n";
  for (unsigned int n = 0; n < numberOfElementsInPath(); n++) {
    const basedrawingelement & elem = pathElement(n);
    if (elem.getNrOfPoints() == 0)
      continue;
    const Point & p = elem.getPoint(elem.getNrOfPoints() - 1);
    float angle = atan2f(centroid.y_ - p.y_, p.x_ - centroid.x_);
    slidef << "              <a:cxn ang=\"" << angle*60000.0*180.0/M_PI << "\">\n"
           << "                <a:pos " << pt2emu(p.x_, p.y_,
                                                  xshift_emu, yshift_emu) << "/>\n"
           << "              </a:cxn>\n";
  }
  slidef << "            </a:cxnLst>\n";
}

void drvPPTX::print_color(int baseIndent, float redF, float greenF, float blueF)
{
  // Output an OOXML solid color.
  string indentStr = string(baseIndent, ' ');
  unsigned int red = (unsigned int)lroundf(redF * 255);
  unsigned int green = (unsigned int)lroundf(greenF * 255);
  unsigned int blue = (unsigned int)lroundf(blueF * 255);
  unsigned int rgb = blue + 256*(green + 256*red);
  slidef << indentStr << "<a:solidFill>\n";
  switch (color_type) {
  case C_ORIGINAL:
    // With -colors=original, output the color exactly as specified.
    slidef << indentStr << "  <a:srgbClr val=\""
           << hex << setw(6) << setfill('0') << rgb << dec << "\"/>\n";
    break;

  case C_THEME:
  case C_THEME_PURE:
    // With -colors=theme, randomly select a theme color.
    if (rgb == 0)
      // Black -> dark 1
      slidef << indentStr << "  <a:schemeClr val=\"dk1\"/>\n";
    else if (rgb == 0xffffff)
      // White -> light 1
      slidef << indentStr << "  <a:schemeClr val=\"lt1\"/>\n";
    else {
      // Randomly select a theme color, but remember it for next time.
      const ThemeColor * colorInfo = rgb2theme.getValue(rgb);
      if (colorInfo == NULL) {
        // This is the first time we've seen this RGB color.
        static const char *colorList[] = {
          "dk2", "lt2", "accent1", "accent2", "accent3",
          "accent4", "accent5", "accent6"
        };
        ThemeColor * newColorInfo = new ThemeColor;
        newColorInfo->name = colorList[random() % (sizeof(colorList)/sizeof(colorList[0]))];
        if (color_type == C_THEME) {
          // Randomly alter the luminosity with the constraint that
          // light colors map to light colors and dark colors map to
          // dark colors.
          float origLum = sqrtf(0.241f*redF*redF + 0.691f*greenF*greenF + 0.068f*blueF*blueF);
          if (origLum >= 0.5)
            newColorInfo->lum = 50000 + random()%40000;  // Map to [50%, 90%].
          else
            newColorInfo->lum = 30000 + random()%20000;  // Map to [30%, 50%].
        }
        rgb2theme.insert(rgb, *newColorInfo);
        colorInfo = newColorInfo;
      }

      // Output the, possibly altered, theme color.
      if (colorInfo->lum == ~0U)
        slidef << indentStr << "  <a:schemeClr val=\"" << colorInfo->name << "\"/>\n";
      else {
        slidef << indentStr << "  <a:schemeClr val=\"" << colorInfo->name << "\">\n"
               << indentStr << "    <a:lum val=\"" << colorInfo->lum << "\"/>\n"
               << indentStr << "  </a:schemeClr>\n";
      }
    }
    break;

  default:
    errorMessage("ERROR: Unexpected color type");
    abort();
    break;
  }
  slidef << indentStr << "</a:solidFill>\n";
}

void drvPPTX::print_join()
{
  // Output the current line join in OOXML format.
  switch (currentLineJoin()) {
  case 0:
    slidef << "            <a:miter/>\n";
    break;
  case 1:
    slidef << "            <a:round/>\n";
    break;
  case 2:
    slidef << "            <a:bevel/>\n";
    break;
  default:
    errorMessage("ERROR: unknown linejoin");
    abort();
    break;
  }
}

void drvPPTX::print_dash()
{
  // Parse a PostScript dash pattern.
  istringstream dashStr(dashPattern());
  float *pattern = new float[2*string(dashPattern()).length()];   // Very generous allocation but expected to be short
  size_t patternLen = 0;    // Number of floats in the above
  string oneToken;
  dashStr >> oneToken;   // "["
  while (dashStr) {
    // Read floats until we reach the "]".  Ignore that and anything
    // that follows it.
    dashStr >> pattern[patternLen];
    if (dashStr.fail())
      break;
    patternLen++;
  }

  // Output an OOXML custom dash.
  if (patternLen > 0) {
    // Repeat odd patterns to make them even.
    size_t p;
    if (patternLen % 2 == 1) {
      for (p = 0; p < patternLen; p++)
        pattern[p + patternLen] = pattern[p];
      patternLen *= 2;
    }

    // Output {dash, space} pairs.
    float lineWidth = currentLineWidth();
    slidef << "            <a:custDash>\n";
    for (p = 0; p < patternLen; p += 2)
      slidef << "              <a:ds d=\"" << 100000.0*pattern[p]/lineWidth
             << "\" sp=\"" << 100000.0*pattern[p+1]/lineWidth << "\"/>\n";
    slidef << "            </a:custDash>\n";
  }
  delete[] pattern;
}

void drvPPTX::show_path()
{
  // Output the non-visible shape properties.
  slidef << "      <p:sp>\n"
         << "        <p:nvSpPr>\n"
         << "          <p:cNvPr id=\"" << next_id << "\" name=\"pstoedit " << next_id << "\"/>\n"
         << "          <p:cNvSpPr/>\n"
         << "          <p:nvPr/>\n"
         << "        </p:nvSpPr>\n";
  next_id++;

  // Compute the path's bounding box.  This might not be perfectly
  // tight due to the way we process curves, but that's not a show
  // stopper.
  BBox pathBBox;
  pathBBox.ll.x_ = FLT_MAX;
  pathBBox.ll.y_ = FLT_MAX;
  pathBBox.ur.x_ = -FLT_MAX;
  pathBBox.ur.y_ = -FLT_MAX;
  Point prevPoint;
  for (unsigned int e = 0; e < numberOfElementsInPath(); e++) {
    // Non-curves are handled by considering each knot in the
    // bounding-box calcuation.
    const basedrawingelement & elem = pathElement(e);
    unsigned int numPoints = elem.getNrOfPoints();
    if (elem.getType() != curveto)
      for (unsigned int p = 0; p < numPoints; p++) {
        Point thisPt = elem.getPoint(p);
        pathBBox.ll.x_ = min(pathBBox.ll.x_, thisPt.x_);
        pathBBox.ll.y_ = min(pathBBox.ll.y_, thisPt.y_);
        pathBBox.ur.x_ = max(pathBBox.ur.x_, thisPt.x_);
        pathBBox.ur.y_ = max(pathBBox.ur.y_, thisPt.y_);
      }

    // Rather than attempt to compute the true bounding box of a
    // curve, we simply sample a large number of evenly spaced points
    // along the curve for our bounding-box calcuation.  This is a lot
    // easier and probably works in virtually all cases.
    if (elem.getType() == curveto) {
      const float numSamples = 100.0f;
      for (float t = 0.0f; t <= 1.0f; t += 1.0f/numSamples) {
        Point bPoint = PointOnBezier(t, prevPoint, elem.getPoint(0),
                                     elem.getPoint(1), elem.getPoint(2));
        pathBBox.ll.x_ = min(pathBBox.ll.x_, bPoint.x_);
        pathBBox.ll.y_ = min(pathBBox.ll.y_, bPoint.y_);
        pathBBox.ur.x_ = max(pathBBox.ur.x_, bPoint.x_);
        pathBBox.ur.y_ = max(pathBBox.ur.y_, bPoint.y_);
      }
    }

    // Keep track of the current point.
    if (numPoints > 0)
      prevPoint = elem.getPoint(numPoints - 1);
  }

  // Output the 2-D transform for the graphic frame (i.e., the shape's
  // offset and size).
  slidef << "        <p:spPr>\n"
         << "          <a:xfrm>\n";
  slidef << "            <a:off " << pt2emu(pathBBox.ll.x_, pathBBox.ur.y_) << "/>\n";
  slidef << "            <a:ext " << pt2emu(pathBBox.ur.x_ - pathBBox.ll.x_,
                                            pathBBox.ur.y_ - pathBBox.ll.y_,
                                            0, 0, "cx", "cy", true) << "/>\n"
         << "          </a:xfrm>\n";

  // For the user's convenience, make each knot a connection site, and
  // specify that any text the user adds should fill the shape's bounding box.
  slidef << "          <a:custGeom>\n";
  print_connections(pathBBox);
  slidef << "            <a:rect l=\"l\" t=\"t\" r=\"r\" b=\"b\"/>\n";

  // Define the coordinate system for the shape within its frame.
  slidef << "            <a:pathLst>\n"
         << "              <a:path " << pt2emu(pathBBox.ur.x_ - pathBBox.ll.x_,
                                               pathBBox.ur.y_ - pathBBox.ll.y_,
                                               0, 0, "w", "h", true) << ">\n";
  // Output all of the shape's lines and curves.
  print_coords(pathBBox);
  slidef << "              </a:path>\n"
         << "            </a:pathLst>\n"
         << "          </a:custGeom>\n";

  // Output a stroke and/or fill.
  if (pathWasMerged() || currentShowType() == drvbase::fill || currentShowType() == drvbase::eofill)
    // Filled region
    print_color(10, fillR(), fillG(), fillB());
  if (pathWasMerged() || currentShowType() == drvbase::stroke) {
    // Stroked line
    slidef << "          <a:ln w=\"" << currentLineWidth()*12700.0
           << "\" cap=\"";
    switch (currentLineCap()) {
    case 0:
      slidef << "flat";
      break;
    case 1:
      slidef << "rnd";
      break;
    case 2:
      slidef << "sq";
      break;
    default:
      errorMessage("ERROR: unknown linecap");
      abort();
      break;
    }
    slidef << "\">\n";
    print_color(12, edgeR(), edgeG(), edgeB());
    print_dash();
    print_join();
    slidef << "          </a:ln>\n";
  }

  // Indicate that if the shape is to be contain a text box, then the
  // text box should be middle-centered within the shape, and the text
  // itself should be centered horizontally within the text box.
  slidef << "        </p:spPr>\n"
         << "        <p:txBody>\n"
         << "          <a:bodyPr wrap=\"none\" lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" rtlCol=\"0\" anchor=\"ctr\" anchorCtr=\"1\"/>\n"
         << "          <a:lstStyle/>\n"
         << "          <a:p>\n"
         << "            <a:pPr algn=\"ctr\"/>\n"
         << "            <a:endParaRPr dirty=\"1\"/>\n"
         << "          </a:p>\n"
         << "        </p:txBody>\n"
         << "      </p:sp>\n";
}

void drvPPTX::show_rectangle(const float llx, const float lly, const float urx, const float ury)
{
  // We could probably generate a rectangle preset if we felt like it.
  unused(&llx); unused(&lly); unused(&urx); unused(&ury); // avoid the compiler warning
  show_path();
}

void drvPPTX::show_image(const PSImage & imageinfo)
{
  // One might think that imageinfo.imageMatrix, which the PostScript
  // Language Reference Manual suggests using to map the unit square
  // to the bounds of the source image, would be useful here.
  // However, experiments suggest that
  // imageinfo.normalizedImageCurrentMatrix in fact provides all the
  // information we need to place an image on a slide.
  const float * ctm = imageinfo.normalizedImageCurrentMatrix;

  // Determine the image's orientation, scale, rotation, and position
  // on the slide.
  // http://stackoverflow.com/questions/4361242/extract-rotation-scale-values-from-2d-transformation-matrix
  // and
  // http://math.stackexchange.com/questions/13150/extracting-rotation-scale-values-from-2d-transformation-matrix
  // were helpful here.
  bool flipH = ctm[0] < 0;
  bool flipV = ctm[3] > 0;    // Reversed sense because we're already flipping the coordinate system
  float xscale = pythagoras(ctm[0], ctm[2]);
  float yscale = pythagoras(ctm[1], ctm[3]);
  float angle = atan2f(ctm[2], ctm[0]) * (float)(180.0f/M_PI);
  if (flipH)
    angle = 180.0f - angle;
  if (flipV)
    angle = -angle;
  long int angle_int = lroundf(-60000.0f*angle);

  // DrawingML rotates the image *after* the location for the image's
  // upper-left corner is specified.  Hence, we determine the image's
  // unrotated upper-left corner by applying the CTM to the image's
  // center and taking an offset from that.
  Point center_orig(imageinfo.width/2.0f, imageinfo.height/2.0f);
  Point center_xform = center_orig.transform(ctm);
  Point ofs = center_xform + Point(-xscale*imageinfo.width/2.0f, yscale*imageinfo.height/2.0f);

  // Place the image on the slide.
  total_images++;
  page_images++;
  slidef << "      <p:pic>\n"
         << "        <p:nvPicPr>\n"
         << "          <p:cNvPr id=\"" << next_id << "\" name=\"pstoedit " << next_id << "\"/>\n"
         << "          <p:cNvPicPr/>\n"
         << "          <p:nvPr/>\n"
         << "        </p:nvPicPr>\n";
  next_id++;
  slidef << "        <p:blipFill>\n"
         << "          <a:blip r:embed=\"rId" << page_images + 1 << "\"/>\n"
         << "          <a:srcRect/>\n"
         << "          <a:stretch>\n"
         << "            <a:fillRect/>\n"
         << "          </a:stretch>\n"
         << "        </p:blipFill>\n";
  slidef << "        <p:spPr bwMode=\"auto\">\n"
         << "          <a:xfrm";
  if (angle_int != 0)
    slidef << " rot=\"" << angle_int << '"';
  if (flipH)
    slidef << " flipH=\"1\"";
  if (flipV)
    slidef << " flipV=\"1\"";
  float cx = imageinfo.width*xscale;
  float cy = imageinfo.height*yscale;
  slidef << ">\n"
         << "            <a:off " << pt2emu(ofs.x_, ofs.y_) << "/>\n";
  slidef << "            <a:ext " << pt2emu(cx, cy, 0, 0, "cx", "cy", true) << "/>\n"
         << "          </a:xfrm>\n"
         << "          <a:prstGeom prst=\"rect\"/>\n"
         << "          <a:noFill/>\n"
         << "        </p:spPr>\n"
         << "      </p:pic>\n";

  // Embed the image in the PPTX file.
  struct zip_source *img_file = zip_source_file(outzip, imageinfo.FileName.value(), 0, -1);
  if (img_file == NULL) {
    errorMessage((RSString("ERROR: Failed to embed image file ") +
                  imageinfo.FileName + " (" + zip_strerror(outzip) + ")").value());
      abort();
  }
  ostringstream img_filename;
  img_filename << "ppt/media/image" << total_images << ".png";
  if (zip_add(outzip, img_filename.str().c_str(), img_file) == -1) {
    errorMessage((RSString("ERROR: Failed to embed image file ") +
                  imageinfo.FileName + " as " + img_filename.str().c_str() +
                  " (" + zip_strerror(outzip) + ")").value());
      abort();
  }
}

static DriverDescriptionT < drvPPTX >
D_pptx("pptx",
       "PresentationML (PowerPoint) format",
       "This is the format used internally by Microsoft PowerPoint.  LibreOffice can also read/write PowerPoint files albeit with some lack of functionality.",
       "pptx",
       true,   // backend supports subpathes
       // if subpathes are supported, the backend must deal with
       // sequences of the following form
       // moveto (start of subpath)
       // lineto (a line segment)
       // lineto
       // moveto (start of a new subpath)
       // lineto (a line segment)
       // lineto
       //
       // If this argument is set to false each subpath is drawn
       // individually which might not necessarily represent
       // the original drawing.
       true,   // backend supports curves
       true,   // backend supports elements which are filled and have edges
       true,   // backend supports text
       DriverDescription::png,         // support for PNG images
       DriverDescription::noopen,      // we create the output file ourself
       true,   // if format supports multiple pages in one file
       false  // clipping
       );
