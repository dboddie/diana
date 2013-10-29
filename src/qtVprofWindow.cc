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

#include <qapplication.h>
#include <QFileDialog>
#include <QToolBar>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qfont.h>
#include <qmotifstyle.h>
#include <QPrintDialog>
#include <QPrinter>
#include <QPixmap>

#define MILOGGER_CATEGORY "diana.VprofWindow"
#include <miLogger/miLogging.h>

#include "qtVprofWindow.h"
#include "qtToggleButton.h"
#include "qtUtility.h"
#include "diStationPlot.h"
#include "qtVprofWidget.h"
#include "qtVprofModelDialog.h"
#include "qtVprofSetupDialog.h"
#include "diVprofManager.h"
#include "qtPrintManager.h"
#include "forover.xpm"
#include "bakover.xpm"


VprofWindow::VprofWindow(Controller *co)
: QMainWindow( 0)
{

  vprofm = new VprofManager(co);

  setWindowTitle( tr("Diana Vertical Profiles") );

#if !defined(USE_PAINTGL)
  QGLFormat fmt;
  fmt.setOverlay(false);
  fmt.setDoubleBuffer(true);
  fmt.setDirectRendering(false);
#endif
  //central widget
#if !defined(USE_PAINTGL)
  vprofw= new VprofWidget(vprofm, fmt, this);
#else
  vprofw= new VprofWidget(vprofm, this);
#endif
  setCentralWidget(vprofw);
  connect(vprofw, SIGNAL(timeChanged(int)),SLOT(timeChangedSlot(int)));
  connect(vprofw, SIGNAL(stationChanged(int)),SLOT(stationChangedSlot(int)));


  // tool bar and buttons
  vpToolbar = new QToolBar(this);
  addToolBar(Qt::TopToolBarArea,vpToolbar);

  // tool bar for selecting time and station
  tsToolbar = new QToolBar(this);
  addToolBar(Qt::TopToolBarArea,tsToolbar);

  // button for modeldialog-starts new dialog
  modelButton = new ToggleButton(this, tr("Model"));
  connect( modelButton, SIGNAL( toggled(bool)), SLOT( modelClicked( bool) ));

  //button for setup - starts setupdialog
  setupButton = new ToggleButton(this, tr("Settings"));
  connect( setupButton, SIGNAL( toggled(bool)), SLOT( setupClicked( bool) ));

  //button for update
  QPushButton * updateButton = NormalPushButton( tr("Refresh"),this);
  connect( updateButton, SIGNAL(clicked()), SLOT(updateClicked()) );

  //button to print - starts print dialog
  QPushButton* printButton = NormalPushButton(tr("Print"),this);
  connect( printButton, SIGNAL(clicked()), SLOT( printClicked() ));

  //button to save - starts save dialog
  QPushButton* saveButton = NormalPushButton(tr("Save"),this);
  connect( saveButton, SIGNAL(clicked()), SLOT( saveClicked() ));

  //button for quit
  QPushButton * quitButton = NormalPushButton(tr("Quit"),this);
  connect( quitButton, SIGNAL(clicked()), SLOT(quitClicked()) );

  //button for help - pushbutton
  QPushButton * helpButton = NormalPushButton(tr("Help"),this);
  connect( helpButton, SIGNAL(clicked()), SLOT(helpClicked()) );


  //combobox to select station
  QPushButton *leftStationButton= new QPushButton(QPixmap(bakover_xpm),"",this);
  connect(leftStationButton, SIGNAL(clicked()), SLOT(leftStationClicked()) );
  leftStationButton->setAutoRepeat(true);

  vector<miutil::miString> stations;
  stations.push_back("                         ");
  stationBox = ComboBox( this, stations, true, 0);
  connect( stationBox, SIGNAL( activated(int) ), SLOT( stationBoxActivated(int) ) );

  QPushButton *rightStationButton= new QPushButton(QPixmap(forward_xpm),"",this);
  connect(rightStationButton, SIGNAL(clicked()), SLOT(rightStationClicked()) );
  rightStationButton->setAutoRepeat(true);

  QPushButton *leftTimeButton= new QPushButton(QPixmap(bakover_xpm),"",this);
  connect(leftTimeButton, SIGNAL(clicked()), SLOT(leftTimeClicked()) );
  leftTimeButton->setAutoRepeat(true);

  //combobox to select time
  vector<miutil::miString> times;
  times.push_back("2002-01-01 00");
  timeBox = ComboBox( this, times, true, 0);
  connect( timeBox, SIGNAL( activated(int) ), SLOT( timeBoxActivated(int) ) );

  QPushButton *rightTimeButton= new QPushButton(QPixmap(forward_xpm),"",this);
  connect(rightTimeButton, SIGNAL(clicked()), SLOT(rightTimeClicked()) );
  rightTimeButton->setAutoRepeat(true);

  vpToolbar->addWidget(modelButton);
  vpToolbar->addWidget(setupButton);
  vpToolbar->addWidget(updateButton);
  vpToolbar->addWidget(printButton);
  vpToolbar->addWidget(saveButton);
  vpToolbar->addWidget(quitButton);
  vpToolbar->addWidget(helpButton);

  insertToolBarBreak(tsToolbar);

  tsToolbar->addWidget(leftStationButton);
  tsToolbar->addWidget(stationBox);
  tsToolbar->addWidget(rightStationButton);
  tsToolbar->addWidget(leftTimeButton);
  tsToolbar->addWidget(timeBox);
  tsToolbar->addWidget(rightTimeButton);

  //connected dialogboxes

  vpModelDialog = new VprofModelDialog(this,vprofm);
  connect(vpModelDialog, SIGNAL(ModelApply()),SLOT(changeModel()));
  connect(vpModelDialog, SIGNAL(ModelHide()),SLOT(hideModel()));
  connect(vpModelDialog, SIGNAL(showsource(const std::string, const std::string)),
      SIGNAL(showsource(const std::string, const std::string)));


  vpSetupDialog = new VprofSetupDialog(this,vprofm);
  connect(vpSetupDialog, SIGNAL(SetupApply()),SLOT(changeSetup()));
  connect(vpSetupDialog, SIGNAL(SetupHide()),SLOT(hideSetup()));
  connect(vpSetupDialog, SIGNAL(showsource(const std::string, const std::string)),
      SIGNAL(showsource(const std::string, const std::string)));

  //initialize everything in startUp
  firstTime = true;
  active = false;
  mainWindowTime= miutil::miTime::nowTime();

#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::VprofWindow() finished");
#endif
}


