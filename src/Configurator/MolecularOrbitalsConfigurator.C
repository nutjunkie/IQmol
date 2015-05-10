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

#include "MolecularOrbitalsConfigurator.h"
#include "MolecularOrbitalsLayer.h"
#include "Spin.h"
#include "Preferences.h"
#include <QColorDialog>


namespace IQmol {
namespace Configurator {

MolecularOrbitals::MolecularOrbitals(Layer::MolecularOrbitals& molecularOrbitals)
  : m_molecularOrbitals(molecularOrbitals)
{
   m_configurator.setupUi(this);

   connect(m_configurator.orbitalRangeMin, SIGNAL(currentIndexChanged(int)),
      m_configurator.orbitalRangeMax, SLOT(setCurrentIndex(int)));
 
   m_configurator.surfaceType->clear();
   m_configurator.surfaceType->addItem("Orbital", Orbital);
   m_configurator.surfaceType->addItem("Density", Density);
   m_configurator.surfaceType->addItem("Spin Density", SpinDiffDensity);
   m_configurator.surfaceType->addItem("Spin Only Density", SpinOnlyDensity);
   m_configurator.surfaceType->setCurrentIndex(0);

   setPositiveColor(Preferences::PositiveSurfaceColor());
   setNegativeColor(Preferences::NegativeSurfaceColor());
   m_configurator.opacity->setValue(Preferences::SurfaceOpacity());
}


void MolecularOrbitals::init() 
{ 
   m_nAlpha = m_molecularOrbitals.nAlpha();
   m_nBeta  = m_molecularOrbitals.nBeta();
   m_nOrbitals = m_molecularOrbitals.nOrbitals();
   updateOrbitalRange(m_nAlpha);
}


void MolecularOrbitals::on_surfaceType_currentIndexChanged(int index) 
{
   QVariant qvar(m_configurator.surfaceType->itemData(index));

   switch (qvar.toUInt()) {
      case Orbital:
         enableOrbitalSelection(true);
         enableSpin(true);
         enableNegativeColor(true);
         break;

      case Density:
         enableOrbitalSelection(false);
         enableSpin(false);
         enableNegativeColor(false);
         break;

      case SpinDiffDensity:
         enableOrbitalSelection(false);
         enableSpin(false);
         enableNegativeColor(true);
         break;

      case SpinOnlyDensity:
         enableOrbitalSelection(false);
         enableNegativeColor(false);
         enableSpin(true);
         break;

      default:
         enableOrbitalSelection(false);
         break;
   }
}


void MolecularOrbitals::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(Preferences::PositiveSurfaceColor(), this));
}


void MolecularOrbitals::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(Preferences::NegativeSurfaceColor(), this));
}


void MolecularOrbitals::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.positiveColorButton->setStyleSheet(bg);
      Preferences::PositiveSurfaceColor(color);
   }
}


void MolecularOrbitals::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.negativeColorButton->setStyleSheet(bg);
      Preferences::NegativeSurfaceColor(color);
   }
}


void MolecularOrbitals::on_cancelButton_clicked(bool)
{ 
   clearQueue(); 
   close(); 
}


void MolecularOrbitals::on_calculateButton_clicked(bool)
{ 
   on_addToQueueButton_clicked(true);
   Preferences::SurfaceOpacity(m_configurator.opacity->value());
   accept();
   calculateSurfaces(); 
}


void MolecularOrbitals::on_addToQueueButton_clicked(bool) 
{
   int quality(m_configurator.quality->value());
   double isovalue(m_configurator.isovalue->value());
   Spin spin = m_configurator.alphaRadio->isChecked() ? Alpha : Beta;
   QColor positive(Preferences::PositiveSurfaceColor());
   QColor negative(Preferences::NegativeSurfaceColor());
   bool simplifyMesh(m_configurator.simplifyMeshCheckBox->isChecked());

   int index(m_configurator.surfaceType->currentIndex());
   QVariant qvar(m_configurator.surfaceType->itemData(index));
   bool isSigned(true);

   int op(m_configurator.opacity->value());
   double opacity = op == 100 ? 0.999 : double(op)/100.0;

   Data::SurfaceInfo info(Data::SurfaceType::Custom, quality, isovalue, 
     positive, negative, isSigned, simplifyMesh, opacity);

   switch (qvar.toUInt()) {

      case Orbital: {
         if (spin == Alpha) {
            info.type().setKind(Data::SurfaceType::AlphaOrbital);
         }else {
            info.type().setKind(Data::SurfaceType::BetaOrbital);
         }

         int orb1(m_configurator.orbitalRangeMin->currentIndex()+1);
         int orb2(m_configurator.orbitalRangeMax->currentIndex()+1);

         for (int i = std::min(orb1,orb2); i <= std::max(orb1, orb2); ++i) {
             info.type().setIndex(i);
             queueSurface(info);
         }
      } break;

      case Density: {
         info.setIsSigned(false);
         info.type().setKind(Data::SurfaceType::TotalDensity);
         queueSurface(info);
      } break;

      case SpinDiffDensity: {
            info.type().setKind(Data::SurfaceType::SpinDensity);
         queueSurface(info);
      } break;

      case SpinOnlyDensity: {
         info.setIsSigned(false);
         if (spin == Alpha) {
            info.type().setKind(Data::SurfaceType::AlphaDensity);
         }else {
            info.type().setKind(Data::SurfaceType::BetaDensity);
         }
         queueSurface(info);
      } break;

   }
}


void MolecularOrbitals::enableOrbitalSelection(bool tf)
{
   m_configurator.orbitalRangeMin->setEnabled(tf);
   m_configurator.orbitalRangeMax->setEnabled(tf);
}


void MolecularOrbitals::enableSpin(bool tf)
{
   m_configurator.alphaRadio->setEnabled(tf);
   m_configurator.betaRadio->setEnabled(tf);
}


void MolecularOrbitals::enableNegativeColor(bool tf)
{
   m_configurator.negativeColorButton->setVisible(tf);
   m_configurator.negativeLabel->setVisible(tf);
}


void MolecularOrbitals::updateOrbitalRange(int nElectrons) 
{
   int index;
   QComboBox* combo;

   combo = m_configurator.orbitalRangeMin;
   index = combo->currentIndex();
   updateOrbitalRange(nElectrons, combo);
   if (index < 0 || index >= (int)m_nOrbitals) index = nElectrons-1;
   combo->setCurrentIndex(index);

   combo = m_configurator.orbitalRangeMax;
   index = combo->currentIndex();
   updateOrbitalRange(nElectrons, combo);
   if (index < 0 || index >= (int)m_nOrbitals) index = nElectrons;
   combo->setCurrentIndex(index);
}


void MolecularOrbitals::updateOrbitalRange(int nElectrons, QComboBox* combo) 
{
   combo->clear();
   for (unsigned int i = 1; i <= m_nOrbitals; ++i) {
       combo->addItem(QString::number(i));
   }
   
   combo->setItemText(nElectrons-1, QString::number(nElectrons) + " (HOMO)");
   combo->setItemText(nElectrons,   QString::number(nElectrons+1) + " (LUMO)");
}

} } // end namespace IQmol::Configurator
