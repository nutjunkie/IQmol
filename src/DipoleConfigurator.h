#ifndef IQMOL_DIPOLECONFIGURATOR_H
#define IQMOL_DIPOLECONFIGURATOR_H
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

#include "BaseConfigurator.h"
#include "ui_DipoleConfigurator.h"


namespace IQmol {

namespace Layer {
   class Dipole;
}

namespace Configurator {

   /// Configurator Dialog which allows the user to change the display
   /// options for the molecular dipole.
   class Dipole : public Base {

      Q_OBJECT

      public:
         explicit Dipole(Layer::Dipole* dipole);
         void sync();
      
      public Q_SLOTS:
         void on_scaleSlider_valueChanged(int);
         void on_colorButton_clicked(bool);

      private:
         Ui::DipoleConfigurator m_dipoleConfigurator;
         Layer::Dipole* m_dipole;
   };

} } // end namespace IQmol::Configurator

#endif
