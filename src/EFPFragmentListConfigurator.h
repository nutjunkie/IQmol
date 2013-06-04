#ifndef IQMOL_EFPFRAGMENTLISTCONFIGURATOR_H
#define IQMOL_EFPFRAGMENTLISTCONFIGURATOR_H
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
#include "ui_EFPFragmentListConfigurator.h"


namespace IQmol {

namespace Layer {
   class EFPFragmentList;
}

namespace Configurator {

   /// Configuration Dialog which allows the user to change the appearance
   /// of the EFP Fragments.
   class EFPFragmentList : public Base {

      Q_OBJECT

      public:
         EFPFragmentList(Layer::EFPFragmentList* efpFragmentList);

      private Q_SLOTS:
         void on_resetButton_clicked(bool);
         void on_ballsAndSticks_clicked(bool);
         void on_tubes_clicked(bool);
         void on_spaceFilling_clicked(bool);
         void on_wireFrame_clicked(bool);
         void on_atomRadiusScale_valueChanged(int);
         void on_bondRadiusScale_valueChanged(int);

      private:
         Ui::EFPFragmentListConfigurator m_efpFragmentListConfigurator;
         Layer::EFPFragmentList* m_efpFragmentList;
   };

} } // end namespace IQmol::Configurator

#endif
