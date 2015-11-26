#ifndef IQMOL_DATA_MULTIPOLEEXPANSION_H
#define IQMOL_DATA_MULTIPOLEEXPANSION_H
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

#include "DataList.h"


namespace IQmol {
namespace Data {

   /// Data structure representing a set of mulitpoles at a location.
   /// Currently the multipoles are Buckingham traceless form.
   class MultipoleExpansion : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::MultipoleExpansion; }

         enum Index { Q = 0, 
                      X, Y, Z, 
                      XX, XY, XZ, YY, YZ, ZZ,
                      XXX, XXY, XXZ, XYY, XYZ, XZZ, YYY, YYZ, YZZ, ZZZ };
                       
         MultipoleExpansion(qglviewer::Vec const& position = qglviewer::Vec()) 
           : m_order(-1), m_position(position) { }


         double moment(Index const index) { 
            return (index < m_multipoles.size()) ? m_multipoles[index] : 0.0;
         }

		 // The input for these are the Buckingham traceless values,  The order is 
         // that which QChem prints them out, for better or worse.
         MultipoleExpansion& addCharge(double const q);
         MultipoleExpansion& addDipole(double const x, double const y, double const z);
         MultipoleExpansion& addQuadrupole(
            double const xx, double const xy, double const xz, 
            double const yy, double const yz, double const zz);
         MultipoleExpansion& addOctopole(
            double const xxx, double const xxy, double const xxz, double const xyy, 
            double const xyz, double const xzz, double const yyy, double const yyz,  
            double const yzz, double const zzz);

         int order() const { return m_order; }
         qglviewer::Vec const& position() const { return m_position; }
         void dump() const;

         // These assume the centres are the same.
         MultipoleExpansion& operator+=(MultipoleExpansion const&);

         MultipoleExpansion const operator+(MultipoleExpansion const& that) const {
            return MultipoleExpansion(*this) += that;
         }

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }


      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned int const) {
            ar & m_order;
            ar & m_position;
            ar & m_multipoles;
         }

         void expandMultipoleVector(int const);

         int m_order;
         qglviewer::Vec m_position;
         QList<double> m_multipoles;
   };

   typedef Data::List<Data::MultipoleExpansion> MultipoleExpansionList;

} } // end namespace IQmol::Data

#endif
