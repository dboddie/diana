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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qtUtility.h"

#include "diCommandParser.h"
#include "diUtilities.h"

#include "diImageGallery.h"
#include "diLinetype.h"

#include <puTools/miStringFunctions.h>

#include <QApplication>
#include <qcombobox.h>
#include <QListWidget>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qpixmap.h>
#include <QPainter>
#include <qimage.h>
#include <qbrush.h>
#include <QIcon>

#define MILOGGER_CATEGORY "diana.Utility"
#include <miLogger/miLogging.h>

using namespace std;

int getIndex(const std::vector<std::string>& vstr, const std::string& def_str)
{
  for (unsigned int k=0; k<vstr.size(); k++) {
    if (def_str == vstr[k]) {
      return k;
    }
  }
  return -1;
}


int getIndex(const std::vector<Colour::ColourInfo>& cInfo, const std::string& def_str)
{
  for (unsigned int k=0; k<cInfo.size(); k++) {
    if (def_str == cInfo[k].name) {
      return k;
    }
  }
  return -1;
}

static std::string findOption(CommandParser& cp,
    std::vector<ParsedCommand>& vpcopt, const std::string& what)
{
  int nc = cp.findKey(vpcopt, what);
  if (nc < 0)
    return std::string();
  const std::string c = miutil::to_lower(vpcopt[nc].allValue);
  if (c == "off" || c == "av")
    return std::string();
  return c;
}

QPixmap createPixmapForStyle(const std::string& options)
{
  QPixmap pixmap;

  CommandParser cp;
  cp.addKey(PlotOptions::key_colour,         "",0,CommandParser::cmdString);
  cp.addKey(PlotOptions::key_palettecolours, "",0,CommandParser::cmdString);
  std::vector<ParsedCommand> vpcopt = cp.parse(options);

  { const std::string c = findOption(cp, vpcopt, PlotOptions::key_palettecolours);
    if (!c.empty())
      pixmap = pixmapForColourShading(ColourShading::getColourShading(c));
  }
  { const std::string c = findOption(cp, vpcopt, PlotOptions::key_colour);
    if (!c.empty()) {
      const int S=20, N=4;

      bool hasPalette = true;
      if (pixmap.isNull()) {
        hasPalette = false;
        pixmap = QPixmap(S, S);
        pixmap.fill(Qt::white);
      }

      const Colour col(c);
      QPen pen(QColor(col.R(), col.G(), col.B()));

      QPainter qp;
      qp.begin(&pixmap);

      if (!hasPalette) {
        // draw arcs similar to felt.xpm but with the chosen colour
        qp.setPen(pen);
        for (int j=1; j<=N; j+=1) {
          const int s = j*S/N;
          qp.drawArc(-s, -s, 2*s, 2*s, 0, 360*16);
        }
      }

      // draw rectangle in line colour
      pen.setWidth(2);
      qp.setPen(pen);
      qp.drawRect(pixmap.rect().adjusted(1,1,-1,-1));

      qp.end();
    }
  }

  return pixmap;
}

/*********************************************/
QLabel* TitleLabel(const QString& name, QWidget* parent)
{
  QLabel* label= new QLabel(name, parent);

  QPalette pal(label->palette());
  pal.setColor(QPalette::WindowText, QColor(0,0,128));
  label->setPalette(pal);

  return label;
}


/*********************************************/
QPushButton* SmallPushButton(const QString& name, QWidget* parent)
{
  QPushButton* b = new QPushButton( name, parent);

  QString qstr=name;
  int height = int(b->fontMetrics().height()*1.4);
  int width  = int(b->fontMetrics().width(qstr)+ height*0.5);
  b->setMaximumSize( width, height );

  return b;
}


/*********************************************/
QPushButton* NormalPushButton(const QString& name, QWidget* parent)
{
  return new QPushButton(name, parent);
}


/*********************************************/
QPushButton* PixmapButton(const QPixmap& pixmap, QWidget* parent, int deltaWidth, int deltaHeight)
{
  QPushButton* b = new QPushButton( parent );

  b->setIcon(QIcon(pixmap));

  int width  = pixmap.width()  + deltaWidth;
  int height = pixmap.height() + deltaHeight;

  b->setMinimumSize( width, height );
  b->setMaximumSize( width, height );

  return b;
}



/*********************************************/
QComboBox* ComboBox( QWidget* parent, const std::vector<std::string>& vstr, bool Enabled, int defItem)
{
  QComboBox* box = new QComboBox( parent );

  const int nr_box = vstr.size();
  for(int i=0; i<nr_box; i++)
    box->addItem(QString::fromStdString(vstr[i]));

  box->setEnabled( Enabled );
  box->setCurrentIndex(defItem);

  return box;
}


