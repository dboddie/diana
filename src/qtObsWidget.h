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
#ifndef _obsWidget_h
#define _obsWidget_h


#include <qcolor.h>
#include <qdialog.h>
#include <QLabel>
#include <diCommonTypes.h>
#include <diController.h>
#include <qtToggleButton.h>
#include <map>

using namespace std;

class QSlider;
class QLabel;
class QComboBox;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QLCDNumber;
class QCHeckBox;

class ButtonLayout;

class QColor;

/**

  \brief Observation settings for one plot style

   Widget for selection of data types, parameters etc.

*/
class ObsWidget : public QWidget
{
  Q_OBJECT
public:

  ObsWidget(QWidget* parent );
  ///init dialog
  void setDialogInfo(Controller* ctrl, ObsDialogInfo,int index);
  ///initialized?
  bool initialized() {return initOK;}
  ///return command strings
  miutil::miString getOKString(bool forLog= false);
  ///insert command strings
  void putOKString(const miutil::miString& str);
  ///return short name of current commonad
  miutil::miString getShortname();
  ///return legal command strings and update dialog
  void requestQuickUpdate(miutil::miString& oldstr,miutil::miString& newstr);
  void readLog(const miutil::miString& str);
  void setFalse();
  void setDatatype(const miutil::miString&);
  vector<miutil::miString> getDataTypes();
  bool moreToggled(){return moreButton->isOn();}
  //Criteria
  ObsDialogInfo::CriteriaList getSavedCriteria(){return savedCriteria;}
  ObsDialogInfo::CriteriaList getCriteriaList();
  bool setCurrentCriteria(int i);
  int getCurrentCriteria(){return currentCriteria;}
  vector<miutil::miString> getCriteriaNames();
  void saveCriteria(const vector<miutil::miString>& criteria);
  bool saveCriteria(const vector<miutil::miString>& criteria,
		    const miutil::miString& name);
  bool getCriteriaLimits(const miutil::miString& name, int& low, int& high);
  void markButton(const miutil::miString& name, bool on);
  bool noButton(){return nobutton;}

private slots:
  void priSelected( int index);
  void outTopClicked( int id );
  void inTopClicked( int id );
  void displayDensity( int number );
  void displaySize( int number );
  void displayDiff( int number );
  void diffComboBoxSlot( int number);
  void devFieldChecked(bool on);
  void criteriaChecked(bool on );
  void onlyposChecked(bool on );
  void extensionSlot(bool on);
  void rightClickedSlot(miutil::miString str);

signals:
  void getTimes();
  void rightClicked(miutil::miString);
  void setRightClicked(miutil::miString,bool);
  void extensionToggled(bool);
  void criteriaOn();
  void newCriteriaList(ObsDialogInfo::CriteriaList);

private:
  void ToolTip();

  bool initOK;

  vector<Colour::ColourInfo> cInfo;
  vector<miutil::miString> markerName;

  ButtonLayout* datatypeButtons;
  ButtonLayout* parameterButtons;

  QPushButton* allButton;
  QPushButton* noneButton;
  QPushButton* defButton;
  QCheckBox* tempPrecisionCheckBox;
  QCheckBox* parameterNameCheckBox;
  QCheckBox* moreTimesCheckBox;
  QCheckBox* devFieldCheckBox;
  QComboBox* devColourBox1;
  QComboBox* devColourBox2;
  QCheckBox* allAirepsLevelsCheckBox;
  QCheckBox* orientCheckBox;
  QCheckBox* showposCheckBox;
  QCheckBox* onlyposCheckBox;
  QComboBox* markerBox;

  QComboBox* pressureComboBox;
  QComboBox* leveldiffComboBox;

  QLCDNumber* densityLcdnum;
  QLCDNumber* sizeLcdnum;
  QLCDNumber* diffLcdnum;
  QSlider* densitySlider;
  QSlider* sizeSlider;
  QSlider* diffSlider;
  QComboBox* diffComboBox;

  QComboBox* colourBox;
  QComboBox* pribox;    //List of priority files

  QCheckBox* criteriaCheckBox;
  ToggleButton* moreButton;

  vector<ObsDialogInfo::PriorityList> priorityList;
  vector<ObsDialogInfo::CriteriaList> criteriaList;
  ObsDialogInfo::CriteriaList savedCriteria;
  int currentCriteria;

  int nr_dataTypes;
  vector<ObsDialogInfo::Button> button;
  vector<ObsDialogInfo::Button> dataTypeButton;
  vector<ObsDialogInfo::DataType> datatype;
  vector<int> time_slider2lcd;

  struct dialogVariables {
    miutil::miString plotType;
    vector<miutil::miString> data;
    vector<miutil::miString> parameter;
    map<miutil::miString,miutil::miString> misc;
  };

  dialogVariables dVariables;

  void decodeString(const miutil::miString&, dialogVariables&,bool fromLog=false);
  void updateDialog(bool setOn);
  miutil::miString makeString(bool forLog=false);

  bool pressureLevels;
  map<miutil::miString,int> levelMap;
  bool leveldiffs;
  map<miutil::miString,int> leveldiffMap;

  // LCD <-> slider
  float scaledensity;
  float scalesize;
  float scalediff;
  int mindensity;
  int maxdensity;

  bool markerboxVisible;
  bool allObs;
  int selectedPressure;
  miutil::miString timediff_minutes;
  int pri_selected;     // Wich priority file
  miutil::miString plotType;
  miutil::miString shortname;
  bool nobutton;

  QVBoxLayout *vlayout;
  QVBoxLayout *vcommonlayout;
  QHBoxLayout *parameterlayout;
  QHBoxLayout *datatypelayout;


};



#endif
