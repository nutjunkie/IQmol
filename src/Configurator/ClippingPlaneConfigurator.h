#ifndef IQMOL_CONFIGURATOR_CLIPPINGPLANE_H
#define IQMOL_CONFIGURATOR_CLIPPINGPLANE_H
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
#include "ui_ClippingPlaneConfigurator.h"


namespace IQmol {

namespace Layer {
   class ClippingPlane;
}

namespace Configurator {

   /// Dialog that allows the user to change the appearance of the mesh.
   class ClippingPlane : public Base {

      Q_OBJECT

      public:
         explicit ClippingPlane(Layer::ClippingPlane&);
         void sync();

      private Q_SLOTS:
         void on_okButton_clicked(bool);

         void on_xValue_valueChanged(double);
         void on_yValue_valueChanged(double);
         void on_zValue_valueChanged(double);
         void on_thetaValue_valueChanged(int);
         void on_phiValue_valueChanged(int);
   
      private:
         void syncFromDialog();
         Ui::ClippingPlaneConfigurator m_configurator;
         Layer::ClippingPlane& m_clippingPlane;
   };

} } // end namespace IQmol::Configurator

#endif
