#ifndef IQMOL_BACKGROUNDCONFIGURATOR_H
#define IQMOL_BACKGROUNDCONFIGURATOR_H
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

#include "Configurator.h"
#include "ui_BackgroundConfigurator.h"
#include <QColor>


namespace IQmol {

namespace Layer {
   class Background;
}

namespace Configurator {

   /// Configurator Dialog which allows the user to select the color of the
   /// Viewer background.
   class Background : public Base {

      Q_OBJECT

      public:
         explicit Background(Layer::Background& background);

      private Q_SLOTS:
         void on_backgroundColorButton_clicked(bool);
         void on_foregroundColorButton_clicked(bool);
         void on_applyButton_clicked(bool);
         void on_okButton_clicked(bool);

      private:
         Ui::BackgroundConfigurator m_backgroundConfigurator;
         Layer::Background& m_background;
         QColor m_backgroundColor;
         QColor m_foregroundColor;
   };

} } // end namespace IQmol::Configurator

#endif
