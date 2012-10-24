#ifndef IQMOL_INFOCONFIGURATOR_H
#define IQMOL_INFOCONFIGURATOR_H
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
#include "ui_InfoConfigurator.h"


namespace IQmol {

namespace Layer {
   class Info;
}

namespace Configurator {

   /// Configurator Dialog which displays information about the Molecule
   /// such as the number of atoms, charge and multiplicity.
   class Info : public Base {

      Q_OBJECT

      public:
         explicit Info(Layer::Info* info);
      
      public Q_SLOTS:
         void sync();
         void on_chargeSpin_valueChanged(int);
         void on_multiplicitySpin_valueChanged(int);

      private:
         Ui::InfoConfigurator m_infoConfigurator;
         Layer::Info* m_info;
   };

} } // end namespace IQmol::Configurator

#endif
