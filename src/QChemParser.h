#ifndef IQMOL_QCHEMPARSER_H
#define IQMOL_QCHEMPARSER_H
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

#include "BaseParser.h"
#include "DataLayer.h"
#include "ConformerListLayer.h"


class QTextStream;

namespace IQmol {
namespace Parser {

   /// Custom parser for QChem input and output files.  This parser checks 
   /// for multiple conformers from geometry optimizations and IRC calculations 
   /// Note that OpenBabel does not parse QChem input files and so we cover 
   /// these here as well.
   class QChem : public Base {

      public:
         QChem() : m_currentConformer(0), m_defaultConformer(0) { }

		 /// Loops over the contents of the file, calling processLine() to do
		 /// the work.
         DataList parse(QTextStream&);

		 /// Loops over the contents of the file checking for any fatal errors.
         QStringList parseForErrors(QTextStream&);

		 /// Returns a pointer to the default conformer, which depends on what
		 /// kind of job it is.  For geometry optimizations it is the final
		 /// geometry in the file, but for IRC calculations it is the intial
		 /// geometry which will be somewhere in the middle of the path.
		 Layer::Conformer* getDefaultConformer() const { return m_defaultConformer; }

      private:
		 /// Looks at the current line and decides if the lines that follow
		 /// need special treatment.  The effect of this function is to fill
         /// the m_conformers array with the different geometric structures
         /// (e.g. from an optimization).  For input files no conformers will
         /// be found.
         void processLine(QTextStream&);

		 /// Reads the coordinates from the 'Standard Nuclear Orientation'
		 /// block that is printed out once per conformer.
         void readCoordinates(QTextStream&);

         /// Reads the Mulliken charges for the current conformer.
         void readMullikenCharges(QTextStream&);

         /// Reads the Multipole charges for the current conformer.
         void readMultipoles(QTextStream&);

		 /// Used in IRC calculations to determine if the new conformer should
		 /// be prepended (true) or appended (false) to the list.  This is to 
         /// ensure the path is continuous rather than flipping to the midpoint.
         bool m_prependConformer; 

         QList<qglviewer::Vec> m_coordinates; ///< Temporary workspace for conformers
         QList<double> m_partialCharges;      ///< Temporary workspace for conformers
         QList<double> m_spinDensities;       ///< Temporary workspace for conformers
         QList<double> m_multipoles;          ///< Temporary workspace for conformers
         
         Layer::Conformer* m_currentConformer;
         Layer::Conformer* m_defaultConformer;
         QList<Layer::Conformer*> m_conformers;

         DataList m_inputDataList;   ///< Only used for input files
         DataList m_chargesList;


      public:
		 /// Subparser used to read the contents of a $molecule section in a
		 /// QChem input/output file.  This is not derived from Parser::Base as
		 /// it does not correspond to a distinct file type.
		 class MoleculeSection {
            public:
               DataList parse(QTextStream&);
         };

		 /// Subparser used to read the contents of a $efp_fragments section in a
		 /// QChem input/output file.  This is not derived from Parser::Base as
		 /// it does not correspond to a distinct file type.
		 class EfpFragmentsSection {
            public:
               DataList parse(QTextStream&);
         };

		 /// NYI!!! Subparser used to read the contents of a $rem section in a QChem
		 /// input/output file.  This is not derived from Parser::Base as it 
		 /// does not correspond to a distinct file type.
         class RemSection {
            public:
               DataList parse(QTextStream&);
         };

   };

} } // end namespace IQmol::Parser

#endif
