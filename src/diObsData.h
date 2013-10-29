/*
  Diana - A Free Meteorological Visualisation Tool

  $Id$

  Copyright (C) 2006 met.no

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
#ifndef diObsData_h
#define diObsData_h

#include <diColour.h>
#include <puTools/miTime.h>
#include <puTools/miString.h>

/**

  \brief Observation data
  
*/
class ObsData
{
public:
  //desc
  miutil::miString dataType;
  miutil::miString id;
  miutil::miString name;
  float xpos;
  float ypos;
  int zone;
  miutil::miTime obsTime;

  //metar
  miutil::miString metarId;
  bool CAVOK;              
  std::vector<miutil::miString> REww;   ///< Recent weather
  std::vector<miutil::miString> ww;     ///< Significant weather
  std::vector<miutil::miString> cloud;  ///< Clouds
  miutil::miString appendix;       ///< For whatever remains
  
  std::map<miutil::miString,float> fdata;
  std::map<miutil::miString,miutil::miString> stringdata;

  //Hqc  
  std::map<miutil::miString,miutil::miString> flag; 
  std::map<miutil::miString,Colour> flagColour;
};

#endif