/***************************************************************************/

void VprofWindow::modelClicked( bool on ){
  //called when the model button is clicked
  if( on ){
#ifdef DEBUGPRINT
    METLIBS_LOG_DEBUG("Model button clicked on");
#endif
    vpModelDialog->show();
  } else {
#ifdef DEBUGPRINT
    METLIBS_LOG_DEBUG("Model button clicked off");
#endif
    vpModelDialog->hide();
  }
}

/***************************************************************************/

void VprofWindow::leftStationClicked(){
  //called when the left Station button is clicked
  miutil::miString s= vprofm->setStation(-1);
  stationChangedSlot(-1);
  vprofw->updateGL();
}


/***************************************************************************/

void VprofWindow::rightStationClicked(){
  //called when the right Station button is clicked
  miutil::miString s= vprofm->setStation(+1);
  stationChangedSlot(+1);
  vprofw->updateGL();
}

/***************************************************************************/


void VprofWindow::leftTimeClicked(){
  //called when the left time button is clicked
  vprofm->setTime(-1);
  timeChangedSlot(-1);
  vprofw->updateGL();
}

/***************************************************************************/

void VprofWindow::rightTimeClicked(){
  //called when the right Station button is clicked
  vprofm->setTime(+1);
  timeChangedSlot(+1);
  vprofw->updateGL();
}


/***************************************************************************/

