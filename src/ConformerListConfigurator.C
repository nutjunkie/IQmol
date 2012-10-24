/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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
#include "ConformerListLayer.h"
#include "ConformerListConfigurator.h"
#include <QHeaderView>


using namespace qglviewer;

namespace IQmol {
namespace Configurator {


ConformerList::ConformerList(Layer::ConformerList* conformerList) : m_conformerList(conformerList)
{
   m_conformerListConfigurator.setupUi(this);
   m_conformerListConfigurator.energyTable->verticalHeader()
      ->setResizeMode(QHeaderView::ResizeToContents);
}


void ConformerList::load()
{
   QTableWidget* table(m_conformerListConfigurator.energyTable);
   QList<Layer::Conformer*> 
      conformers(m_conformerList->findLayers<Layer::Conformer>(Layer::Children));
   table->setRowCount(conformers.size());

   QTableWidgetItem* energy;

   int row(0);
   QList<Layer::Conformer*>::iterator iter;

   for (iter = conformers.begin(); iter != conformers.end(); ++iter) {
       energy = new QTableWidgetItem( (*iter)->text() );
       energy->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
       energy->setData(Qt::UserRole, QVariantPointer<Layer::Conformer>::toQVariant(*iter));
       table->setItem(row, 0, energy);
       ++row;
   }

   if (conformers.size() < 2) {
      m_conformerListConfigurator.playButton->setEnabled(false);
      m_conformerListConfigurator.forwardButton->setEnabled(false);
      m_conformerListConfigurator.backButton->setEnabled(false);
      m_conformerListConfigurator.bounceButton->setEnabled(false);
      m_conformerListConfigurator.updateBondsButton->setEnabled(false);
      m_conformerListConfigurator.speedSlider->setEnabled(false);
      m_conformerListConfigurator.speedLabel->setEnabled(false);
   }
}


void ConformerList::reset()
{
   m_conformerListConfigurator.playButton->setChecked(false);
   m_conformerList->setDefaultConformer();
   on_energyTable_itemSelectionChanged();
}


void ConformerList::on_playButton_clicked(bool play)
{
   m_conformerListConfigurator.playButton->setChecked(play);
   m_conformerList->setPlay(play);
}


void ConformerList::on_backButton_clicked(bool)
{
   QTableWidget* table(m_conformerListConfigurator.energyTable);
   int currentRow(table->currentRow());
   if (currentRow > 0) {
      table->setCurrentCell(currentRow-1, 0, 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void ConformerList::on_forwardButton_clicked(bool)
{
   QTableWidget* table(m_conformerListConfigurator.energyTable);
   int currentRow(table->currentRow());
   if (currentRow < table->rowCount()-1) {
      table->setCurrentCell(currentRow+1, 0, 
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void ConformerList::on_speedSlider_valueChanged(int value)
{
   // Default is 25 => 0.125 in ConformerList ctor
   m_conformerList->setSpeed(value/200.0);
}


void ConformerList::on_energyTable_itemSelectionChanged()
{
   QList<QTableWidgetItem*> selection = m_conformerListConfigurator.energyTable->selectedItems();
   on_playButton_clicked(false); 
   if (selection.isEmpty()) return;
   Layer::Conformer* conformer = QVariantPointer<Layer::Conformer>::toPointer(
      selection.first()->data(Qt::UserRole));
   if (conformer) m_conformerList->setActiveConformer(*conformer);
}


void ConformerList::on_bounceButton_clicked(bool tf)
{
   m_conformerList->setBounce(tf);
}


void ConformerList::on_loopButton_clicked(bool tf)
{
   m_conformerList->setLoop(tf);
}


void ConformerList::on_updateBondsButton_clicked(bool tf)
{
   m_conformerList->reperceiveBonds(tf);
   on_energyTable_itemSelectionChanged(); 
}


void ConformerList::closeEvent(QCloseEvent* e)
{
   m_conformerList->setDefaultConformer();
   on_playButton_clicked(false); 
   Base::closeEvent(e);
}

} } // end namespace IQmol::Configurator
