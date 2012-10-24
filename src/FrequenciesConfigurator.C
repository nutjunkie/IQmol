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

#include "FrequenciesConfigurator.h"
#include "QVariantPointer.h"
#include "FrequenciesLayer.h"
#include "Preferences.h"
#include "AtomLayer.h"
#include <QColorDialog>
#include <QHeaderView>


namespace IQmol {
namespace Configurator { 

Frequencies::Frequencies(Layer::Frequencies* frequencies) : m_frequencies(frequencies)
{
   m_frequenciesConfigurator.setupUi(this);

   m_frequenciesConfigurator.frequencyTable->horizontalHeader()
      ->setResizeMode(QHeaderView::Stretch);

   connect(this, SIGNAL(update()), frequencies, SIGNAL(update()));

   setVectorColor(Preferences::VibrationVectorColor());
}


void Frequencies::load()
{
   QTableWidget* table(m_frequenciesConfigurator.frequencyTable);
   QList<Layer::Mode*> modes(m_frequencies->findLayers<Layer::Mode>(Layer::Children));
   table->setRowCount(modes.size());

   QTableWidgetItem* frequency;
   QTableWidgetItem* intensity;
         
   int row(0);
   QList<Layer::Mode*>::iterator iter;

   for (iter = modes.begin(); iter != modes.end(); ++iter) {
       frequency = new QTableWidgetItem( (*iter)->text() );
       frequency->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

       if (m_frequencies->haveIntensities()) {
          intensity = new QTableWidgetItem(QString::number((*iter)->intensity(), 'f', 3));
       }else {
          intensity = new QTableWidgetItem("-");
       }

       frequency->setData(Qt::UserRole, QVariantPointer<Layer::Mode>::toQVariant(*iter));
       intensity->setData(Qt::UserRole, QVariantPointer<Layer::Mode>::toQVariant(*iter));

       intensity->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
       table->setItem(row, 0, frequency);
       table->setItem(row, 1, intensity);
       ++row;
   }

   table->setCurrentCell(0, 0, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   on_frequencyTable_itemSelectionChanged();
}


void Frequencies::reset()
{
   m_frequenciesConfigurator.playButton->setChecked(false);
   m_frequencies->setPlay(false);
   on_frequencyTable_itemSelectionChanged();
}


void Frequencies::on_playButton_clicked(bool play)
{
   m_frequencies->setPlay(play);
}


void Frequencies::on_backButton_clicked(bool)
{
   QTableWidget* table(m_frequenciesConfigurator.frequencyTable);
   int currentRow(table->currentRow());
   if (currentRow > 0) {
      table->setCurrentCell(currentRow-1, 0,
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void Frequencies::on_forwardButton_clicked(bool)
{
   QTableWidget* table(m_frequenciesConfigurator.frequencyTable);
   int currentRow(table->currentRow());
   if (currentRow < table->rowCount()-1) {
      table->setCurrentCell(currentRow+1, 0,
         QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
   }
}


void Frequencies::on_colorButton_clicked(bool)
{
   QColor vectorColor(Preferences::VibrationVectorColor());
   setVectorColor(QColorDialog::getColor(vectorColor, this));
}


void Frequencies::on_loopButton_clicked(bool tf)
{
   m_frequencies->setLoop(tf);
   on_frequencyTable_itemSelectionChanged();
}


void Frequencies::setVectorColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_frequenciesConfigurator.colorButton->setStyleSheet(bg);
      Layer::Atom::setVibrationVectorColor(color);
      Preferences::VibrationVectorColor(color);
      update();
   }
}


void Frequencies::on_amplitudeSlider_valueChanged(int value)
{
   m_frequencies->setScale(value/100.0);
}


void Frequencies::on_speedSlider_valueChanged(int value)
{
   // Default is 25 => 0.0625 in Frequencies ctor
   m_frequencies->setSpeed(value/400.0);
}


void Frequencies::on_frequencyTable_itemDoubleClicked(QTableWidgetItem* item)
{
   Layer::Mode* mode = QVariantPointer<Layer::Mode>::toPointer(item->data(Qt::UserRole));
   if (mode) {
      m_frequencies->setActiveMode(*mode);
      m_frequencies->setPlay(true);
      m_frequenciesConfigurator.playButton->setChecked(true);
   }
}


void Frequencies::on_frequencyTable_itemSelectionChanged()
{
   QList<QTableWidgetItem*> selection = m_frequenciesConfigurator.frequencyTable->selectedItems();
   if (selection.isEmpty()) return;
   Layer::Mode* mode = QVariantPointer<Layer::Mode>::toPointer(
       selection.first()->data(Qt::UserRole));
   if (mode) {
      m_frequencies->setActiveMode(*mode);
      m_frequencies->setPlay();
   }
}


void Frequencies::closeEvent(QCloseEvent* e)
{
   m_frequencies->clearActiveMode();
   m_frequencies->setPlay(false);
   Base::closeEvent(e);
}

} } // end namespace IQmol::Configurator
