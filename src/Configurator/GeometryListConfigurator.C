/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
         
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software  
  Foundation, either version 3 of the License, or (at your option) any later  
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.
   
********************************************************************************/

#include "MoleculeLayer.h"
#include "GeometryLayer.h"
#include "GeometryListLayer.h"
#include "GeometryListConfigurator.h"
#include <QHeaderView>

#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Configurator {


GeometryList::GeometryList(Layer::GeometryList& geometryList) : m_geometryList(geometryList)
{
   m_configurator.setupUi(this);
   m_configurator.energyTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}


void GeometryList::load()
{
   QTableWidget* table(m_configurator.energyTable);
   QList<Layer::Geometry*> 
      geometries(m_geometryList.findLayers<Layer::Geometry>(Layer::Children));
   table->setRowCount(geometries.size());

   QTableWidgetItem* energy;

   int row(0);
   QList<Layer::Geometry*>::iterator iter;
   for (iter = geometries.begin(); iter != geometries.end(); ++iter, ++row) {
       energy = new QTableWidgetItem( (*iter)->text() );
       energy->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       table->setItem(row, 0, energy);
   }

   if (geometries.size() < 2) {
      m_configurator.playButton->setEnabled(false);
      m_configurator.forwardButton->setEnabled(false);
      m_configurator.backButton->setEnabled(false);
      m_configurator.bounceButton->setEnabled(false);
      m_configurator.updateBondsButton->setEnabled(false);
      m_configurator.speedSlider->setEnabled(false);
      m_configurator.speedLabel->setEnabled(false);
   }
}


void GeometryList::reset()
{
   m_configurator.playButton->setChecked(false);
   m_configurator.backButton->setEnabled(true);
   m_configurator.forwardButton->setEnabled(true);
}


void GeometryList::on_playButton_clicked(bool play)
{
   if (play) {
      QTableWidget* table(m_configurator.energyTable);
      table->setCurrentCell(-1, 0,  // clear the current selection 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }

   m_configurator.backButton->setEnabled(!play);
   m_configurator.forwardButton->setEnabled(!play);
   m_geometryList.setPlay(play);
}


void GeometryList::on_backButton_clicked(bool)
{
   QTableWidget* table(m_configurator.energyTable);
   int currentRow(table->currentRow());
   if (currentRow > 0) {
      table->setCurrentCell(currentRow-1, 0, 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void GeometryList::on_forwardButton_clicked(bool)
{
   QTableWidget* table(m_configurator.energyTable);
   int currentRow(table->currentRow());
   if (currentRow < table->rowCount()-1) {
      table->setCurrentCell(currentRow+1, 0, 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void GeometryList::on_speedSlider_valueChanged(int value)
{
   // Default is 25 => 0.125 in GeometryList ctor
   m_geometryList.setSpeed(value/200.0);
}


void GeometryList::on_energyTable_itemSelectionChanged()
{
qDebug()<<"GeometryList::on_energyTable_itemSelectionChanged() called";
   QList<QTableWidgetItem*> selection = m_configurator.energyTable->selectedItems();
   if (!selection.isEmpty()) {
      int row(selection.first()->row());
      if (row > -1) m_geometryList.setCurrentGeometry(row);
   }
}


void GeometryList::on_bounceButton_clicked(bool tf)
{
   m_geometryList.setBounce(tf);
}


void GeometryList::on_loopButton_clicked(bool tf)
{
   m_geometryList.setLoop(tf);
}


void GeometryList::on_updateBondsButton_clicked(bool tf)
{
   m_geometryList.setReperceiveBonds(tf);
   on_energyTable_itemSelectionChanged(); 
}


void GeometryList::closeEvent(QCloseEvent* e)
{
   on_playButton_clicked(false);
   m_geometryList.resetGeometry();
   Base::closeEvent(e);
}

} } // end namespace IQmol::Configurator
