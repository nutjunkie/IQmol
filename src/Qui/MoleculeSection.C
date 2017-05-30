/*!
 *  \file MoleculeSection.C 
 *
 *  \brief Non-inline member functions of the MoleculeSection class, see
 *  MoleculeSection.h for details.
 *   
 *  \author Andrew Gilbert
 *  \date February 2008
 */

#include "MoleculeSection.h"
#include "QuiMolecule.h"

#include <QRegExp>
#include <QStringList>
#include <QMessageBox>

#include <QtDebug>


namespace Qui {


//! Takes a the text from between the $molecule-$end tags and parses it.
void MoleculeSection::read(QString const& input) 
{
   QStringList lines( input.trimmed().split(QRegExp("\\n")) );
   bool okay(false);

   if (lines.count() > 0) {
      QString first(lines[0].replace(QChar(','),QChar(' ')));
      QStringList tokens(first.split(QRegExp("\\s+"), QString::SkipEmptyParts));
      lines.removeFirst();

      if (tokens.count() == 1) {
         if (tokens[0].toLower() == "read") {
            setCoordinates("read");
            okay = true;
         }
      }else if (tokens.count() == 2) {
         // line 1 is charge + multiplicity
         // everything else is the molecule
         bool c,m;
         m_charge = tokens[0].toInt(&c);
         m_multiplicity = tokens[1].toInt(&m);
         okay = c && m;
         setCoordinates(lines.join("\n"));
        // m_molecule = new Molecule();
        // m_molecule->setCoordinates(input);
      }
      
   }

   // TODO: This should really load a molecule object so that the coordinate
   // conversion can be done.
   if (!okay) {
      QString msg("Problem reading $molecule section: \n");
      msg += input;
      QMessageBox::warning(0, "Parse Error", msg);
   }
}


bool MoleculeSection::isReadCoordinates() {
    return (m_coordinates == "read");
}


//! Returns a Molecule object corresponding to the current contents of the
//! $molecule section
Molecule* MoleculeSection::getMolecule() {
   if (m_molecule == 0) {
      m_molecule = new Molecule();
      m_molecule->setCoordinates(myDump());
      qDebug() << "Creating new molecule from";
      qDebug() << myDump();
      qDebug() << "--------------------------";
   }
   return m_molecule;
}


QString MoleculeSection::myDump() 
{
   QString s;
   if (isReadCoordinates()) {
      s += "read\n";
   }else if (m_molecule) {
      s = m_molecule->formatForQui(Coordinates::Cartesian);
      s += "\n";
   }else {
      s += QString::number(m_charge) + " ";
      s += QString::number(m_multiplicity) + "\n";
      if (m_isFsm) {
         s += "  " + m_coordinatesFsm + "\n";
      }else {
         if (m_coordinates != "") {
            s += "  " + m_coordinates + "\n";
         }
      }
   }
   return s;
}


QString MoleculeSection::dump() {
   QString s("$molecule\n");
   s += myDump();
   s += "$end\n";
   return s;
}



MoleculeSection* MoleculeSection::clone() const {
   return new MoleculeSection(m_coordinates, m_charge, m_multiplicity);
}


void MoleculeSection::setCoordinates(QString const& coordinates) { 
   m_coordinates = coordinates; 
   parseCoordinates();
}


void MoleculeSection::setCoordinatesFsm(QString const& coordinates) { 
qDebug() << "Fsm coordinates set to " << coordinates;
   m_coordinatesFsm = coordinates; 
}




void MoleculeSection::setMolecule(Molecule* mol) { 
   m_molecule = mol; 
}


// TODO: this should be smartened up a bit, for the time being all I want is
// the number of atoms.
void MoleculeSection::parseCoordinates() {
   m_coordinates = m_coordinates.trimmed();
   if (m_coordinates == "read" || m_coordinates == "") {
      m_numberOfAtoms = 0;
   }else {
      m_numberOfAtoms = m_coordinates.count(QRegExp("\\n")) + 1; 
   }
   return;
}


void MoleculeSection::setCharge(int charge) {
   m_charge = charge;
   if (m_molecule) m_molecule->SetTotalCharge(charge);
}


void MoleculeSection::setMultiplicity(int multiplicity) {
   m_multiplicity = multiplicity;
   if (m_molecule) m_molecule->SetTotalSpinMultiplicity(multiplicity);
}

} // end namespace Qui