bool VprofWindow::timeChangedSlot(int diff){
  //called if signal timeChanged is emitted from graphics
  //window (qtVprofWidget)
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("timeChangedSlot(int) is called ");
#endif
  int index=timeBox->currentIndex();
  while(diff<0){
    if(--index < 0) {
      //set index to the last in the box !
      index=timeBox->count()-1;
    }
    timeBox->setCurrentIndex(index);
    diff++;
  }
  while(diff>0){
    if(++index > timeBox->count()-1) {
      //set index to the first in the box !
      index=0;
    }
    timeBox->setCurrentIndex(index);
    diff--;
  }
  miutil::miTime t = vprofm->getTime();
  miutil::miString tstring=t.isoTime(false,false);
  if (!timeBox->count()) return false;
  miutil::miString tbs=timeBox->currentText().toStdString();
  if (tbs!=tstring){
    //search timeList
    int n = timeBox->count();
    for (int i = 0; i<n;i++){
      if(tstring ==timeBox->itemText(i).toStdString()){
        timeBox->setCurrentIndex(i);
        tbs=timeBox->currentText().toStdString();
        break;
      }
    }
  }
  if (tbs!=tstring){
    METLIBS_LOG_WARN("WARNING! timeChangedSlot  time from vprofm ="
    << t    <<" not equal to timeBox text = " << tbs);
    METLIBS_LOG_WARN("You should search through timelist!");
    return false;
  }

  if (onlyObs) {
    //emit to main Window (updates stationPlot)
    emit modelChanged();
    //update combobox lists of stations and time
    updateStationBox();
    //get correct selection in comboboxes
    stationChangedSlot(0);
  }

  emit setTime("vprof",t);

  return true;
}


/***************************************************************************/

bool VprofWindow::stationChangedSlot(int diff){
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("stationChangedSlot(int) is called ");
#endif
  int index=stationBox->currentIndex();
  while(diff<0){
    if(--index < 0) {
      //set index to the last in the box !
      index=stationBox->count()-1;
    }
    stationBox->setCurrentIndex(index);
    diff++;
  }
  while(diff>0){
    if(++index > stationBox->count()-1) {
      //set index to the first in the box !
      index=0;
    }
    stationBox->setCurrentIndex(index);
    diff--;
  }
  //get current station
  miutil::miString s = vprofm->getStation();
  //if (!stationBox->count()) return false;
  //if no current station, use last station plotted
  if (s.empty()) s = vprofm->getLastStation();
  miutil::miString sbs=stationBox->currentText().toStdString();
  if (sbs!=s){
    int n = stationBox->count();
    for(int i = 0;i<n;i++){
      if (s==stationBox->itemText(i).toStdString()){
        stationBox->setCurrentIndex(i);
        sbs=miutil::miString(stationBox->currentText().toStdString());
        break;
      }
    }
  }
  QString sq = s.c_str();
  if (sbs==s) {
    emit stationChanged(sq); //name of current station (to mainWindow)
    return true;
  } else {
    //    METLIBS_LOG_WARN("WARNING! stationChangedSlot  station from vprofm ="
    // 	 << s    <<" not equal to stationBox text = " << sbs);
    //current or last station plotted is not in the list, insert it...
    stationBox->addItem(sq,0);
    stationBox->setCurrentIndex(0);
    return false;
  }
}


/***************************************************************************/

void VprofWindow::printClicked()
{
  QPrinter qprt;

  printerManager pman;
  miutil::miString command= pman.printCommand();

  fromPrintOption(qprt,priop);

  QPrintDialog printerDialog(&qprt, this);
  if (printerDialog.exec()) {
    if (!qprt.outputFileName().isNull()) {
      priop.fname= qprt.outputFileName().toStdString();
    } else {
      priop.fname= "prt_" + miutil::miTime::nowTime().isoTime() + ".ps";
      priop.fname= priop.fname.replace(' ','_');
    }

    // fill printOption from qprinter-selections
    toPrintOption(qprt, priop);

    // set printername
    if (qprt.outputFileName().isNull())
      priop.printer= qprt.printerName().toStdString();

    // start the postscript production
    QApplication::setOverrideCursor( Qt::WaitCursor );

    vprofm->startHardcopy(priop);
    vprofw->updateGL();
    vprofm->endHardcopy();
    vprofw->updateGL();

    // if output to printer: call appropriate command
    if (qprt.outputFileName().isNull()){
      priop.numcopies= qprt.numCopies();

      // expand command-variables
      pman.expandCommand(command, priop);

      int res = system(command.c_str());

      if (res != 0){
        METLIBS_LOG_WARN("Print command:" << command << " failed");
      }

    }
    QApplication::restoreOverrideCursor();
  }
}

