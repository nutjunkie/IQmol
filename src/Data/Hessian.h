#ifndef IQMOL_DATA_HESSIAN_H
#define IQMOL_DATA_HESSIAN_H
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

#include "Data.h"
#include "Matrix.h"


namespace IQmol {
namespace Data {

   class Hessian : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::Hessian; }

         Hessian(unsigned const nAtoms = 0, QList<double> const& hessian = QList<double>());

         void setData(unsigned const nAtoms, QList<double> const& hessian);
         void setData(Matrix const&);
         void setPartialData(unsigned const nAtoms, QList<unsigned> const& atomList, 
            Matrix const& partialHhessian);

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void dump() const;

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_hessian;
         }

         Matrix m_hessian;
   };


} } // end namespace IQmol::Data

#endif
