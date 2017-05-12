#ifndef IQMOL_LAYER_EXCITEDSTATES_H
#define IQMOL_LAYER_EXCITEDSTATES_H
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

#include "Layer.h"


namespace IQmol {

namespace Configurator {
   class ExcitedStates;
}

namespace Data {
   class ExcitedStates;
   class OrbitalSymmetries;
}


namespace Layer {

   class ExcitedStates: public Base {

      Q_OBJECT 

      public:
         ExcitedStates(Data::ExcitedStates const&);
         ~ExcitedStates();

         Data::ExcitedStates const& stateData() const { return m_excitedStates; }
      
         void configure();

      Q_SIGNALS:
         void update();

      private:
         Data::ExcitedStates const& m_excitedStates;
         Configurator::ExcitedStates* m_configurator;
   };

} } // end namespace IQmol::Layer

#endif