/***************************************************************************/

void VprofWindow::saveClicked()
{
  static QString fname = "./"; // keep users preferred image-path for later
  QString s = QFileDialog::getSaveFileName(this,
      tr("Save plot as image"),
      fname,
      tr("Images (*.png *.xpm *.bmp *.eps);;All (*.*)"));


  if (!s.isNull()) {// got a filename
    fname= s;
    miutil::miString filename= s.toStdString();
    miutil::miString format= "PNG";
    int quality= -1; // default quality

    // find format
    if (filename.contains(".xpm") || filename.contains(".XPM"))
      format= "XPM";
    else if (filename.contains(".bmp") || filename.contains(".BMP"))
      format= "BMP";
    else if (filename.contains(".eps") || filename.contains(".epsf")){
      // make encapsulated postscript
      // NB: not screendump!
      makeEPS(filename);
      return;
    }

    // do the save
    vprofw->saveRasterImage(filename, format, quality);
  }
}


void VprofWindow::makeEPS(const miutil::miString& filename)
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  printOptions priop;
  priop.fname= filename;
  priop.colop= d_print::incolour;
  priop.orientation= d_print::ori_automatic;
  priop.pagesize= d_print::A4;
  priop.numcopies= 1;
  priop.usecustomsize= false;
  priop.fittopage= false;
  priop.drawbackground= true;
  priop.doEPS= true;

  vprofm->startHardcopy(priop);
  vprofw->updateGL();
  vprofm->endHardcopy();
  vprofw->updateGL();

  QApplication::restoreOverrideCursor();
}

/***************************************************************************/

void VprofWindow::setupClicked(bool on){
  //called when the setup button is clicked
  if( on ){
#ifdef DEBUGPRINT
    METLIBS_LOG_DEBUG("Setup button clicked on");
#endif
    vpSetupDialog->start();
    vpSetupDialog->show();

  } else {
#ifdef DEBUGPRINT
    METLIBS_LOG_DEBUG("Setup button clicked off");
#endif
    vpSetupDialog->hide();
  }
}


/***************************************************************************/

void VprofWindow::quitClicked(){
  //called when the quit button is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("quit clicked");
#endif
  //for now, only hide window, not really quit !
  vpToolbar->hide();
  tsToolbar->hide();
  modelButton->setChecked(false);
  setupButton->setChecked(false);
  active = false;
  emit VprofHide();
  vector<miutil::miTime> t;
  emit emitTimes("vprof",t);
}


/***************************************************************************/

void VprofWindow::hideClicked(){
  //called when the hide button in Vprofwindow is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("hide clicked");
#endif
}

/***************************************************************************/

void VprofWindow::updateClicked(){
  //called when the update button in Vprofwindow is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("update clicked");
#endif
  vprofm->updateObs();      // check obs.files
  miutil::miTime t= mainWindowTime; // use the main time (fields etc.)
  mainWindowTimeChanged(t);
}

/***************************************************************************/

void VprofWindow::helpClicked(){
  //called when the help button in Vprofwindow is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("help clicked");
#endif
  emit showsource("ug_verticalprofiles.html");
}


/***************************************************************************/

void VprofWindow::MenuOK(){
  //obsolete - nothing happens here
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::MenuOK()");
#endif
}


/***************************************************************************/

void VprofWindow::changeModel(){
  //called when the apply button from model dialog is clicked
  //... or field is changed ?
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::changeModel()");
#endif
  QApplication::setOverrideCursor( Qt::WaitCursor );
  vprofm->setModel();
  QApplication::restoreOverrideCursor();

  onlyObs= vprofm->onlyObsState();

  //emit to main Window (updates stationPlot)
  emit modelChanged();
  //update combobox lists of stations and time
  updateStationBox();
  updateTimeBox();
  //get correct selection in comboboxes
  stationChangedSlot(0);
  timeChangedSlot(0);
  vprofw->updateGL();
}


/***************************************************************************/

