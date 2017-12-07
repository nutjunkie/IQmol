#ifndef IQMOL_CONFIGURATOR_AXES_H
#define IQMOL_CONFIGURATOR_AXES_H
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
#include "ui_AxesConfigurator.h"


namespace IQmol {

namespace Layer {
   class Axes;
}

namespace Configurator {

   /// Dialog that allows the user to change the appearance of the axes.
   class Axes: public Base {

      Q_OBJECT

      friend class Layer::Axes;

      public:
         explicit Axes(Layer::Axes& axes);
         void sync();

      private Q_SLOTS:
         void on_xCheckBox_clicked(bool);
         void on_yCheckBox_clicked(bool);
         void on_zCheckBox_clicked(bool);
         void on_scaleSlider_valueChanged(int);
         void on_okButton_clicked(bool);
   
      private:
         Ui::AxesConfigurator m_axesConfigurator;
         Layer::Axes& m_axes;
   };

} } // end namespace IQmol::Configurator

#endif
