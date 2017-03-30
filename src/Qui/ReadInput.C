/*!
 *  \file ReadInput.C 
 *
 *  \brief Functions used to read input information - geometry, job specification etc.
 *   
 *  The target is to populate the $sections and the OBMol object with as much
 *  information as possible.
 *   
 *  QChem input file  --> QCInputParser --> $sections --> Preview
 *                                                    --> OBMol
 *   
 *  QChem output file --> QCInputParser --> $sections
 *                    --> OpenBabel     --> OBMol
 *   
 *  Other input       --> OpenBabel     --> OBMol     --> $molecule
 *   
 *  \author Andrew Gilbert
 *  \date January 2008
 */

#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QMessageBox>

#include "QCJob.h"
#include "Qui.h" 
#include "RemSection.h"
#include "MoleculeSection.h"

#include <QtDebug>



namespace Qui {

class Molecule;

//! Opens the specified QChem input/output file and attempts to read in a list
//! of jobs which are separated by @@@.  Returns the number of jobs created.
int ReadInput(QFileInfo const&, QList<Job*>* ) {
   return 0;
}

//! Opens the specified file (which could be any one of the formats supported
//! by OpenBabel) and attempts to load the data into a Molecule object.
int ReadMolecule(QFileInfo const& , Molecule* ) {
   return 0;
}


//! Takes a string containing the contents of a QChem input file and extracts
//! Jobs from it.
QList<Job*> ParseQChemFileContents(QString const& contents) 
{
   QList<Job*> jobs;
   QStringList blocks;

   if (contents.contains("A Quantum Leap Into The Future Of Chemistry")) {
      // Output file
      blocks = contents.split("A Quantum Leap Into The Future Of Chemistry", 
         QString::SkipEmptyParts); 
      blocks.pop_front();  // Get rid of initial guff at the start of the file
   } else {
      // Input file
      blocks = contents.split("@@@");
   }

   for (int i = 0; i < blocks.count(); ++i) {
       Job* job = new Job(ReadKeywordSections(blocks.at(i)));
       jobs.append(job);
   }

   return jobs;
}


//! Takes the contents of an xyz file and atttmpts to parse the geometry. At 
//! the moment this just returns a string containing the coordinates, but it
//! should really return something smarter, like a Geometry object.  A simple
//! formatting check is made and if no valid geometry is found, an empty string
//! is returned.  Note that xyz format and a list of xyz coordinates are both ok.
//! TODO: This should really return a Geometry object
QString ParseXyzFileContents(QString const& contents, bool bailOnError) {
   QString geometry;

   QStringList lines(contents.split(QRegExp("\\n")));

   // First check to see if we have an xyz format and, if so, remove the extra
   // lines
   if (lines.count() > 2) {
      bool isInt(false);
      int nAtoms = lines[0].toInt(&isInt);
      if (isInt && nAtoms <= lines.count() - 2) {  
         lines.removeFirst();              // Number of atoms line
         lines.removeFirst();              // Comment line
         while (lines.count() > nAtoms) {  // only take the number of atoms specified
            lines.removeLast();
         }
      }
   }
   geometry = ParseXyzCoordinates(lines, bailOnError);

   return geometry;
}


//! Parses and checks list containing xyz coordinates.
QString ParseXyzCoordinates(QStringList const& coords, bool bailOnError) {
   QString geometry;
   QStringList tokens;
   bool okay(true), xOK, yOK, zOK;

   for (int i = 0; i < coords.count(); ++i) {
       tokens = coords.at(i).split(QRegExp("\\s+"), QString::SkipEmptyParts);

       if (tokens.count() == 4) {
          tokens[1].toDouble(&xOK);
          tokens[2].toDouble(&yOK);
          tokens[3].toDouble(&zOK);
          okay = xOK && yOK && zOK;
       }else {
          okay = false;
       }

       if (okay) {
          geometry += coords.at(i) + "\n";
       }else if (bailOnError) {
          return QString();
       }else {
          geometry += "ERROR: " + coords.at(i);
          qDebug() << "ERROR: ParseXyzCoordinates() error at line" << i;
       }
   }

   return geometry.trimmed();
}




//! Takes a string containing $sections for a QChem job and reads them into the
//! appropriate KeywordSection objects.  Note that it is assumed that the input
//! only contains one Job.
std::vector<KeywordSection*> ReadKeywordSections(QString input) {
   std::vector<KeywordSection*> sections;

   if (input.contains("@@@") || input.count("User input:") > 1) {
      qDebug() << "WARNING: Multiple jobs found in ReadSections()";
   }

   int i(-1), j, k;
   QString tmp, name;

   // This could be tightened up a bit - no checking is performed on the
   // validity of the input.  A better strategy would be to seek for $end and
   // search backwards for the '$'.
   while ( (i = input.indexOf("$")) != -1) {
      j = input.indexOf("$end",0,Qt::CaseInsensitive);
      
      tmp = input.mid(i+1,j-i-1);
      k = tmp.indexOf(QRegExp("\\W"));
      name = tmp.left(k);
      tmp.remove(0,k);

      // This allocates a KeywordSection, we rely on the Job distructor to free
      // this up.  That is, the KeywordSection object must be associated with a
      // Job object.
      qDebug() << "finding section" << name;
      KeywordSection* ks = KeywordSectionFactory(name);
      ks->read(tmp);
      ks->print(true);
      sections.push_back(ks);
      input.remove(0,j+3);
   }
   
   return sections;
}


} // end namespace Qui
