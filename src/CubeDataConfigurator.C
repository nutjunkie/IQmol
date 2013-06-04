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

#include "CubeDataLayer.h"
#include "SurfaceLayer.h"
#include "Preferences.h"
#include <QColorDialog>


namespace IQmol {
namespace Configurator {

CubeData::CubeData(Layer::CubeData* cubeFile) : m_cubeFile(cubeFile)
{
   m_cubeDataConfigurator.setupUi(this);
   setPositiveColor(Preferences::PositiveSurfaceColor());
   setNegativeColor(Preferences::NegativeSurfaceColor());
}


void CubeData::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(Preferences::PositiveSurfaceColor(), this));
}


void CubeData::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(Preferences::NegativeSurfaceColor(), this));
}


void CubeData::on_signedButton_clicked(bool)
{
   if (m_cubeDataConfigurator.signedButton->checkState() == Qt::Checked) {
      m_cubeDataConfigurator.negativeLabel->setVisible(true);
      m_cubeDataConfigurator.positiveLabel->setVisible(true);
      m_cubeDataConfigurator.negativeColorButton->setVisible(true);
      m_cubeDataConfigurator.isovalue->setRange(0.001, 99.9999);
   }else {
      m_cubeDataConfigurator.negativeLabel->setVisible(false);
      m_cubeDataConfigurator.positiveLabel->setVisible(false);
      m_cubeDataConfigurator.negativeColorButton->setVisible(false);
      m_cubeDataConfigurator.isovalue->setRange(-99.999, 99.9999);
   }
}


void CubeData::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_cubeDataConfigurator.positiveColorButton->setStyleSheet(bg);
      Preferences::PositiveSurfaceColor(color);
   }
}


void CubeData::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_cubeDataConfigurator.negativeColorButton->setStyleSheet(bg);
      Preferences::NegativeSurfaceColor(color);
   }
}


void CubeData::on_cancelButton_clicked(bool)
{ 
   reject(); 
}


void CubeData::on_calculateButton_clicked(bool)
{ 
   accept();
   double isoValue(m_cubeDataConfigurator.isovalue->value());
   QColor positive(Preferences::PositiveSurfaceColor());
   QColor negative(Preferences::NegativeSurfaceColor());

   Grid::DataType data(Grid::DataType::CubeData);
   Layer::Surface* surface(new Layer::Surface(data, -1, isoValue, positive, negative)); 
   surface->setCubeIsSigned(m_cubeDataConfigurator.signedButton->checkState() == Qt::Checked);
   calculateSurface(surface);
}

} } // end namespace IQmol::Configurator
