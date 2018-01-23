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

#include "PopulationsDialog.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {

PopulationsDialog::PopulationsDialog(Data::ShellList const& shellList, 
   Data::Density const& density, QWidget* parent) : QDialog(parent), 
   m_shellList(shellList), m_density(density)
{
   m_dialog.setupUi(this);
   setWindowTitle("Population decomposition for " + m_density.label());
   compute();
}


void PopulationsDialog::compute()
{
   Vector const& overlap(m_shellList.overlapMatrix());
   Vector const* density(m_density.vector());

   // assume the last shell is on the last atom
   unsigned nAtoms(m_shellList.last()->atomIndex()+1);
   unsigned nBasis(m_shellList.nBasis());

   if (overlap.size()  != nBasis*(nBasis+1)/2 ||
       density->size() != nBasis*(nBasis+1)/2) {
      qDebug() << "Incommensurate vectorised matrices in PopulationsDialog";
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

/*
   qDebug() << "printing population matrix";
   QStringList pr(PrintMatrix(M));
   for (int i = 0; i < pr.size(); ++i) {
       qDebug() << pr[i];
   }
*/

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
}


void PopulationsDialog::fillTable(Matrix const& M)
{
    QTableWidget* table(m_dialog.populationsTable); 
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
