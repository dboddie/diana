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

#include <diEditItemManager.h>
#include <diController.h>
#include <EditItems/editdrawingdialog.h>
#include <EditItems/toolbar.h>
#include <EditItems/layergroupspane.h>
#include <EditItems/editdrawinglayerspane.h>
#include <paint_mode.xpm> // ### for now
#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QVBoxLayout>

#include <EditItems/layer.h>
#include <EditItems/layergroup.h>
#include <EditItems/layermanager.h>
#include <QPushButton> // ### FOR TESTING
#include <QCheckBox> // ### FOR TESTING
#include <QDir>
#include <qtUtility.h>

#include <QDebug>

namespace EditItems {

EditDrawingDialog::EditDrawingDialog(QWidget *parent, Controller *ctrl)
  : DataDialog(parent, ctrl)
{
  editm_ = EditItemManager::instance();

  // create an action that can be used to open the dialog from within a menu or toolbar
  m_action = new QAction(QIcon(QPixmap(paint_mode_xpm)), tr("Edit Drawing Dialog"), this);
  m_action->setShortcutContext(Qt::ApplicationShortcut);
  m_action->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_B);
  m_action->setCheckable(true);
  m_action->setIconVisibleInMenu(true);

  // create the GUI
  setWindowTitle("Edit Drawing Dialog");
  setFocusPolicy(Qt::StrongFocus);
  layersPane_ = new EditDrawingLayersPane(editm_->getLayerManager(), "Active Layers");
  layersPane_->init();
  //
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(layersPane_);

  QPushButton *hideButton = NormalPushButton(tr("Hide"), this);
  connect(hideButton, SIGNAL(clicked()), SIGNAL(hideData()));
  QHBoxLayout* hideLayout = new QHBoxLayout();
  hideLayout->addWidget(hideButton);
  mainLayout->addLayout(hideLayout);

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

  connect(layersPane_, SIGNAL(updated()), editm_, SLOT(handleLayersUpdate()));
}

std::string EditDrawingDialog::name() const
{
  return "EDITDRAWING";
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
           .arg(layer == lm->currentLayer() ? "curr" : "    ")
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
void EditDrawingDialog::dumpStructure()
{
  qDebug() << "\nLAYER MANAGERS:";
  qDebug() << "\n1: In EditDrawingManager: =====================================";
  dumpLayerManagerStructure(editm_->getLayerManager());
}

void EditDrawingDialog::showInfo(bool checked)
{
  layersPane_->showInfo(checked);
}

} // namespace