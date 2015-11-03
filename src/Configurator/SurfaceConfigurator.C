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
#include <openbabel/mol.h>  // for etab
#include <QColorDialog>


using namespace qglviewer;

namespace IQmol {
namespace Configurator {

Surface::Surface(Layer::Surface& surface) : m_surface(surface), 
   m_gradientColors(Preferences::DefaultGradientColors()), m_initialized(false)
{        
   m_surfaceConfigurator.setupUi(this);
   m_surfaceConfigurator.ambientOcclusionCheckBox->setVisible(false);
   m_surfaceConfigurator.scaleButton->setEnabled(false);
}


// Should only be called when the surface is constructed
void Surface::sync()
{
   if (m_initialized) return;
   m_initialized = true;

   setPositiveColor(m_surface.colorPositive());
   setNegativeColor(m_surface.colorNegative());

   if (!m_surface.isSigned()) {
      m_surfaceConfigurator.negativeColorButton->setVisible(false);
      m_surfaceConfigurator.negativeLabel->setVisible(false);
      m_surfaceConfigurator.swapColorsButton->setEnabled(false);
   }

   if (m_surface.hasProperty()) {
      m_gradientColors = m_surface.colors();
      m_surfaceConfigurator.scaleButton->setEnabled(m_surface.propertyIsSigned());
   }

   double area(m_surface.area());
   m_surfaceConfigurator.areaLabel->setText(QString::number(area, 'f', 3));

   m_surfaceConfigurator.transparencySlider->setValue(100*m_surface.getAlpha());
   
   switch (m_surface.m_drawMode) {
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

   MoleculeList parents(m_surface.findLayers<Layer::Molecule>(Layer::Parents));
   if (parents.isEmpty()) {
      QLOG_ERROR() << "Could not find Molecule parent for surface";
   }else {
      QStringList properties(parents.first()->getAvailableProperties());
      // remove Nuclei for non-vdW surfaces 
      if (!m_surface.isVdW()) {
         properties.removeAll("Nuclei");
      }
      m_surfaceConfigurator.propertyCombo->addItems(properties);
   }
}  


void Surface::on_propertyCombo_currentIndexChanged(int)
{

   QString type(m_surfaceConfigurator.propertyCombo->currentText());
qDebug() << "Changing property combo" << type;
   disconnect(m_surfaceConfigurator.positiveColorButton, 0, 0, 0);

   if (type == "None") {
      m_surfaceConfigurator.positiveColorButton->setProperty("gradient",false);
      m_surfaceConfigurator.negativeLabel->setVisible(false);
      m_surfaceConfigurator.positiveLabel->setText("Positive");
      m_surfaceConfigurator.scaleButton->setEnabled(false);
      setPositiveColor(m_surface.colorPositive());
      connect(m_surfaceConfigurator.positiveColorButton, SIGNAL(clicked(bool)),
         this, SLOT(on_positiveColorButton_clicked(bool)));

      if (m_surface.isSigned()) {
         m_surfaceConfigurator.negativeColorButton->setVisible(true);
         m_surfaceConfigurator.negativeLabel->setText("Negative");
         m_surfaceConfigurator.swapColorsButton->setEnabled(true);
      }
 
      m_surface.clearPropertyData();

   }else if (type == "Nuclei") {

      QList<Layer::Molecule*> parents = m_surface.findLayers<Layer::Molecule>(Layer::Parents);
      if (parents.isEmpty()) {
         QLOG_ERROR() << "No Molecule found";
      }else {
         m_surface.setColors(atomColorGradient(parents.first()->maxAtomicNumber()));
         m_surface.computeIndexField();
         updateScale();
      }

   }else {
      m_surfaceConfigurator.positiveColorButton->setProperty("gradient",true);
      setPositiveColor(m_gradientColors);

      connect(m_surfaceConfigurator.positiveColorButton, SIGNAL(clicked(bool)),
         this, SLOT(editGradientColors(bool)));

      m_surfaceConfigurator.negativeLabel->setVisible(true);
      m_surfaceConfigurator.negativeColorButton->setVisible(false);
      m_surfaceConfigurator.swapColorsButton->setEnabled(false);

      QList<Layer::Molecule*> parents(m_surface.findLayers<Layer::Molecule>(Layer::Parents));

      if (parents.isEmpty()) {
         QLOG_ERROR() << "No Molecule found";
      }else {
         m_surface.setColors(m_gradientColors);
qDebug() << "Computing propert data";
         m_surface.computePropertyData(parents.first()->getPropertyEvaluator(type));
qDebug() << "Computing propert data finished";
      }

      m_surfaceConfigurator.scaleButton->setEnabled(m_surface.propertyIsSigned());

      updateScale();
   }

   m_surface.updated();
}

  
void Surface::on_positiveColorButton_clicked(bool)
{
   QColor color(m_surface.colorPositive());
   setPositiveColor(QColorDialog::getColor(color, this));
   m_surface.updated();
}     
      
   
void Surface::on_negativeColorButton_clicked(bool)
{
   QColor color(m_surface.colorNegative());
   setNegativeColor(QColorDialog::getColor(color, this));
   m_surface.updated();
}


void Surface::on_scaleButton_clicked(bool tf)
{
   m_surface.balanceScale(tf);
   updateScale();
}


void Surface::updateScale()
{
   double min, max;
   m_surface.getPropertyRange(min, max);
   QString v1(QString::number(min, 'f', 4));
   QString v2(QString::number(max, 'f', 4));

   m_surfaceConfigurator.negativeLabel->setText(v1);
   m_surfaceConfigurator.positiveLabel->setText(v2);
}


void Surface::on_swapColorsButton_clicked(bool)
{
   QColor positive(m_surface.colorPositive());
   QColor negative(m_surface.colorNegative());
   setPositiveColor(negative);
   setNegativeColor(positive);
   m_surface.updated();
}
   

ColorGradient::ColorList Surface::atomColorGradient(unsigned const maxAtomicNumber)
{
   ColorGradient::ColorList atomColors;
   QColor color;
   for (unsigned int Z = 1; Z <= maxAtomicNumber; ++Z) {
       std::vector<double> rgb(OpenBabel::etab.GetRGB(Z));
       color.setRgbF(rgb[0],rgb[1],rgb[2],1.0);
       atomColors.append(color);
   }

/*
   setPositiveColor(atomColors);
   m_surface.recompile();
   m_surface.updated();
*/

   return atomColors;
}

 
void Surface::editGradientColors(bool)
{
   QList<QColor> colors(m_gradientColors);
   setPositiveColor(GetGradient(colors, this)); 
   m_surface.recompile();
   m_surface.updated();
}


void Surface::setPositiveColor(QList<QColor> const& colors)
{
   QString bg("background-color: ");
   bg += ColorGradient::ToString(colors);
   m_surfaceConfigurator.positiveColorButton->setStyleSheet(bg);
   m_surface.setColors(colors);
}


void Surface::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_surfaceConfigurator.positiveColorButton->setStyleSheet(bg);

      QColor negative(m_surface.colorNegative());
      m_surface.setColors(negative, color);
   }
}


void Surface::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_surfaceConfigurator.negativeColorButton->setStyleSheet(bg);

      QColor positive(m_surface.colorPositive());
      m_surface.setColors(color, positive);
   }
}


void Surface::setArea(double const area)
{
   m_surfaceConfigurator.areaLabel->setText(QString::number(area, 'f', 3));
}


void Surface::on_transparencySlider_valueChanged(int value)
{
   m_surface.setAlpha(value/100.0);
   m_surface.updated();
}


void Surface::on_fillButton_clicked(bool)
{
   m_surface.setDrawMode(Layer::Surface::Fill);
   m_surface.updated();
}


void Surface::on_linesButton_clicked(bool)
{
   m_surface.setDrawMode(Layer::Surface::Lines);
   m_surface.updated();
}


void Surface::on_dotsButton_clicked(bool)
{
   m_surface.setDrawMode(Layer::Surface::Dots);
   m_surface.updated();
}

} } // end namespace IQmol::Configurator
