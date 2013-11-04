
#include "diVcrossAxis.h"

#include "diVcrossUtil.h"

#define MILOGGER_CATEGORY "diana.VcrossAxis"
#include <miLogger/miLogging.h>

namespace VcrossPlotDetail {

bool Axis::legalPaint(float p) const
{
  return VcrossUtil::value_between(p, paintMin, paintMax);
}

bool Axis::legalValue(float v) const
{
  return VcrossUtil::value_between(v, valueMin, valueMax);
}

bool Axis::legalData(float d) const
{
  return VcrossUtil::value_between(d, dataMin, dataMax);
}

float Axis::function(float x) const
{
  switch (type) {
  case LINEAR:
    return x;
  case EXNER:
    return VcrossUtil::exnerFunction(x);
  }
  return x; // not reached
}

float Axis::functionInverse(float x) const
{
  switch (type) {
  case LINEAR:
    return x;
  case EXNER:
    return VcrossUtil::exnerFunctionInverse(x);
  }
  return x; // not reached
}

void Axis::calculateScale()
{
  //METLIBS_LOG_SCOPE();
  const float dp = (paintMax - paintMin),
      dv = (valueMax-valueMin),
      fdv = function(dv);
  scale = dp / fdv;
  //METLIBS_LOG_DEBUG(LOGVAL(dp) << LOGVAL(dv) << LOGVAL(fdv) << LOGVAL(scale) << LOGVAL(valueMin) << LOGVAL(valueMax));
}


float Axis::value2paint(float v, bool check) const
{
  //METLIBS_LOG_SCOPE();
  if (check and not legalValue(v))
    return -1e35;
  const float vv = (v-valueMin),
      fvv = function(vv),
      sfvv = scale * fvv;
  //METLIBS_LOG_DEBUG(LOGVAL(vv) << LOGVAL(fvv) << LOGVAL(sfvv) << LOGVAL(scale));
  return sfvv + paintMin;
}

float Axis::paint2value(float p, bool check) const
{
  METLIBS_LOG_SCOPE();
  if (check and not legalPaint(p))
    return -1e35;

  const float pp = (p-paintMin),
      spp = pp/scale,
      ispp = functionInverse(spp),
      v = ispp + valueMin;
  METLIBS_LOG_DEBUG(LOGVAL(p) << LOGVAL(pp) << LOGVAL(spp) << LOGVAL(ispp) << LOGVAL(v));
  return v;

  return functionInverse((p-paintMin)/scale) + valueMin;
}

bool Axis::zoomIn(float paint0, float paint1)
{
  if ((horizontal and paint0 > paint1) or (not horizontal and paint0 < paint1))
    std::swap(paint0, paint1);
  const float v0 = paint2value(paint0), v1 = paint2value(paint1);
  if (legalData(v0) and legalData(v1)) {
    setValueRange(v0, v1);
    return true;
  } else {
    return false;
  }
}

bool Axis::zoomOut()
{
  const float delta = (valueMax - valueMin) * 0.3 * 0.5;
  const float vmin = VcrossUtil::constrain_value(valueMin - delta, dataMin, dataMax);
  const float vmax = VcrossUtil::constrain_value(valueMax + delta, dataMin, dataMax);
  if (vmin != valueMin or vmax != valueMax) {
    setValueRange(vmin, vmax);
    return true;
  } else {
    return false;
  }
}

bool Axis::pan(float delta)
{
  if (delta == 0)
    return false;
  if (not horizontal)
    delta = -delta;

  float vMax = paint2value(paintMax + delta, false);
  float vMin = paint2value(paintMin + delta, false);
  if (legalData(vMax) and legalData(vMin)) {
    setValueRange(vMin, vMax);
    return true;
  } else {
    return false;
  }
}

} // namespace VcrossPlotDetail
