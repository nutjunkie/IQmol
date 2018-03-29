#ifndef IQMOL_CONFIGURATOR_ISOTOPES_H
#define IQMOL_CONFIGURATOR_ISOTOPES_H
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
#include "ui_IsotopesConfigurator.h"


class QComboBox;

namespace IQmol {

namespace Layer {
   class Isotopes;
   class Atom;
}

namespace Configurator {

   class Isotopes : public Base {

      Q_OBJECT

      public:
         Isotopes(Layer::Isotopes& isotopes);

         void loadTable(QList<Layer::Atom*> const& atomicNumbers);
         QString toString() const;
         QMap<unsigned, double> makeMassList() const;

      private:
         Ui::IsotopesConfigurator m_configurator;
         Layer::Isotopes& m_isotopes;

         void loadMasses(unsigned Z, QComboBox*);
   };

} } // end namespace IQmol::Configurator

#endif