void VprofWindow::changeSetup(){
  //called when the apply from setup dialog is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::changeSetup()");
#endif
  vprofw->updateGL();
}



/***************************************************************************/

void VprofWindow::hideModel(){
  //called when the hide button (from model dialog) is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::hideModel()");
#endif
  vpModelDialog->hide();
  modelButton->setChecked(false);
}

/***************************************************************************/
void VprofWindow::hideSetup(){
  //called when the hide button (from setup dialog) is clicked
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::hideSetup()");
#endif
  vpSetupDialog->hide();
  setupButton->setChecked(false);
}

/***************************************************************************/

StationPlot* VprofWindow::getStations(){
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::getStations()");
#endif
  const vector <miutil::miString> stations = vprofm->getStationList();
  const vector <float> latitude = vprofm->getLatitudes();
  const vector <float> longitude = vprofm->getLongitudes();
  StationPlot* stationPlot = new StationPlot(stations,longitude,latitude);
  miutil::miString ann = vprofm->getAnnotationString();
  stationPlot->setStationPlotAnnotation(ann);

  // ADC set plotname (for StatusPlotButtons)
  stationPlot->setPlotName(ann);

  //the coordinates are defined here
#ifdef DEBUGPRINT
  //  for (int i = 0; i<n;i++){
  //     METLIBS_LOG_DEBUG("Station number " << i << " name = " << stations[i]
  // 	 << " latitude = " << latitude[i]
  // 	 << " longitude = " << longitude[i]);
  //   }
#endif

  return stationPlot;
}

/***************************************************************************/

void VprofWindow::updateStationBox(){
  //update list of stations in stationBox
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::updateStationBox");
#endif

  stationBox->clear();
  vector<miutil::miString> stations= vprofm->getStationList();

  std::sort(stations.begin(),stations.end());

  //add dummy to make stationBox wide enough
  stations.push_back("                        ");

  int n =stations.size();
  for (int i=0; i<n; i++){
    stationBox->addItem(QString(stations[i].c_str()));
  }
}


/***************************************************************************/

void VprofWindow::updateTimeBox(){
  //update list of times
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::updateTimeBox");
#endif

  timeBox->clear();
  vector<miutil::miTime> times= vprofm->getTimeList();

  int n =times.size();
  for (int i=0; i<n; i++){
    timeBox->addItem(QString(times[i].isoTime(false,false).c_str()));
  }

  emit emitTimes("vprof",times);
}

/***************************************************************************/

void VprofWindow::stationBoxActivated(int index){


  //vector<miutil::miString> stations= vprofm->getStationList();
  miutil::miString sbs=stationBox->currentText().toStdString();
  //if (index>=0 && index<stations.size()) {
  vprofm->setStation(sbs);
  vprofw->updateGL();
  QString sq = sbs.c_str();
  emit stationChanged(sq); //name of current station (to mainWindow)
  //}
}

/***************************************************************************/

void VprofWindow::timeBoxActivated(int index){

  vector<miutil::miTime> times= vprofm->getTimeList();

  if (index>=0 && index<int(times.size())) {
    vprofm->setTime(times[index]);

    if (onlyObs) {
      //emit to main Window (updates stationPlot)
      emit modelChanged();
      //update combobox lists of stations and time
      updateStationBox();
      //get correct selection in comboboxes
      stationChangedSlot(0);
    }

    vprofw->updateGL();
  }
}

/***************************************************************************/

bool VprofWindow::changeStation(const string& station)
{
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("VprofWindow::changeStation");
#endif
  vprofm->setStation(station); //HK ??? should check if station exists ?
  vprofw->updateGL();
  raise();
  if (stationChangedSlot(0))
    return true;
  else
    return false;
}


/***************************************************************************/

void VprofWindow::setFieldModels(const vector<string>& fieldmodels)
{
  vprofm->setFieldModels(fieldmodels);
  if (active)
    changeModel();
}

/***************************************************************************/

