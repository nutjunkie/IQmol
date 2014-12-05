#ifndef QUI_MOLECULE_H
#define QUI_MOLECULE_H

/*!
 *  \class Molecule
 *
 *  \brief Wrapper around the OBMol class that adds  functionality, in
 *  particular signals and slots.
 *   
 *  \author Andrew Gilbert
 *  \date September 2010
 */

#include <QObject>
#include <QFileInfo>

#include "openbabel/mol.h"
#include "openbabel/forcefield.h"


namespace OpenBabel {
   class OBAtom;
}

namespace Qui {

struct Coordinates {
   enum ID { Cartesian = 0, ZMatrix };
};
 

class Molecule : public QObject, public OpenBabel::OBMol {

   Q_OBJECT

   public:
      Molecule();

      bool setCoordinates(QString const& data);

      QString formatForQui(Coordinates::ID const& type);
      QString formatConstraintsForQui();
      void setCoordinates(QString const& coords, Coordinates::ID const& type);

      OpenBabel::OBFFConstraints& getConstraints() { return m_constraints; }
      int numberOfConstraints() { return m_constraints.Size(); }

      // Constraint functions
      void clearAllConstraints();
      void fixAtom(OpenBabel::OBAtom* a);
      void fixBond(OpenBabel::OBAtom* a, OpenBabel::OBAtom* b, double length);
      void fixAngle(OpenBabel::OBAtom* a, OpenBabel::OBAtom* b, 
                    OpenBabel::OBAtom* c, double angle);
      void fixTorsion(OpenBabel::OBAtom* a, OpenBabel::OBAtom* b, 
                      OpenBabel::OBAtom* c, OpenBabel::OBAtom* d, double angle);
      void deleteAtomAndConstraints(OpenBabel::OBAtom* a);


   public Q_SLOTS:

   Q_SIGNALS:
      void updated();


   private:
	  // Flag set if the $molecule section contains 'read'.  This idicates the
	  // data from the previous job should be used.
      bool m_readFlag;
      OpenBabel::OBFFConstraints m_constraints;
};

} // end namespace Qui

#endif
