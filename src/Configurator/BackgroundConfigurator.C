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

#include "SetButtonColor.h"
#include "BackgroundConfigurator.h"
#include "BackgroundLayer.h"
#include "Preferences.h"
#include <QColorDialog>


namespace IQmol {
namespace Configurator {

Background::Background(Layer::Background& background) : m_background(background), 
   m_backgroundColor(m_background.m_backgroundColor),
   m_foregroundColor(m_background.m_foregroundColor)
{
   m_backgroundConfigurator.setupUi(this);
   Util::SetButtonColor(*(m_backgroundConfigurator.backgroundColorButton), m_backgroundColor);
   Util::SetButtonColor(*(m_backgroundConfigurator.foregroundColorButton), m_foregroundColor);
}


/*
function smarty_modifier_contrast($hexcolor, $dark = '#000000', $light = '#FFFFFF')
{
    return (hexdec($hexcolor) > 0xffffff/2) ? $dark : $light;
}
*/


void Background::on_backgroundColorButton_clicked(bool)
{
   m_backgroundColor = QColorDialog::getColor(m_backgroundColor, this);
   Util::SetButtonColor(*(m_backgroundConfigurator.backgroundColorButton), m_backgroundColor);

   // determine whether black or white gives better contrast.
   int r(m_backgroundColor.red());
   int g(m_backgroundColor.green());
   int b(m_backgroundColor.blue());
   int yiq(((r*299)+(g*587)+(b*114))/1000);
   // softer whites and blacks look better, but don't go for a mid-gray
   // background unless you want eye strain.
   QColor dark(33,33,33);
   QColor light(170,170,170);

   m_foregroundColor = (yiq >= 128) ? dark : light;
   Util::SetButtonColor(*(m_backgroundConfigurator.foregroundColorButton), m_foregroundColor);
}


void Background::on_foregroundColorButton_clicked(bool)
{
   m_foregroundColor = QColorDialog::getColor(m_foregroundColor, this);
   Util::SetButtonColor(*(m_backgroundConfigurator.foregroundColorButton), m_foregroundColor);
}


void Background::on_applyButton_clicked(bool)
{
   m_background.foregroundColorChanged(m_foregroundColor);
   m_background.m_backgroundColor = m_backgroundColor;
   m_background.m_foregroundColor = m_foregroundColor;
   m_background.updated();
   m_background.updated();  // Apply doesn't seem to take effect with one update.
}


void Background::on_okButton_clicked(bool)
{
   close();
   on_applyButton_clicked(true);
   if (m_backgroundConfigurator.makeDefaultButton->isChecked()) {
      Preferences::BackgroundColor(m_backgroundColor); 
      Preferences::ForegroundColor(m_foregroundColor); 
   }
}

} } // end namespace IQmol::Configurator
