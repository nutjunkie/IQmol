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

#include "ColorGradientDialog.h"
#include <QColorDialog>
#include <cmath>


namespace IQmol {
namespace ColorGradient {

Dialog::Dialog(ColorList const& colors, QWidget* parent) : QDialog(parent),  m_colors(colors)
{
   m_dialog.setupUi(this);
   if (m_colors == Preferences::DefaultGradientColors()) {
      m_dialog.gradientCombo->setCurrentIndex(Default);
   }else if (m_colors == Preferences::SpectrumGradientColors()) {
      m_dialog.gradientCombo->setCurrentIndex(Spectrum);
   }else {
      m_dialog.gradientCombo->setCurrentIndex(Custom);
   }
   updateGradient();
}


Dialog::Dialog(StandardGradient const grad, QWidget* parent) : QDialog(parent)
{
   m_dialog.setupUi(this);
   switch (grad) {
      case Default:
         m_colors = Preferences::DefaultGradientColors();      
         m_dialog.gradientCombo->setCurrentIndex(Default);
         break;
      case Spectrum:
         m_colors = Preferences::SpectrumGradientColors();      
         m_dialog.gradientCombo->setCurrentIndex(Spectrum);
         break;
      case Custom:
         m_colors = Preferences::CustomGradientColors();      
         m_dialog.gradientCombo->setCurrentIndex(Custom);
         break;
   }
   updateGradient();
}


void Dialog::updateGradient()
{
   clearStops();

   int nColors(m_colors.size());
   QHBoxLayout* layout(m_dialog.buttonLayout);
   QSpacerItem* spacer;
   StopButton*  button;

   for (int i = 0; i < nColors-1; ++i) {
       button = new StopButton(this, i, m_colors[i]);
       connect(button, SIGNAL(colorChanged(int const, QColor const&)), 
          this, SLOT(colorChanged(int const, QColor const&)));
       layout->addWidget(button);
       spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
       layout->addItem(spacer);
   }

   button = new StopButton(this, nColors-1, m_colors.last());
   connect(button, SIGNAL(colorChanged(int const, QColor const&)), 
      this, SLOT(colorChanged(int const, QColor const&)));
   layout->addWidget(button);
  
   QString grad("background-color: " + ToString(m_colors));
   m_dialog.gradientView->setStyleSheet(grad);
   m_dialog.stopsSpin->setValue(m_colors.size());
}


void Dialog::clearStops()
{
   QHBoxLayout* layout(m_dialog.buttonLayout);
   QLayoutItem* item;
   while (layout->count() > 0) {
      item = layout->takeAt(0);
      if (item->widget()) delete item->widget();
   }
}


void Dialog::on_gradientCombo_currentIndexChanged(int n)
{
   switch (n) {
      case Default:
         m_colors = Preferences::DefaultGradientColors();
         break;
      case Spectrum:
         m_colors = Preferences::SpectrumGradientColors();
         break;
      case Custom:
         m_colors = Preferences::CustomGradientColors();
         break;
      default:
         break;
   }

   updateGradient();
}


void Dialog::on_stopsSpin_valueChanged(int n)
{
   if (n != m_colors.size()) {
      Function f(m_colors);
      f.resample(n);
      m_colors = f.colors();
      setCustom();
      updateGradient();
   }
}


StandardGradient Dialog::type() const 
{
   if (m_dialog.gradientCombo->currentIndex() == Default) {
      return Default;
   }else if (m_dialog.gradientCombo->currentIndex() == Spectrum) {
      return Spectrum;
   }
   return Custom;
}



void Dialog::colorChanged(int const index, QColor const& color)
{
   if (0 <= index && index < m_colors.size()) {
      m_colors[index] = color;
      setCustom();
      updateGradient();
   }
}


void Dialog::setCustom()
{
   disconnect(m_dialog.gradientCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(on_gradientCombo_currentIndexChanged(int)));
   m_dialog.gradientCombo->setCurrentIndex(Custom);
   connect(m_dialog.gradientCombo, SIGNAL(currentIndexChanged(int)),
      this, SLOT(on_gradientCombo_currentIndexChanged(int)));
}


// --------------- StopButton ----------------

StopButton::StopButton(QWidget* parent, int const index, QColor const& color) 
   : QToolButton(parent), m_index(index)
{
   setColor(color);
   connect(this, SIGNAL(clicked()), this, SLOT(getNewColor()));
}


void StopButton::setColor(QColor const& color) 
{
   m_color = color;
   QString bg("background-color: ");
   bg += m_color.name();
   setStyleSheet(bg);
}


void StopButton::getNewColor()
{
   QColor tmp(QColorDialog::getColor(m_color, this));
   if (tmp.isValid()) {
      setColor(tmp);
      colorChanged(m_index, m_color);
   }
}


}  // end namespace ColorGradient


ColorGradient::ColorList GetGradient(ColorGradient::StandardGradient grad, QWidget* parent)
{
   ColorGradient::Dialog dialog(grad, parent);
   dialog.exec();
   if (dialog.type() == ColorGradient::Custom) {
       Preferences::CustomGradientColors(dialog.colors());
   }
   return dialog.colors();
}


ColorGradient::ColorList GetGradient(ColorGradient::ColorList const& colors, QWidget* parent) 
{
   ColorGradient::Dialog dialog(colors, parent);
   if (dialog.exec() == QDialog::Rejected) return colors;
   if (dialog.type() == ColorGradient::Custom) {
       Preferences::CustomGradientColors(dialog.colors());
   }
   return dialog.colors();
}


} // end namespace IQmol
