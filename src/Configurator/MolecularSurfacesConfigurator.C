/*******************************************************************************

  Copyright (C) 2011-2015 Andrew Gilbert

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

#include "MolecularSurfacesConfigurator.h"
#include "SurfaceInfo.h"
#include "Preferences.h"
#include <QColorDialog>


namespace IQmol {
namespace Configurator {

MolecularSurfaces::MolecularSurfaces()
{
   m_configurator.setupUi(this);

   m_configurator.surfaceType->clear();
   m_configurator.surfaceType->addItem("van der Waals", Data::SurfaceType::VanDerWaals);
   m_configurator.surfaceType->addItem("Promolecule",   Data::SurfaceType::Promolecule);
   m_configurator.surfaceType->addItem("SID",           Data::SurfaceType::SID);
   m_configurator.surfaceType->setCurrentIndex(0);

   setPositiveColor(Preferences::PositiveSurfaceColor());
   setNegativeColor(Preferences::NegativeSurfaceColor());
   enableNegativeColor(false);
   m_configurator.opacity->setValue(Preferences::SurfaceOpacity());
}


void MolecularSurfaces::on_surfaceType_currentIndexChanged(int index) 
{
   QVariant qvar(m_configurator.surfaceType->itemData(index));

   switch (qvar.toUInt()) {

      case Data::SurfaceType::VanDerWaals:
         m_configurator.isovalue->setSuffix("  ");
         m_configurator.isovalue->setValue(1.000);
         m_configurator.isovalueLabel->setText("Scale");
         break;

      case Data::SurfaceType::Promolecule:
         m_configurator.isovalue->setSuffix("  ");
         m_configurator.isovalue->setValue(0.020);
         m_configurator.isovalueLabel->setText("Isovalue");
         break;

      case Data::SurfaceType::SID:
         m_configurator.isovalue->setSuffix("  ");
         m_configurator.isovalue->setValue(0.020);
         m_configurator.isovalueLabel->setText("Isovalue");
         break;
   } 
}


void MolecularSurfaces::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(Preferences::PositiveSurfaceColor(), this));
}


void MolecularSurfaces::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(Preferences::NegativeSurfaceColor(), this));
}


void MolecularSurfaces::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.positiveColorButton->setStyleSheet(bg);
      Preferences::PositiveSurfaceColor(color);
   }
}


void MolecularSurfaces::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_configurator.negativeColorButton->setStyleSheet(bg);
      Preferences::NegativeSurfaceColor(color);
   }
}


void MolecularSurfaces::on_cancelButton_clicked(bool)
{ 
   close(); 
}


void MolecularSurfaces::on_calculateButton_clicked(bool)
{
   Preferences::SurfaceOpacity(m_configurator.opacity->value());

   int quality(m_configurator.quality->value());
   double isovalue(m_configurator.isovalue->value());
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

      case Data::SurfaceType::VanDerWaals:
         info.type().setKind(Data::SurfaceType::VanDerWaals);
          break;
      case Data::SurfaceType::Promolecule:
         info.type().setKind(Data::SurfaceType::Promolecule);
         info.setIsSigned(false);
         break;
      case Data::SurfaceType::SID:
         info.type().setKind(Data::SurfaceType::SID);
         info.setIsSigned(false);
         break;
   }

   info.setIsSigned(false);

   calculateSurface(info);
   accept();
}


void MolecularSurfaces::enableNegativeColor(bool tf)
{
   m_configurator.negativeColorButton->setVisible(tf);
   m_configurator.negativeLabel->setVisible(tf);
}

} } // end namespace IQmol::Configurator