/*********************************************/
QComboBox* ComboBox(QWidget* parent, QColor* pixcolor, int nr_colors, bool Enabled, int defItem)
{
  QComboBox* box = new QComboBox(parent);
  for(int t=0; t<nr_colors; t++) {
    QPixmap pmap(20, 20);
    pmap.fill(pixcolor[t]);
    box->addItem(pmap, "");
  }

  box->setEnabled( Enabled );
  box->setCurrentIndex(defItem);

  return box;
}

/*********************************************/
QComboBox* ColourBox( QWidget* parent, bool Enabled, int defItem, const std::string& firstItem, bool name)
{
  vector<Colour::ColourInfo> cInfo = Colour::getColourInfo();
  return ColourBox(parent, cInfo, Enabled, defItem, firstItem, name);
}

static void ExpandColourBox(QComboBox* box, const QColor& pixcolor, const QString& name)
{
  QPixmap pmap(20, 20);
  pmap.fill(pixcolor);
  QIcon qicon(pmap);
  box->addItem(qicon, name, pixcolor);
}

void installColours(QComboBox* box, const vector<Colour::ColourInfo>& cInfo, bool name)
{
  const int nr_colors= cInfo.size();
  for (int t=0; t<nr_colors; t++) {
    const QColor pixcolor(cInfo[t].rgb[0],cInfo[t].rgb[1],cInfo[t].rgb[2] );
    ExpandColourBox(box, pixcolor, name ? QString::fromStdString(cInfo[t].name) : QString());
  }
}

QComboBox* ColourBox(QWidget* parent, const vector<Colour::ColourInfo>& cInfo,
    bool Enabled, int defItem, const std::string& firstItem, bool name)
{
  QComboBox* box = new QComboBox(parent);
  if (not firstItem.empty())
    box->addItem(QString::fromStdString(firstItem));
  installColours(box, cInfo, name);
  box->setEnabled( Enabled );
  box->setCurrentIndex(defItem);
  return box;
}

void ExpandColourBox(QComboBox* box, const Colour& col)
{
  const QColor pixcolor(col.R(),col.G(),col.B() );
  ExpandColourBox(box, pixcolor, QString::fromStdString(col.Name()));
}

/*********************************************/

QPixmap pixmapForColourShading(const std::vector<Colour>& colour)
{
  const int nr_colours = colour.size();
  if (nr_colours == 0)
    return QPixmap();

  int maxwidth=20;
  int step = nr_colours/maxwidth+1;
  int factor = maxwidth/(nr_colours/step);
  int width = (nr_colours/step) * factor;
  QPixmap pmap(width, 20);

  QPainter qp;
  qp.begin(&pmap);
  for( int j=0; j<nr_colours; j+=step ){
    QColor pixcolor=QColor(colour[j].R(), colour[j].G(), colour[j].B() );
    qp.fillRect(j*factor, 0, factor, 20, pixcolor);
  }
  qp.end();
  return pmap;
}

void installPalette(QComboBox* box, const std::vector<ColourShading::ColourShadingInfo>& csInfo, bool name)
{
  const int nr_palettes= csInfo.size();
  for (int i=0; i<nr_palettes; i++) {
    const int nr_colours = csInfo[i].colour.size();
    if (nr_colours == 0)
      continue;
    QIcon qicon(pixmapForColourShading(csInfo[i].colour));
    QString qs;
    if(name)
      qs = QString::fromStdString(csInfo[i].name);
    box->addItem(qicon, qs);
  }
}

QComboBox* PaletteBox( QWidget* parent, const vector<ColourShading::ColourShadingInfo>& csInfo,
    bool Enabled, int defItem, const std::string& firstItem, bool name)
{
  QComboBox* box = new QComboBox( parent );
  if (not firstItem.empty())
    box->addItem(QString::fromStdString(firstItem));
  installPalette(box, csInfo, name);
  box->setEnabled(Enabled);
  box->setCurrentIndex(defItem);
  return box;
}

void ExpandPaletteBox( QComboBox* box, const ColourShading& palette )
{
  vector<Colour> colours = palette.getColourShading();

  int nr_colours = colours.size();
  if ( nr_colours == 0 )
    return;
  int maxwidth=20;
  int step = nr_colours/maxwidth+1;
  int factor = maxwidth/(nr_colours/step);
  int width = (nr_colours/step) * factor;
  QPixmap* pmap = new QPixmap( width, 20 );
  QPainter qp;
  qp.begin( pmap );
  for( int j=0; j<nr_colours; j+=step ){
    QColor pixcolor=QColor(colours[j].R(),
        colours[j].G(),
        colours[j].B() );

    qp.fillRect( j*factor,0,factor,20,pixcolor);
  }

  qp.end();

  QIcon qicon( *pmap );
  QString qs = QString::fromStdString(palette.Name());
  box->addItem(qicon,qs);
  delete pmap;
  pmap=0;
}

