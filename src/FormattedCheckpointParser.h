#ifndef IQMOL_FORMATTEDCHECKPOINTPARSER_H
#define IQMOL_FORMATTEDCHECKPOINTPARSER_H
/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
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

#include "BaseParser.h"
#include "DataLayer.h"
#include "Shell.h"


class QTextStream;

namespace IQmol {
namespace Parser {

   /// Custom parser for FChk files.  Note that here we only concern ourselves 
   /// with getting the Layer::MolecularOrbitals data, we rely on  Parser::OpenBabel 
   /// to obtain the rest (such as coordinates).
   class FormattedCheckpoint : public Base {

      public:
         FormattedCheckpoint() { }
         DataList parse(QTextStream&);

      private:
         void processLine(QTextStream&);
         QList<int> readIntegerArray(QTextStream&, int nTokens);
         QList<double> readDoubleArray(QTextStream&, int nTokens);
         void generateShells();
         void makeAtomList();

         bool m_parseOkay;
         double m_energy;
         unsigned int m_nAlpha;
         unsigned int m_nBeta;
         unsigned int m_nBasis;
         unsigned int m_nOrbitals;

         QList<int>    m_atomicNumbers;
         QList<int>    m_shellTypes;
         QList<int>    m_shellPrimitives;
         QList<int>    m_shellToAtom;
         QList<double> m_coordinates;
         QList<double> m_exponents;
         QList<double> m_contractionCoefficients;
         QList<double> m_contractionCoefficientsSP;
         QList<double> m_alphaCoefficients;
         QList<double> m_betaCoefficients;
         QList<double> m_alphaEnergies;
         QList<double> m_betaEnergies;

         ShellList m_shells;
   };

} } // end namespace IQmol::Parser

#endif
