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

#include "Hessian.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {

Hessian::Hessian(unsigned const nAtoms, QList<double> const& hessian)
{
   setData(nAtoms, hessian);
}


void Hessian::setData(unsigned const nAtoms, QList<double> const& hessian) 
{
   unsigned n(hessian.size());
   unsigned d(3*nAtoms);
   unsigned k(0);

   if (n == d*d) {                           // square matrix
      m_hessian.resize(d,d);
      for (unsigned i = 0; i < d; ++i) {
          for (unsigned j = 0; j < d; ++j, ++k) {
              m_hessian(i,j) = hessian[k];
          }
      }
   }else if (n == d*(d+1)/2) {                 // triangular matrix
      m_hessian.resize(d,d);
      for (unsigned i = 0; i < d; ++i) {
          for (unsigned j = i; j < d; ++j, ++k) {
              m_hessian(i,j) = hessian[k];
              m_hessian(j,i) = hessian[k];
          }
      }
   }else {
      QLOG_DEBUG() << "Invalid Hessian data";
      m_hessian.resize(0,0);
   }
}


void Hessian::setPartialData(unsigned const nAtoms, QList<unsigned> const& atomList, 
   Matrix const& partialHessian)
{
   unsigned pAtoms(atomList.size());
   unsigned d(3*nAtoms);  // dimension of the full hessian
   unsigned pd(3*pAtoms); // dimension of the reduced hessian

   m_hessian.resize(d,d);
   for (unsigned i = 0; i < d; ++i) {
       for (unsigned j = 0; j < d; ++j) {
           m_hessian(i,j) = 0;
       }
   }

   if (partialHessian.size1() != pd || partialHessian.size2() != pd) {
      QLOG_DEBUG() << "Invalid Partial Hessian data";
      return;
   }

   for (unsigned i = 0; i < (unsigned)atomList.size(); ++i) {
       if (atomList[i] > nAtoms) {
          QLOG_DEBUG() << "Invalid Partial Hessian index" << i << atomList[i];
          return;
       }
   }

   // block insert the partial hessian into the full hessian
   for (unsigned pa = 0; pa < pAtoms; ++pa) {
       unsigned a(atomList[pa]-1);
       for (unsigned pb = 0; pb < pAtoms; ++pb) {
           unsigned b(atomList[pb]-1);

           for (unsigned i = 0; i < 3; ++i) {
               for (unsigned j = 0; j < 3; ++j) {
                   m_hessian(3*a+i,3*b+j) = partialHessian(3*pa+i,3*pb+j);
               }
           }
       }
   }
}


void Hessian::setData(Matrix const& hessian)
{
   m_hessian = hessian;
}


void Hessian::dump() const
{
   QStringList list(PrintMatrix(m_hessian));
    
   QStringList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       qDebug() << *iter;
   }
}

} } // end namespace IQmol::Data
