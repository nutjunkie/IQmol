#ifndef IQMOL_CONFIGURATOR_EFPFRAGMENTLIST_H
#define IQMOL_CONFIGURATOR_EFPFRAGMENTLIST_H
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

#include "Configurator.h"
#include "ui_EfpFragmentListConfigurator.h"


namespace IQmol {

namespace Layer {
   class EfpFragmentList;
}

namespace Configurator {

   /// Configuration Dialog which allows the user to change the appearance
   /// of the EFP Fragments.
   class EfpFragmentList : public Base {

      Q_OBJECT

      public:
         explicit EfpFragmentList(Layer::EfpFragmentList& efpFragmentList);

      private Q_SLOTS:
         void on_resetButton_clicked(bool);
         void on_ballsAndSticks_clicked(bool);
         void on_tubes_clicked(bool);
         void on_spaceFilling_clicked(bool);
         void on_wireFrame_clicked(bool);
         void on_atomRadiusScale_valueChanged(int);
         void on_bondRadiusScale_valueChanged(int);

      private:
         Ui::EfpFragmentListConfigurator m_efpFragmentListConfigurator;
         Layer::EfpFragmentList& m_efpFragmentList;
   };

} } // end namespace IQmol::Configurator

#endif
