#ifndef IQMOL_LAYER_ISOTOPES_H
#define IQMOL_LAYER_ISOTOPES_H
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

#include "IsotopesConfigurator.h"
#include "AtomLayer.h"
#include "openbabel/data.h"


namespace IQmol {
namespace Layer {

   class Isotopes : public Base {

      Q_OBJECT

      public:
		 Isotopes(AtomList const& atoms);

         QString const& formatQChem() const { return m_qchem; }
         bool accepted() const { return m_accepted; }
         void updateLabels();

      Q_SIGNALS:

      public Q_SLOTS:
         void configure();

      private:
         void updateLabels(QMap<unsigned, double> const& masses);

         Configurator::Isotopes m_configurator;
         QString m_qchem;
         bool m_accepted;
   };

} } // end namespace IQmol::Layer

#endif
