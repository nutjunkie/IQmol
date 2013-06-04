/*******************************************************************************
       
  Copyright (C) 2011-13 Andrew Gilbert
           
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

#include "ZMatrixCoordinatesParser.h"
#include "Geometry.h"
#include "openbabel/obconversion.h"
#include "openbabel/format.h"
#include "openbabel/mol.h"

#include <QDebug>

using namespace OpenBabel;
using namespace qglviewer;

namespace IQmol {
namespace Parser2 {

Data::Geometry* ZMatrixCoordinates::parse(QString const& str)
{
   OBConversion conv;
   conv.SetInFormat("gzmat");
   // create dummy z-matrix input
   std::string s("#\n\nzmat\n\n0  1\n");
   s += str.toStdString();

   //OpenBabel::OBMol* mol(new OpenBabel::OBMol);
   OBMol mol;
   std::istringstream iss(s);
   conv.Read(&mol, &iss);

   Data::Geometry* geometry(new Data::Geometry);

   FOR_ATOMS_OF_MOL(obAtom, &mol) {
      geometry->append(obAtom->GetAtomicNum(), Vec(obAtom->x(), obAtom->y(), obAtom->z()));
   }

   //delete mol;

   return geometry;
}

} } // end namespace IQmol::Parser
