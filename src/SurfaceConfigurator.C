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

#include "IQmol.h"
#include "QsLog.h"
#include "SurfaceConfigurator.h"
#include "SurfaceLayer.h"
#include "MoleculeLayer.h"
#include <openbabel/mol.h>
#include <QColorDialog>


using namespace qglviewer;

namespace IQmol {
namespace Configurator {

Gradient::ColorList Surface::s_atomColors;

Surface::Surface(Layer::Surface* surface) : m_surface(surface), 
   m_colors(Preferences::DefaultGradientColors()), m_initialized(false)
{        
   m_surfaceConfigurator.setupUi(this);
   m_surfaceConfigurator.ambientOcclusionCheckBox->setVisible(false);
}


// Should only be called when the surface is constructed
void Surface::sync()
{
   if (m_initialized) return;
   m_initialized = true;

   m_colorPositive = m_surface->color(Layer::Surface::Positive);
   m_colorNegative = m_surface->color(Layer::Surface::Negative);

   setPositiveColor(m_colorPositive);
   setNegativeColor(m_colorNegative);

   if (!m_surface->gridDataType().isSigned()) {
      m_surfaceConfigurator.negativeColorButton->setVisible(false);
      m_surfaceConfigurator.negativeLabel->setVisible(false);
      m_surfaceConfigurator.swapColorsButton->setEnabled(false);
   }
           
   m_surfaceConfigurator.transparencySlider->setValue(100*m_surface->getAlpha());
   
   switch (m_surface->m_drawMode) {
      case Layer::Surface::Fill:
           m_surfaceConfigurator.fillButton->setChecked(true);
         break;
      case Layer::Surface::Lines:
           m_surfaceConfigurator.linesButton->setChecked(true);
         break;
      case Layer::Surface::Dots:
           m_surfaceConfigurator.dotsButton->setChecked(true);
         break;
   }  

   QList<Layer::Molecule*> parents = m_surface->findLayers<Layer::Molecule>(Layer::Parents);
   if (parents.isEmpty()) {
      QLOG_ERROR() << "Could not find Molecule parent for surface";
   }else {
      m_surfaceConfigurator.propertyCombo->addItems(parents.first()->getAvailableProperties());
   }
}  


void Surface::on_propertyCombo_currentIndexChanged(int)
{
   QString type(m_surfaceConfigurator.propertyCombo->currentText());
   disconnect(m_surfaceConfigurator.positiveColorButton, 0, 0, 0);

   if (type == "None") {
      m_surfaceConfigurator.positiveColorButton->setProperty("gradient",false);
      m_surfaceConfigurator.negativeLabel->setVisible(false);
      m_surfaceConfigurator.positiveLabel->setText("Positive");
      setPositiveColor(m_colorPositive);
      connect(m_surfaceConfigurator.positiveColorButton, SIGNAL(clicked(bool)),
         this, SLOT(on_positiveColorButton_clicked(bool)));

      if (m_surface->gridDataType().isSigned()) {
         m_surfaceConfigurator.negativeColorButton->setVisible(true);
         m_surfaceConfigurator.negativeLabel->setText("Negative");
         m_surfaceConfigurator.swapColorsButton->setEnabled(true);
      }
 
      m_surface->clearPropertyData();
      m_surface->updated();

   }else if (type == "Nearest nucleus") {

      QList<Layer::Molecule*> parents = m_surface->findLayers<Layer::Molecule>(Layer::Parents);
      if (parents.isEmpty()) {
         QLOG_ERROR() << "No Molecule found";
      }else {
         m_surface->setGradient(atomColorGradient());
         m_surface->computePropertyData(parents.first()->getPropertyEvaluator(type));
         m_surface->updated();
      }

   }else {
      m_surfaceConfigurator.positiveColorButton->setProperty("gradient",true);
      setPositiveColor(m_colors);
      connect(m_surfaceConfigurator.positiveColorButton, SIGNAL(clicked(bool)),
         this, SLOT(editGradientColors(bool)));

      m_surfaceConfigurator.negativeLabel->setVisible(true);
      m_surfaceConfigurator.negativeColorButton->setVisible(false);
      m_surfaceConfigurator.swapColorsButton->setEnabled(false);

      QList<Layer::Molecule*> parents = m_surface->findLayers<Layer::Molecule>(Layer::Parents);
      if (parents.isEmpty()) {
         QLOG_ERROR() << "No Molecule found";
      }else {
         m_surface->setGradient(m_colors);
         m_surface->computePropertyData(parents.first()->getPropertyEvaluator(type));
         m_surface->updated();
      }
      QString v1(QString::number(m_surface->minPropertyValue(), 'f', 4));
      QString v2(QString::number(m_surface->maxPropertyValue(), 'f', 4));
      m_surfaceConfigurator.negativeLabel->setText(v1);
      m_surfaceConfigurator.positiveLabel->setText(v2);
   }
}


Gradient::ColorList const& Surface::atomColorGradient()
{
   if (s_atomColors.isEmpty()) {
      QColor color;
      for (unsigned int Z = 1; Z <= 9; ++Z) {
          std::vector<double> rgb(OpenBabel::etab.GetRGB(Z));
          color.setRgbF(rgb[0],rgb[1],rgb[2],1.0);
          s_atomColors.append(color);
      }
   }

   return s_atomColors;
}

   
void Surface::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(m_colorPositive, this));
   m_surface->updated();
}     
      
   
void Surface::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(m_colorNegative, this));
   m_surface->updated();
}


void Surface::on_swapColorsButton_clicked(bool)
{
   QColor tmp(m_surface->color(Layer::Surface::Positive));
   setPositiveColor(m_surface->color(Layer::Surface::Negative));
   setNegativeColor(tmp);
   m_surface->updated();
}
   


void Surface::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_surfaceConfigurator.positiveColorButton->setStyleSheet(bg);
      m_surface->setColor(color, Layer::Surface::Positive);
      m_colorPositive = color;
   }
}


void Surface::editGradientColors(bool)
{
   setPositiveColor(GetGradient(m_colors, this)); 
   m_surface->recompile();
   m_surface->updated();
}


void Surface::setPositiveColor(Gradient::ColorList const& colors)
{
   m_colors = colors;
   QString bg("background-color: ");
   bg += Gradient::ToString(m_colors);
   m_surfaceConfigurator.positiveColorButton->setStyleSheet(bg);
   m_surface->setGradient(m_colors);
}


void Surface::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_surfaceConfigurator.negativeColorButton->setStyleSheet(bg);
      m_surface->setColor(color, Layer::Surface::Negative);
      m_colorNegative = color;
   }
}


void Surface::on_transparencySlider_valueChanged(int value)
{
   m_surface->setAlpha(value/100.0);
   m_surface->updated();
}


void Surface::on_fillButton_clicked(bool)
{
   m_surface->m_drawMode = Layer::Surface::Fill;
   m_surface->updated();
}


void Surface::on_linesButton_clicked(bool)
{
   m_surface->m_drawMode = Layer::Surface::Lines;
   m_surface->updated();
}


void Surface::on_dotsButton_clicked(bool)
{
   m_surface->m_drawMode = Layer::Surface::Dots;
   m_surface->updated();
}


void Surface::on_ambientOcclusionCheckBox_clicked(bool tf)
{
   m_surface->addAmbientOcclusion(tf);
}


void Surface::setArea(double const area)
{
   m_surfaceConfigurator.areaLabel->setText(QString::number(area, 'f', 3));
}

} } // end namespace IQmol::Configurator
