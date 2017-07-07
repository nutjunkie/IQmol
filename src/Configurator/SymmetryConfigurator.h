#ifndef IQMOL_CONFIGURATOR_SYMMETRY_H
#define IQMOL_CONFIGURATOR_SYMMETRY_H
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
#include "ui_SymmetryConfigurator.h"


namespace IQmol {

namespace Layer {
   class Symmetry;
}

namespace Configurator {

   class Symmetry: public Base {

      Q_OBJECT

      public:
         explicit Symmetry(Layer::Symmetry&);
         void sync();
      
      public Q_SLOTS:

      private:
         Ui::SymmetryConfigurator m_configurator;
         Layer::Symmetry& m_symmetry;
   };

} } // end namespace IQmol::Configurator

#endif
