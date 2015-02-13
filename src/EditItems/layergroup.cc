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

#include <EditItems/layergroup.h>
#include <EditItems/layer.h>

namespace EditItems {

LayerGroup::LayerGroup(const QString &name, bool editable, bool active)
  : id_(nextId())
  , name_(name)
  , editable_(editable)
  , active_(active)
{
}

LayerGroup::LayerGroup(const LayerGroup &other)
  : id_(other.id_)
  , name_(other.name_)
  , editable_(other.editable_)
  , active_(other.active_)
{
  foreach (const QSharedPointer<Layer> &layer, other.layers_)
    layers_.append(QSharedPointer<Layer>(new Layer(*(layer.data()))));
}

LayerGroup::~LayerGroup()
{
}

int LayerGroup::id() const
{
  return id_;
}

int LayerGroup::nextId_ = 0;

int LayerGroup::nextId()
{
  return nextId_++; // ### not thread safe; use a mutex for that
}

QString LayerGroup::name() const
{
  return name_;
}

void LayerGroup::setName(const QString &n)
{
  name_ = n;
}

bool LayerGroup::isEditable() const
{
  return editable_;
}

bool LayerGroup::isActive() const
{
  return active_;
}

void LayerGroup::setActive(bool active)
{
  active_ = active;
}

const QList<QSharedPointer<Layer> > &LayerGroup::layersRef() const
{
  return layers_;
}

QList<QSharedPointer<Layer> > &LayerGroup::layersRef()
{
  return layers_;
}

QSet<QString> LayerGroup::getTimes() const
{
  QSet<QString> times;

  for (int i = 0; i < tfiles_.size(); ++i)
    times.insert(tfiles_.at(i).second.toString(Qt::ISODate) + "Z");

  // The following strings could be made configurable:
  static const char* timeProps[2] = {"time", "TimeSpan:begin"};

  foreach (const QSharedPointer<Layer> &layer, layersRef()) {

    if (layer->isVisible()) {
      foreach (const QSharedPointer<DrawingItemBase> item, layer->items()) {
        QString timeStr;
        for (unsigned int i = 0; i < 2; ++i) {
          timeStr = item->propertiesRef().value(timeProps[i]).toString();
          if (!timeStr.isEmpty()) {
            times.insert(timeStr);
            break;
          }
        }
      }
    }
  }

  return times;
}

void LayerGroup::setFiles(const QList<QPair<QFileInfo, QDateTime> > &tfiles)
{
  tfiles_ = tfiles;
}

} // namespace
