/*!  
 *  \file Molecule.C
 *   
 *  \brief  very
 *   
 *  \author Andrew Gilbert
 *  \date September 2010
 */

#include <iostream>
#include <sstream>
#include <string>
#include <QTextStream>
#include <QtDebug>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QRegExp>

#include "openbabel/atom.h"
#include "openbabel/format.h"
#include "openbabel/obconversion.h"
#include "openbabel/forcefield.h"

#include "QuiMolecule.h"


using namespace OpenBabel;

namespace Qui {

Molecule::Molecule() {
   setCoordinates("0 1\nH  0.00 0.00 0.00");
   OBAtom* atom = GetAtom(1);
   DeleteAtom(atom);
   m_readFlag = false;
}


//! This function assumes that the molecule data is coming from a QChem
//! $molecule section and is therefore in either XYZ or z-matrix format 
//! or is the token 'read'.  The $keywords should have been removed.
//! Returns true if valid (in the QChem sense) information is loaded.
bool Molecule::setCoordinates(QString const& input) {
   m_readFlag = false; 
   QStringList lines( input.trimmed().split(QRegExp("\\n")) );

   if (lines.count() < 1) return false;

   QString first(lines[0].replace(QChar(','),QChar(' ')));
   QStringList tokens(first.split(QRegExp("\\s+"), QString::SkipEmptyParts));

   // Molecule read from previous job
   if (lines.count() == 1 && tokens[0].toLower() == "read") {
      m_readFlag = true;
      return true;
   }

   // Regular case, need at least the charge/multiplicity line and a single atom
   if (lines.count() < 2 || tokens.count() < 2) return false;

   bool ok;
   int charge = tokens[0].toInt(&ok);
   if (!ok) return false;
   unsigned int mult = tokens[1].toUInt(&ok);
   if (!ok) return false;


   // We take a peek at the first line of coordinates.  If there is only one
   // token then we assume a z-matrix format, otherwise we assume an XYZ.
   tokens.clear();
   tokens = lines[1].split(QRegExp("\\s+"), QString::SkipEmptyParts);

   Clear();  // Clear the current OBMol data
   OBConversion conv;
   QString s;

   if (tokens.count() == 1) {
      conv.SetInFormat("gzmat");
      // create dummy z-matrix input
      s = "#\n\nzmat\n\n";
      s += input;
   }else {
      conv.SetInFormat("xyz");
      // create dummy xyz input
      lines.pop_front();  // remove the charge/multiplicity line
      int nAtoms = lines.count();
      s = QString::number(nAtoms);
      s += "\n\n" + lines.join("\n");
      SetTotalCharge(charge);                  // !!! Does this get overwritten
      SetTotalSpinMultiplicity(mult);
   }

   //qDebug() << "Molecule::setCoordinates temporaray string:";
   //qDebug() << input;
   //qDebug() << "-------------------------------------------";
   //qDebug() << s;
   //qDebug() << "-------------------------------------------";
   
   std::istringstream iss( std::string(s.toLatin1()) );
   conv.Read(this, &iss);
      SetTotalCharge(charge);          
      SetTotalSpinMultiplicity(mult);

   return true;
}


QString Molecule::formatConstraintsForQui() {
   QString fixed; 
   QString other; 

   int a, b, c, d; 
   double v;

   qDebug() << "formatConstraints #" << m_constraints.Size();

   for (int i = 0; i < m_constraints.Size(); ++i) {
       switch (m_constraints.GetConstraintType(i)) {

          case OBFF_CONST_ATOM:
             a = m_constraints.GetConstraintAtomA(i);
             fixed += QString::number(a) + "  XYZ\n";
             break;

          case OBFF_CONST_DISTANCE:
             v = m_constraints.GetConstraintValue(i);
             a = m_constraints.GetConstraintAtomA(i);
             b = m_constraints.GetConstraintAtomB(i);
             other += "stre  " + QString::number(a) + "  "
                               + QString::number(b) + "  "
                               + QString::number(v, 'f', 5) +  "\n";
             break;

          case OBFF_CONST_ANGLE:
             v = m_constraints.GetConstraintValue(i);
             a = m_constraints.GetConstraintAtomA(i);
             b = m_constraints.GetConstraintAtomB(i);
             c = m_constraints.GetConstraintAtomC(i);
             other += "bend  " + QString::number(a) + "  "
                               + QString::number(b) + "  "
                               + QString::number(c) + "  "
                               + QString::number(v, 'f', 3) +  "\n";
                                                      
             break;

          case OBFF_CONST_TORSION:
             v = m_constraints.GetConstraintValue(i);
             a = m_constraints.GetConstraintAtomA(i);
             b = m_constraints.GetConstraintAtomB(i);
             c = m_constraints.GetConstraintAtomC(i);
             d = m_constraints.GetConstraintAtomD(i);
             other += "tors  " + QString::number(a) + "  "
                               + QString::number(b) + "  "
                               + QString::number(c) + "  "
                               + QString::number(d) + "  "
                               + QString::number(v, 'f', 3) +  "\n";
             break;

       }

       //if (!fixed.isEmpty()) fixed = "FIXED\n" + other + "ENDFIXED\n";
       //if (!other.isEmpty()) other = "CONSTRAINT\n" + other + "ENDCONSTRAINT\n";

       other += fixed;
   }

   return other;
}


QString Molecule::formatForQui(Coordinates::ID const& coords) {
   QString buffer;
   QTextStream mol(&buffer);

   switch (coords) {

      case Coordinates::Cartesian: {
         mol << qSetFieldWidth(3) << right 
             << GetTotalCharge() << GetTotalSpinMultiplicity() << "\n";
         FOR_ATOMS_OF_MOL(atom, this) {
            mol << qSetFieldWidth(3) << right 
                << QString(etab.GetSymbol(atom->GetAtomicNum()))
                << qSetFieldWidth(12) << qSetRealNumberPrecision(6) << forcepoint
                << fixed << right << atom->GetX() << atom->GetY() << atom->GetZ()
                << qSetFieldWidth(0) << "\n";
         }
      } break;

      case Coordinates::ZMatrix: {
         OBAtom *a, *b, *c;
         double r, w, t;

         /* Taken from OpenBabel's gzmat file format converter */
         std::vector<OBInternalCoord*> vic;
         vic.push_back((OBInternalCoord*)NULL);

         FOR_ATOMS_OF_MOL(atom, this) {
            vic.push_back(new OBInternalCoord);
         }

         CartesianToInternal(vic, (OpenBabel::OBMol&)*this);

         FOR_ATOMS_OF_MOL(atom, this) {
         a = vic[atom->GetIdx()]->_a;
         b = vic[atom->GetIdx()]->_b;
         c = vic[atom->GetIdx()]->_c;

         mol << qSetFieldWidth(4) << right
             << QString(etab.GetSymbol(atom->GetAtomicNum())
                      + QString::number(atom->GetIdx()))
             << qSetFieldWidth(0);
 
         if (atom->GetIdx() > 1) {
            mol << " "  << QString(etab.GetSymbol(a->GetAtomicNum())
                           + QString::number(a->GetIdx()))
                << " r" << atom->GetIdx();
         }

         if (atom->GetIdx() > 2) {
            mol << " "  << QString(etab.GetSymbol(b->GetAtomicNum())
                         + QString::number(b->GetIdx()))
                << " a" << atom->GetIdx();
         }

         if (atom->GetIdx() > 3) {
            mol << " " << QString(etab.GetSymbol(c->GetAtomicNum())
                          + QString::number(c->GetIdx()))
                << " d" << atom->GetIdx();
         }
         mol << "\n";
      }

      mol << "\n";

      FOR_ATOMS_OF_MOL(atom, this) {
         r = vic[atom->GetIdx()]->_dst;
         w = vic[atom->GetIdx()]->_ang;
         t = vic[atom->GetIdx()]->_tor;

         if (w < 0.0) w += 360.0;
         if (t < 0.0) t += 360.0;
          
         if (atom->GetIdx() > 1) {
            mol << "   r" << atom->GetIdx() << " = " << qSetFieldWidth(12)
                << qSetRealNumberPrecision(5) << forcepoint << fixed << right
                << r << qSetFieldWidth(0) << "\n";
         }

         if (atom->GetIdx() > 2) {
            mol << "   a" << atom->GetIdx() << " = " << qSetFieldWidth(12)
                << qSetRealNumberPrecision(5) << forcepoint << fixed << right
                << w << qSetFieldWidth(0) << "\n";
         }

         if (atom->GetIdx() > 3) {
             mol << "   d" << atom->GetIdx() << " = " << qSetFieldWidth(12)
                 << qSetRealNumberPrecision(5) << forcepoint << fixed << right
                 << t << qSetFieldWidth(0) << "\n";
         }
      }



      } break;

      default: {
         OBAtom *a, *b, *c;
         double r, w, t;

         /* Taken from OpenBabel's gzmat file format converter */
      std::vector<OBInternalCoord*> vic;
      vic.push_back((OBInternalCoord*)NULL);

      FOR_ATOMS_OF_MOL(atom, this) {
         vic.push_back(new OBInternalCoord);
      }

      CartesianToInternal(vic, (OpenBabel::OBMol&)*this);

      FOR_ATOMS_OF_MOL(atom, this) {
         a = vic[atom->GetIdx()]->_a;
         b = vic[atom->GetIdx()]->_b;
         c = vic[atom->GetIdx()]->_c;
         r = vic[atom->GetIdx()]->_dst;
         w = vic[atom->GetIdx()]->_ang;
         t = vic[atom->GetIdx()]->_tor;

         if (w < 0.0) w += 360.0;
         if (t < 0.0) t += 360.0;

         mol << qSetFieldWidth(4) << right
             << QString(etab.GetSymbol(atom->GetAtomicNum())
                 + QString::number(atom->GetIdx()));

         if (atom->GetIdx() > 1) {
            mol << qSetFieldWidth(6) << right
                << QString(etab.GetSymbol(a->GetAtomicNum())
                        + QString::number(a->GetIdx())) << qSetFieldWidth(12)
                << qSetRealNumberPrecision(5) << forcepoint << fixed << right << r;
         }

         if (atom->GetIdx() > 2) {
            mol << qSetFieldWidth(6) << right
                << QString(etab.GetSymbol(b->GetAtomicNum())
                    + QString::number(b->GetIdx())) << qSetFieldWidth(12)
                << qSetRealNumberPrecision(5) << forcepoint << fixed << right << w;
         }

         if (atom->GetIdx() > 3) {
            mol << qSetFieldWidth(6) << right
                << QString(etab.GetSymbol(c->GetAtomicNum())
                    + QString::number(c->GetIdx())) << qSetFieldWidth(12)
                << qSetRealNumberPrecision(5) << forcepoint << fixed << right << t;
         }

         mol << qSetFieldWidth(0) << "\n";
      }
   }
   break;
   }

   return buffer.trimmed();
}


/*
 *   Constraint functions
 */


void Molecule::clearAllConstraints() {
   m_constraints.Clear();
}


void Molecule::fixAtom(OBAtom* a) {
   m_constraints.AddAtomConstraint(a->GetIdx());
}


void Molecule::fixBond(OBAtom* a, OBAtom* b, double length) {
   m_constraints.AddDistanceConstraint(a->GetIdx(), b->GetIdx(), length);
}


void Molecule::fixAngle(OBAtom* a, OBAtom* b, OBAtom* c, double angle) {
   m_constraints.AddAngleConstraint(a->GetIdx(), b->GetIdx(), c->GetIdx(), angle);
}


void Molecule::fixTorsion(OBAtom* a, OBAtom* b, OBAtom* c, OBAtom* d, double angle) {
   m_constraints.AddTorsionConstraint(a->GetIdx(), b->GetIdx(), c->GetIdx(), d->GetIdx(), angle);
}
                      

void Molecule::deleteAtomAndConstraints(OBAtom* a) {
   int ia(a->GetIdx());
   qDebug() << ia;
}



} // end namespace Qui