/*********************************************/
QComboBox* PatternBox(QWidget* parent, const vector<Pattern::PatternInfo>& patternInfo,
    bool Enabled, int defItem, const std::string& firstItem, bool name)
{
  QComboBox* box = new QComboBox( parent );

  ImageGallery ig;

  if (not firstItem.empty())
    box->addItem(QString::fromStdString(firstItem));

  int nr_patterns= patternInfo.size();
  for( int i=0; i<nr_patterns; i++ ){
    int index = patternInfo[i].pattern.size()-1;
    if(index<0) continue;
    std::string filename = ig.getFilename(patternInfo[i].pattern[index],true);
    QIcon qicon(QString::fromStdString(filename));
    QString qs;
    if (name)
      qs = QString::fromStdString(patternInfo[i].name);
    box->addItem(qicon,qs);
  }

  box->setEnabled( Enabled );

  box->setCurrentIndex(defItem);

  return box;
}

static ushort lineStipplePattern(const QString &pattern)
{
  ushort p = 0;
  if (pattern.size() == 16) {
    for (int i = 0; i < 16; ++i)
      if (!pattern.at(i).isSpace())
        p |= (1 << i);
  }
  return p;
}

/*********************************************/
void installLinetypes(QComboBox* box)
{
  vector<std::string> slinetypes = Linetype::getLinetypeInfo();
  const int nr_linetypes= slinetypes.size();
  for (int i=0; i<nr_linetypes; i++) {
    size_t k1= slinetypes[i].find_first_of('[',0);
    size_t k2= slinetypes[i].find_first_of(']',0);
    std::string pattern = "- - - - - - - - ";
    if (k2-k1-1 >= 16)
      pattern = slinetypes[i].substr(k1+1,16);
    std::auto_ptr<QPixmap> pmapLinetype(linePixmap(pattern, 3));
    box->addItem(*pmapLinetype, "", lineStipplePattern(QString::fromStdString(pattern)));
  }
}

QComboBox* LinetypeBox( QWidget* parent, bool Enabled, int defItem)
{
  QComboBox* box = new QComboBox(parent);
  installLinetypes(box);
  box->setEnabled( Enabled );
  return box;
}

/*********************************************/
void installLinewidths(QComboBox* box, int nr_linewidths)
{
  for (int i=0; i < nr_linewidths; i++) {
    std::auto_ptr<QPixmap> pmapLinewidth(linePixmap("x", i+1));
    box->addItem(*pmapLinewidth, QString("  %1").arg(i+1), i + 1);
  }
}

QComboBox* LinewidthBox(QWidget* parent, bool Enabled, int nr_linewidths, int defItem)
{
  QComboBox* box = new QComboBox( parent );
  installLinewidths(box, nr_linewidths);
  box->setEnabled(Enabled);
  return box;
}

/*********************************************/
void ExpandLinewidthBox(QComboBox* box, int new_nr_linewidths)
{
  const int current_nr_linewidths = box->count();
  for (int i=current_nr_linewidths; i < new_nr_linewidths; i++) {
    std::auto_ptr<QPixmap> pmapLinewidth(linePixmap("x", i+1));
    QString label = QString("  %1").arg(i+1);
    box->addItem(*pmapLinewidth, label, i + 1);
  }
}

/*********************************************/
QComboBox* PixmapBox(QWidget* parent, std::vector<std::string>& markerName)
{
  /* Image support in Qt
BMP Windows Bitmap Read/write 
GIF Graphic Interchange Format (optional) Read 
JPG Joint Photographic Experts Group Read/write 
JPEG Joint Photographic Experts Group Read/write 
PNG Portable Network Graphics Read/write 
PBM Portable Bitmap Read 
PGM Portable Graymap Read 
PPM Portable Pixmap Read/write 
XBM X11 Bitmap Read/write 
XPM X11 Pixmap Read/write
   */

  QComboBox* box = new QComboBox( parent );

  ImageGallery ig;

  vector<std::string> name;
  ig.ImageNames(name,ImageGallery::marker);

  int n=name.size();
  for( int i=0;i<n; i++){
    std::string filename = ig.getFilename(name[i]);
    markerName.push_back(name[i]);

    std::string format;
    // sometimes Qt doesnt understand the format
    if(miutil::contains(filename, ".xpm"))
      format = "XPM";
    else if(miutil::contains(filename, ".png"))
      format = "PNG";
    else if(miutil::contains(filename, ".jpg"))
      format = "JPG";
    else if(miutil::contains(filename, ".jpeg"))
      format = "JPEG";
    else if(miutil::contains(filename, ".gif"))
      format = "GIF";
    else if(miutil::contains(filename, ".pbm"))
      format = "PBM";
    else if(miutil::contains(filename, ".pgm"))
      format = "PGM";
    else if(miutil::contains(filename, ".ppm"))
      format = "PPM";
    else if(miutil::contains(filename, ".xbm"))
      format = "XBM";
    else if(miutil::contains(filename, ".bmp"))
      format = "BMP";
    QImage image;
    if (!format.empty())
      image.load(filename.c_str(),format.c_str());
    else
      image.load(filename.c_str(),NULL);
    if (image.isNull())
    {
      METLIBS_LOG_WARN("PixmapBox: problem loading image: " << filename);
      continue;
    }
    QPixmap p = QPixmap::fromImage(image);
    if (p.isNull())
    {
      METLIBS_LOG_WARN("PixmapBox: problem converting from QImage to QPixmap" << filename);
      continue;
    }
    box->addItem (p, "" );
  }

  return box;
}

