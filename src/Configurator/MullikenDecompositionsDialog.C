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

#include "MullikenDecompositionsDialog.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {

MullikenDecompositionsDialog::MullikenDecompositionsDialog(Data::ShellList const& shellList, 
   Data::Density const& density, QWidget* parent) : QDialog(parent), 
   m_shellList(shellList), m_density(density)
{
   m_dialog.setupUi(this);
   setWindowTitle("Mulliken decomposition for " + m_density.label());
}


void MullikenDecompositionsDialog::compute()
{
   Vector const& overlap(m_shellList.overlapMatrix());
   Vector const* density(m_density.vector());

   // assume the last shell is on the last atom
   unsigned nAtoms(m_shellList.last()->atomIndex()+1);
   unsigned nBasis(m_shellList.nBasis());

   if (2*overlap.size()  != nBasis*(nBasis+1) ||
       2*density->size() != nBasis*(nBasis+1)) {
      qDebug() << "Incommensurate vectorised matrices in MullikenDecompositionsDialog";
      qDebug() << "S" << overlap.size() << "P" << density->size() << ":"
               << nBasis*(nBasis+1)/2 ;
      return;
   }

   Matrix P(nBasis, nBasis);
   Matrix S(nBasis, nBasis);

   unsigned k(0);
   for (unsigned i = 0; i < nBasis; ++i) {
       for (unsigned j = 0; j <=i; ++j, ++k) {
           P(i,j) = P(j,i) = (*density)[k];
           S(i,j) = S(j,i) = overlap[k];
       }
   }

   Matrix M(nAtoms, nAtoms);
   for (unsigned i = 0; i < nAtoms; ++i) {
       for (unsigned j = 0; j < nAtoms; ++j) {
           M(i,j) = 0.0;
       }
   }

   Data::ShellList::const_iterator iShell, jShell;
   unsigned iAtom(0), jAtom(0), iBasOff(0), jBasOff(0);

   for (iShell = m_shellList.begin(); iShell != m_shellList.end(); ++iShell) {
       iAtom = (*iShell)->atomIndex();
       jBasOff= 0;

       for (jShell = m_shellList.begin(); jShell != m_shellList.end(); ++jShell) {
           jAtom = (*jShell)->atomIndex();

           for (unsigned iBasis = iBasOff; iBasis < iBasOff+(*iShell)->nBasis(); ++iBasis) {
               for (unsigned jBasis = jBasOff; jBasis < jBasOff+(*jShell)->nBasis(); ++jBasis) {
                   M(iAtom, jAtom) += P(iBasis,jBasis) * S(iBasis,jBasis);
               }
           }
         
           jBasOff += (*jShell)->nBasis();
       }
       iBasOff += (*iShell)->nBasis();
   }

   double nElectrons(0.0);
   for (unsigned i = 0; i < nAtoms; ++i) {
       for (unsigned j = 0; j < nAtoms; ++j) {
           nElectrons += M(i,j);
       }
   }

   QLOG_TRACE() << "Number of electrons: " << nElectrons;
   QString text("Number of electrons: ");
   text += QString::number(nElectrons,'f',6);
   m_dialog.label->setText(text);
   fillTable(M);

   mullikenDecompositionsAvailable(M);
}


void MullikenDecompositionsDialog::fillTable(Matrix const& M)
{
    QTableWidget* table(m_dialog.decompositionTable); 
    unsigned nAtoms(M.size1());

    table->setRowCount(nAtoms);
    table->setColumnCount(nAtoms);

    for (unsigned i = 0; i < nAtoms; ++i) {
        for (unsigned j = i; j < nAtoms; ++j) {
            double m( i == j ? M(i,j) : 2.0*M(i,j));
            QTableWidgetItem* item(new QTableWidgetItem(QString::number(m,'f',6)));
            table->setItem(j,i,item);
        }
    }
}

} // end namespace IQmol
