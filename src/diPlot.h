/*
  Diana - A Free Meteorological Visualisation Tool

  Copyright (C) 2006-2013 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no

  This file is part of Diana

  Diana is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Diana is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Diana; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef diPlot_h
#define diPlot_h

#include "diColour.h"
#include "diPlotOptions.h"
#include "diPrintOptions.h"

#include <diField/diArea.h>
#include <diField/diGridConverter.h>
#include <puTools/miTime.h>

#include <GL/gl.h>

#include <vector>

class GLPfile;
class FontManager;


/**
   StaticPlot keeps all previously static data shared by the various plotting classes.
   - postscript generation initiated here
*/
class StaticPlot {
private:
  Area area;          // Projection and size of current grid
  Area requestedarea; // Projection and size of requested grid
  Rectangle maprect;  // Size of map plot area
  Rectangle fullrect; // Size of full plot area
  miutil::miTime ctime;       // current time
  float pwidth;       // physical size of plotarea
  float pheight;      // --- " ---
  bool dirty;         // plotarea has changed
  int pressureLevel;          // current pressure level
  int oceandepth;       // current ocean depth
  std::string bgcolour;  // name of background colour
  Colour backgroundColour;   // background colour
  Colour backContrastColour; // suitable contrast colour
  float gcd;          // great circle distance
  bool panning;       // panning in progress
  FontManager* fp;    // master fontpack
  printerManager printman;   // printer manager

public:
  static GridConverter gc;   // gridconverter class
  GLPfile* psoutput;  // PostScript module
  bool hardcopy;      // producing postscript

  // FIXME xyLimit and xyPart only used by obsolete config in MapPlot::prepare
  std::vector<float> xyLimit; // MAP ... xyLimit=x1,x2,y1,y2
  std::vector<float> xyPart;  // MAP ... xyPart=x1%,x2%,y1%,y2%

public:
  StaticPlot();
  ~StaticPlot();

  void psAddImage(const GLvoid*,GLint,GLint,GLint, // pixels,size,nx,ny
		  GLfloat,GLfloat,GLfloat,GLfloat, // x,y,sx,sy
		  GLint,GLint,GLint,GLint,   // start,stop
		  GLenum,GLenum);  // format, type

  /// init fonts
  void initFontManager();

  /// kill current FontManager, start new and init
  void restartFontManager();

  /// return current area on map
  const Area& getMapArea()
    { return area; }

  /// set area, possibly trying to keep the current physical area
  bool setMapArea(const Area&, bool keepcurrentarea);

  /// with a new projection: find the best matching physical area with the current one
  Area findBestMatch(const Area&);

  /// this is the area we really want
  void setRequestedarea(const Area &a)
    { requestedarea = a; }

  /// this is the area we really wanted
  const Area& getRequestedarea()
    { return requestedarea; }

  /// this is the full size of the plot in the current projection
  const Rectangle& getPlotSize()
    { return fullrect; }

  /// set the  full size of the plot in the current projection
  void setPlotSize(const Rectangle& r);

  /// this is size of the data grid
  const Rectangle& getMapSize()
    { return maprect; }

  /// set the size of the data grid
  void setMapSize(const Rectangle& r);

  /// set the physical size of the map in pixels
  void setPhysSize(float w, float h);

  /// this is  the physical size of the map in pixels
  void getPhysSize(float& w, float& h);

  float getPhysWidth()
    { return pwidth; }

  float getPhysHeight()
    { return pheight; }

  /// set the current data time
  void setTime(const miutil::miTime& t)
    { ctime= t; }

  /// return the current data time
  const miutil::miTime& getTime()
    { return ctime; }

  /// set current pressure level
  void setPressureLevel(int l)
    { pressureLevel= l; }

  /// this is the current pressure level
  int getPressureLevel()
    { return pressureLevel; }

  /// set current ocean depth
  void setOceanDepth(int depth)
    { oceandepth= depth; }

  /// this is the current ocean depth
  int getOceanDepth()
    {return oceandepth;}

  /// set name of background colour
  void setBgColour(const std::string& cn)
    { bgcolour= cn; }

  /// return the name of the current background colour
  const std::string& getBgColour()
    { return bgcolour; }

  /// set background colour
  void setBackgroundColour(const Colour& c)
    { backgroundColour= c; }

  /// set colour with good contrast to background
  void setBackContrastColour(const Colour& c)
    { backContrastColour= c; }

  /// return the current background colour
  const Colour& getBackgroundColour()
    { return backgroundColour; }

  /// return colour with good contrast to background
  const Colour& getBackContrastColour()
    { return backContrastColour; }

  /// return pointer to the FontManager
  FontManager* getFontPack() {return fp;}

  /// mark this as 'redraw needed'
  void setDirty(bool dirty=true);

  /// is redraw needed
  bool getDirty()
    { return dirty; }

  /// clear clipping variables
  void xyClear();

  // hardcopy routines
  /// start postscript output
  bool startPSoutput(const printOptions& po);
  /// add a stencil as x,y arrays (postscript only)
  void addHCStencil(int size, const float* x, const float* y);
  /// add a scissor in GL coordinates (postscript only)
  void addHCScissor(double x0, double y0, // GL scissor
		    double  w, double  h);
  /// add a scissor in pixel coordinates (postscript only)
  void addHCScissor(int x0, int y0, // Pixel scissor
		    int  w, int  h);
  /// remove all clipping (postscript only)
  void removeHCClipping();
  /// for postscript output - resample state vectors
  void UpdateOutput();
  /// start new page in postscript
  bool startPSnewpage();
  /// for postscript output - reset state vectors
  void resetPage();
  /// end postscript output
  bool endPSoutput();

  /// set great circle distance
  void setGcd(float dist);

  float getGcd()
    { return gcd; }

  /// toggle panning
  void panPlot(bool pan);

  bool isPanning()
    { return panning; }

  /// convert from geo to xy using getMapArea()
  bool geo2xy(int n, float* x, float* y);
};

/**
   \brief Ancestor of all map plotting classes
*/

class Plot {
public:
  Plot();

  // "implemented" below
  virtual ~Plot() = 0;

  // Equality operator
  bool operator==(const Plot &rhs) const;

  StaticPlot* getStaticPlot() const;

  /// plot
  virtual bool plot(){return false; }
  /// plot for specified layer
  virtual bool plot(int zorder){return false; }

  /// enable this plot object
  void setEnabled(bool enable=true);

  /// is this plot object enabled
  bool isEnabled() const
    { return enabled; }

  /// set the plot info string
  void setPlotInfo(const std::string& pin);

  /// return n elements of the current plot info string
  std::string getPlotInfo(int n=0) const;

  /// return the elements given
  std::string getPlotInfo(const std::string& str) const;

  /// return true if right plot string
  bool plotInfoOK(const std::string& pin) const
    { return (pinfo == pin); }

  /// return the current PlotOptions
  const PlotOptions& getPlotOptions() const
    { return poptions; }

  /// set colour mode (rgb or color-index)
  void setColourMode(bool isrgb=true);

  /// return current colourmode
  bool getColourMode() const
    { return rgbmode; }

  /// set name of this plot object
  void setPlotName(const std::string& name)
    { plotname= name; }

  /// return name of this plot object
  std::string getPlotName() const
    { return plotname; }

protected:
  std::string pinfo;            // plotinfo
  PlotOptions poptions;      // plotoptions

private:
  bool enabled;              // plot enabled
  bool rgbmode;               // rgb or colour-index mode
  std::string plotname;       // name of plot
};

inline Plot::~Plot()
{
}

#endif