void VprofWindow::mainWindowTimeChanged(const miutil::miTime& t){

  // keep time for next "update" (in case not found now)
  mainWindowTime= t;

  if (!active) return;
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("vprofWindow::mainWindowTimeChanged called with time " << t);
#endif
  vprofm->mainWindowTimeChanged(t);
  if (onlyObs) {
    //emit to main Window (updates stationPlot)
    emit modelChanged();
    //update combobox lists of stations and time
    updateStationBox();
  }
  //get correct selection in comboboxes
  stationChangedSlot(0);
  timeChangedSlot(0);
  vprofw->updateGL();
}


/***************************************************************************/

void VprofWindow::startUp(const miutil::miTime& t){
#ifdef DEBUGPRINT
  METLIBS_LOG_DEBUG("vprofWindow::startUp called with time " << t);
#endif
  active = true;
  vpToolbar->show();
  tsToolbar->show();
  //do something first time we start Vertical profiles
  if (firstTime){
    vector<string> models;
    //define models for dialogs, comboboxes and stationplot
    vprofm->setSelectedModels(models,false,true,true,true);
    vpModelDialog->updateModelfileList();
    vpModelDialog->setSelection();
    firstTime=false;
    // show default diagram without any data
    vprofw->updateGL();
  }

  changeModel();
  mainWindowTimeChanged(t);
}

/***************************************************************************/

void VprofWindow::parseSetup()
{

  vprofm->parseSetup();
  vpModelDialog->updateModelfileList();

}

vector<string> VprofWindow::writeLog(const string& logpart)
{
  vector<string> vstr;
  miutil::miString str;

  if (logpart=="window") {

    str= "VprofWindow.size " + miutil::miString(this->width()) + " "
    + miutil::miString(this->height());
    vstr.push_back(str);
    str= "VprofWindow.pos "  + miutil::miString(this->x()) + " "
    + miutil::miString(this->y());
    vstr.push_back(str);
    str= "VprofModelDialog.pos " + miutil::miString(vpModelDialog->x()) + " "
    + miutil::miString(vpModelDialog->y());
    vstr.push_back(str);
    str= "VprofSetupDialog.pos " + miutil::miString(vpSetupDialog->x()) + " "
    + miutil::miString(vpSetupDialog->y());
    vstr.push_back(str);

    // printer name & options...
    if (priop.printer.exists()){
      str= "PRINTER " + priop.printer;
      vstr.push_back(str);
      if (priop.orientation==d_print::ori_portrait)
        str= "PRINTORIENTATION portrait";
      else
        str= "PRINTORIENTATION landscape";
      vstr.push_back(str);
    }

  } else if (logpart=="setup") {

    vstr= vprofm->writeLog();

  }

  return vstr;
}


void VprofWindow::readLog(const string& logpart, const vector<string>& vstr,
    const string& thisVersion, const string& logVersion,
    int displayWidth, int displayHeight)
{
  if (logpart=="window") {

    int n= vstr.size();

    for (int i=0; i<n; i++) {
      vector<string> tokens= miutil::split(vstr[i], 0, " ");

      if (tokens.size()==3) {

        int x= atoi(tokens[1].c_str());
        int y= atoi(tokens[2].c_str());
        if (x>20 && y>20 && x<=displayWidth && y<=displayHeight) {
          if (tokens[0]=="VprofWindow.size")  this->resize(x,y);
        }
        if (x>=0 && y>=0 && x<displayWidth-20 && y<displayHeight-20) {
          if      (tokens[0]=="VprofWindow.pos")      this->move(x,y);
          else if (tokens[0]=="VprofModelDialog.pos") vpModelDialog->move(x,y);
          else if (tokens[0]=="VprofSetupDialog.pos") vpSetupDialog->move(x,y);
        }

      } else if (tokens.size()==2) {

        if (tokens[0]=="PRINTER") {
          priop.printer=tokens[1];
        } else if (tokens[0]=="PRINTORIENTATION") {
          if (tokens[1]=="portrait")
            priop.orientation=d_print::ori_portrait;
          else
            priop.orientation=d_print::ori_landscape;
        }

      }
    }

  } else if (logpart=="setup") {

    vprofm->readLog(vstr,thisVersion,logVersion);

  }
}

/***************************************************************************/

void VprofWindow::closeEvent(QCloseEvent * e) {
  quitClicked();
}