/*********************************************/
QLCDNumber* LCDNumber( uint numDigits, QWidget * parent)
{
  QLCDNumber* lcdnum = new QLCDNumber( numDigits, parent );
  lcdnum->setSegmentStyle ( QLCDNumber::Flat );
  //   lcdnum->setMinimumSize( lcdnum->sizeHint() );
  //   lcdnum->setMaximumSize( lcdnum->sizeHint() );
  return lcdnum;
}


/*********************************************/
QSlider* Slider( int minValue, int maxValue, int pageStep, int value,
    Qt::Orientation orient, QWidget* parent, int width )
{
  QSlider* slider = new QSlider(orient, parent);
  slider->setMinimum(minValue);
  slider->setMaximum(maxValue);
  slider->setSingleStep(pageStep);
  slider->setValue(value);
  slider->setMinimumSize( slider->sizeHint() );
  slider->setMaximumWidth( width );
  return slider;
}

/*********************************************/
QSlider* Slider(int minValue, int maxValue, int pageStep, int value,
    Qt::Orientation orient, QWidget* parent)
{
  QSlider* slider = new QSlider(orient, parent);
  slider->setMinimum(minValue);
  slider->setMaximum(maxValue);
  slider->setSingleStep(pageStep);
  slider->setValue(value);
  slider->setMinimumSize( slider->sizeHint() );
  slider->setMaximumSize( slider->sizeHint() );
  return slider;
}


/*********************************************/
void listWidget(QListWidget* listwidget, const std::vector<std::string>& vstr, int defItem)
{
  if( listwidget->count() )
    listwidget->clear();

  for( unsigned int i=0; i<vstr.size(); i++ ){
    listwidget->addItem( QString::fromStdString(vstr[i]) );
  }

  if( defItem> -1 ) listwidget->setCurrentRow( defItem );
}

/*********************************************/
QPixmap* linePixmap(const std::string& pattern, int linewidth)
{
  // make a 32x20 pixmap of a linepattern of length 16 (where ' ' is empty)

  std::string xpmEmpty= "################################";
  std::string xpmLine=  "................................";
  int i;
  int lw= linewidth;
  if (lw<1)  lw=1;
  if (lw>20) lw=20;

  if (pattern.length()>=16) {
    for (i=0; i<16; i++)
      if (pattern[i]==' ') xpmLine[16+i]= xpmLine[i]= '#';
  }

  int l1= 10 - lw/2;
  int l2= l1 + lw;

  const char** xpmData= new const char*[3+20];
  xpmData[0]= "32 20 2 1";
  xpmData[1]= ". c #000000";
  xpmData[2]= "# c None";

  for (i=0;  i<l1; i++) xpmData[3+i]= xpmEmpty.c_str();
  for (i=l1; i<l2; i++) xpmData[3+i]= xpmLine.c_str();
  for (i=l2; i<20; i++) xpmData[3+i]= xpmEmpty.c_str();

  QPixmap* pmap= new QPixmap(xpmData);

  delete[] xpmData;

  return pmap;
}

namespace diutil {

OverrideCursor::OverrideCursor(const QCursor& cursor)
{
  QApplication::setOverrideCursor(cursor);
}

OverrideCursor::~OverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

std::vector<std::string> numberList(QComboBox* cBox, float number, const float* enormal, bool onoff)
{
  std::vector<std::string> numbers = numberList(number, enormal);

  int current = (numbers.size() - 1)/2;
  if (onoff) {
    numbers.insert(numbers.begin(), "off");
    current += 1;
  }
  cBox->clear();
  for (size_t i=0; i<numbers.size(); ++i)
    cBox->addItem(QString::fromStdString(numbers[i]));
  cBox->setCurrentIndex(current);

  return numbers;
}

} // namespace diutil
