#ifndef IQMOL_LAYER_GEOMETRY_H
#define IQMOL_LAYER_GEOMETRY_H
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
#include "QGLViewer/vec.h"


namespace IQmol {

namespace Data {
   class Geometry;
}

namespace Layer {

   class Geometry : public Base {

      Q_OBJECT

      public:
         Geometry(Data::Geometry&);

         qglviewer::Vec dipole() const;
         unsigned nAtoms() const;
         double energy() const;
         QString label() const;
// temp hack until these layers are re-engineered to match the Nmr model
         Data::Geometry& data() { return m_geometry; }

         qglviewer::Vec atomicPosition(unsigned i) const;
         double atomicCharge(unsigned i) const;
         double atomicSpin(unsigned i) const;

      private:
         Data::Geometry& m_geometry;
   };

} } // end namespace IQmol::Layer

#endif
