/*
  Diana - A Free Meteorological Visualisation Tool

  $Id$

  Copyright (C) 2013 met.no

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

#include <diDrawingManager.h>
#include <diController.h>
#include <EditItems/drawingdialog.h>
#include <EditItems/toolbar.h>
#include <EditItems/layergroupspane.h>
#include <EditItems/drawinglayerspane.h>
#include <EditItems/drawingpolyline.h>
#include <EditItems/drawingsymbol.h>
#include <EditItems/drawingtext.h>
#include <EditItems/drawingcomposite.h>

#include <paint_mode.xpm> // ### for now
#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QSplitter>
#include <QVBoxLayout>

#include <EditItems/layer.h>
#include <EditItems/layergroup.h>
#include <EditItems/layermanager.h>
#include <EditItems/kml.h>
#include <QPushButton> // ### FOR TESTING
#include <QCheckBox> // ### FOR TESTING
#include <QMessageBox>
#include <QDir>
#include <QDebug>

namespace EditItems {

DrawingDialog::DrawingDialog(QWidget *parent, Controller *ctrl)
  : DataDialog(parent, ctrl)
{
  drawm_ = DrawingManager::instance();

  // create an action that can be used to open the dialog from within a menu or toolbar
  m_action = new QAction(QIcon(QPixmap(paint_mode_xpm)), tr("Drawing Dialog"), this);
  m_action->setShortcutContext(Qt::ApplicationShortcut);
  m_action->setShortcut(Qt::ALT + Qt::Key_B);
  m_action->setCheckable(true);
  m_action->setIconVisibleInMenu(true);

  layerMgr_ = new EditItems::LayerManager();

  // create the GUI
  setWindowTitle("Drawing Dialog");
  setFocusPolicy(Qt::StrongFocus);
  QSplitter *splitter = new QSplitter(Qt::Vertical);
  layerGroupsPane_ = new LayerGroupsPane(layerMgr_);
  splitter->addWidget(layerGroupsPane_);
  layersPane_ = new DrawingLayersPane(layerMgr_, "Active Layers");
  layersPane_->init();
  splitter->addWidget(layersPane_);
  splitter->setSizes(QList<int>() << 500 << 500);
  //
  //
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(splitter);

  mainLayout->addLayout(createStandardButtons());

  // ### FOR TESTING
  QHBoxLayout *bottomLayout = new QHBoxLayout;
  //
  QPushButton *dsButton = new QPushButton("dump structure");
  connect(dsButton, SIGNAL(clicked()), SLOT(dumpStructure()));
  dsButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  bottomLayout->addWidget(dsButton);
  //
  QCheckBox *infoCBox = new QCheckBox("show info");
  connect(infoCBox, SIGNAL(toggled(bool)), SLOT(showInfo(bool)));
  infoCBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  bottomLayout->addWidget(infoCBox);
  //
  bottomLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
  mainLayout->addLayout(bottomLayout);

  // load available layer groups
  foreach (const QString &fileName, drawm_->getDrawings()) {
    QString error;
    const QList<QSharedPointer<Layer> > layers =
        KML::createFromFile<DrawingItemBase, DrawingItem_PolyLine::PolyLine, DrawingItem_Symbol::Symbol,
        DrawingItem_Text::Text, DrawingItem_Composite::Composite>(layerMgr_, fileName, &error);
    if (error.isEmpty())
      layerMgr_->addToNewLayerGroup(layers, fileName);
    else
      qDebug() << QString("DrawingDialog::DrawingDialog(): failed to load layer group from %1: %2").arg(fileName).arg(error).toLatin1().data();
  }
  layerGroupsPane_->updateWidgetStructure();

  // add connections
  connect(layerGroupsPane_, SIGNAL(updated()), layersPane_, SLOT(updateWidgetStructure()));
  connect(layersPane_, SIGNAL(updated()), layerGroupsPane_, SLOT(updateWidgetContents()));

  connect(layerGroupsPane_, SIGNAL(updated()), SLOT(handleDialogUpdated()));
  connect(layersPane_, SIGNAL(updated()), SLOT(handleDialogUpdated()));

  connect(this, SIGNAL(applyData()), SLOT(makeProduct()));
}

// ### FOR TESTING:
static void dumpLayerManagerStructure(const LayerManager *lm)
{
  const QList<QSharedPointer<LayerGroup> > &layerGroups = lm->layerGroups();
  qDebug() << QString("LAYER GROUPS (%1):").arg(layerGroups.size()).toLatin1().data();
  int i = 0;
  foreach (const QSharedPointer<LayerGroup> &lg, layerGroups) {
    QString layers_s;
    const QList<QSharedPointer<Layer> > layers = lg->layersRef();
    foreach (QSharedPointer<Layer> layer, layers)
      layers_s.append(QString("%1 ").arg((long)(layer.data()), 0, 16));
    qDebug()
        <<
           QString("  %1:%2  %3  >%4<  editable=%5  active=%6  layers: %7")
           .arg(i + 1)
           .arg(layerGroups.size())
           .arg((long)(lg.data()), 0, 16)
           .arg(lg->name(), 30)
           .arg(lg->isEditable() ? 1 : 0)
           .arg(lg->isActive() ? 1 : 0)
           .arg(layers_s)
           .toLatin1().data();
    i++;
  }

  const QList<QSharedPointer<Layer> > &layers = lm->orderedLayers();
  qDebug() << QString("ORDERED LAYERS (%1):").arg(layers.size()).toLatin1().data();
  i = 0;
  foreach (const QSharedPointer<Layer> &layer, layers) {
    qDebug()
        <<
           QString("  %1:%2  %3  >%4<  [%5]  LG:%6  editable:%7  active:%8  visible:%9  nItems:%10  nSelItems:%11")
           .arg(i + 1)
           .arg(layers.size())
           .arg((long)(layer.data()), 0, 16)
           .arg(layer->name(), 30)
           .arg(lm->selectedLayers().contains(layer) ? "sel" : "   ")
           .arg((long)(layer->layerGroupRef().data()), 0, 16)
           .arg(layer->isEditable() ? 1 : 0)
           .arg(layer->isActive() ? 1 : 0)
           .arg(layer->isVisible() ? 1 : 0)
           .arg(layer->itemCount())
           .arg(layer->selectedItemCount())
           .toLatin1().data();
    i++;
  }
}

// ### FOR TESTING:
void DrawingDialog::dumpStructure()
{
  qDebug() << "\nLAYER MANAGERS:";
  qDebug() << "1: In DrawingDialog: =====================================";
  dumpLayerManagerStructure(layerMgr_);
  qDebug() << "\n2: In DrawingManager: =====================================";
  dumpLayerManagerStructure(drawm_->getLayerManager());
}

void DrawingDialog::showInfo(bool checked)
{
  layerGroupsPane_->showInfo(checked);
  layersPane_->showInfo(checked);
}

std::string DrawingDialog::name() const
{
  return "DRAWING";
}

void DrawingDialog::updateTimes()
{
  std::vector<miutil::miTime> times;
  times = drawm_->getTimes();
  emit emitTimes("DRAWING", times);
}

void DrawingDialog::updateDialog()
{
}

std::vector<std::string> DrawingDialog::getOKString()
{
  std::vector<std::string> lines;

  if (!drawm_->isEnabled())
    return lines;

  foreach (QString filePath, drawm_->getLoaded()) {
    QString line = "DRAWING file=\"" + filePath + "\"";
    lines.push_back(line.toStdString());
  }

  return lines;
}

void DrawingDialog::putOKString(const std::vector<std::string>& vstr)
{
  // Submit the lines as new input.
  std::vector<std::string> inp;
  inp.insert(inp.begin(), vstr.begin(), vstr.end());
  drawm_->processInput(inp);
}

void DrawingDialog::makeProduct()
{
  // Write the layers to temporary files in the working directory.
  QDir dir(drawm_->getWorkDir());
  QString filePath = dir.absoluteFilePath("temp.kml");
  layersPane_->saveVisible(filePath);

  std::vector<std::string> inp;
  inp.push_back("DRAWING file=\"" + filePath.toStdString() + "\"");

  putOKString(inp);

  // Update the available times.
  updateTimes();

  indicateUnappliedChanges(false); // indicate that the DrawingManager is now synchronized with the dialog
}

void DrawingDialog::handleDialogUpdated()
{
  indicateUnappliedChanges(true);
}


} // namespace
