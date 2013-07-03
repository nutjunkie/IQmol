#ifndef IQMOL_MESHCONFIGURATOR_H
#define IQMOL_MESHCONFIGURATOR_H
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

#include "BaseConfigurator.h"
#include "ui_MeshConfigurator.h"


namespace IQmol {

namespace Layer {
   class Mesh;
}

namespace Configurator {

   /// Dialog that allows the user to change the appearance of the mesh.
   class Mesh : public Base {

      Q_OBJECT

      public:
         explicit Mesh(Layer::Mesh* mesh);
         void sync();

      private Q_SLOTS:
         void on_xyCheckBox_clicked(bool);
         void on_xzCheckBox_clicked(bool);
         void on_yzCheckBox_clicked(bool);
         void on_ticksSpinBox_valueChanged(int);
         void on_stepSpinBox_valueChanged(double);
         void on_polarButton_clicked(bool);
         void on_cartesianButton_clicked(bool);
         void on_totalButton_clicked(bool);
         void on_stepButton_clicked(bool);
   
      private:
         Ui::MeshConfigurator m_meshConfigurator;
         Layer::Mesh* m_mesh;
   };

} } // end namespace IQmol::Configurator

#endif
