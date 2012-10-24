/*!
 *  \file InputDialogAvogadro.C 
 *  
 *  \brief This file contains extensions to the InputDialog class that are only
 *  relevant when the QUI is compiled as an Avogadro extension.
 *  
 *  \author Andrew Gilbert
 *  \date   November 2008
 */

#ifndef AVOGADRO
#error Macro AVOGADRO not defined in InputDialogAvogadro.C
#endif


#include <QTextStream>
#include <QtDebug>
#include "Qui.h"


using namespace Avogadro;
using namespace OpenBabel;


namespace Qui {


int TotalChargeOfNuclei(Avogadro::Molecule* molecule) {
   int count(0);

   if (molecule) {
      FOR_ATOMS_OF_MOL(atom, molecule) {
         count += atom->GetAtomicNum();
      }
   }

   return count;
}


QString ExtractGeometry(Avogadro::Molecule* molecule, QString const& coords) {

   QString buffer;
   QTextStream mol(&buffer);
   if (!molecule) return buffer;

   if (coords.toUpper() == "CARTESIAN") {
      FOR_ATOMS_OF_MOL(atom, molecule) {
         mol << qSetFieldWidth(3) << right 
             << QString(etab.GetSymbol(atom->GetAtomicNum()))
             << qSetFieldWidth(12) << qSetRealNumberPrecision(5) << forcepoint
             << fixed << right << atom->GetX() << atom->GetY() << atom->GetZ()
             << qSetFieldWidth(0) << "\n";
      }


   }else if (coords.toUpper() == "Z-MATRIX") {
      OBAtom *a, *b, *c;
      double r, w, t;

      /* Taken from OpenBabel's gzmat file format converter */
      std::vector<OBInternalCoord*> vic;
      vic.push_back((OBInternalCoord*)NULL);

      FOR_ATOMS_OF_MOL(atom, molecule) {
         vic.push_back(new OBInternalCoord);
      }

      CartesianToInternal(vic, (OpenBabel::OBMol&)*molecule);

      FOR_ATOMS_OF_MOL(atom, molecule) {
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

      FOR_ATOMS_OF_MOL(atom, molecule) {
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


   }else {
      OBAtom *a, *b, *c;
      double r, w, t;

      /* Taken from OpenBabel's gzmat file format converter */
      std::vector<OBInternalCoord*> vic;
      vic.push_back((OBInternalCoord*)NULL);

      FOR_ATOMS_OF_MOL(atom, molecule) {
         vic.push_back(new OBInternalCoord);
      }

      CartesianToInternal(vic, (OpenBabel::OBMol&)*molecule);

      FOR_ATOMS_OF_MOL(atom, molecule) {
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

   return buffer.trimmed();
}


} // end namespace Qui
