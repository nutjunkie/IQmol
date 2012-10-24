#ifndef IQMOL_MOLECULELAYER_H
#define IQMOL_MOLECULELAYER_H
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

#include "IQmol.h"
#include "InfoLayer.h"
#include "FileLayer.h"
#include "AtomLayer.h"
#include "BondLayer.h"
#include "ChargeLayer.h"
#include "MoleculeConfigurator.h"
#include "Animator.h"
#include <QFileInfo>
#include <QMap>
#include <QItemSelectionModel>
#include "boost/bind.hpp"
#include "boost/function.hpp"


class QUndoCommand;
class QDropEvent;
class QDragEnterEvent;

namespace OpenBabel {
   class OBMol;
   class OBAtom;
   class OBBond;
}

namespace IQmol {

namespace Handler {
   class Build;
   class ReindexAtoms;
}

namespace Command {
   class AppendData;
   class RemoveData;
   class AddPrimitives;
   class EditPrimitives;
   class AddHydrogens;
   class RemovePrimitives;
   class MoveObjects;
   class AddMolecule;
   class RemoveMolecule;
   class ChangeBondOrder;
   class ChangeAtomType;
}


class SpatialProperty;
class PointChargePotential;
class JobInfo;

namespace Layer {

   class Conformer;
   class Constraint;
   class Surface;
   typedef QMap<OpenBabel::OBAtom*, Atom*> AtomMap;
   typedef QMap<OpenBabel::OBBond*, Bond*> BondMap;

   /// Container Layer for all things related to a particular molecule.
   /// This is the main data structure for a molecule.  It contains the Primitve
   /// GLObjects that visually represent the molecule along with Data Layers
   /// containing information such as checkpoint files, cube files and output
   /// files.
   class Molecule : public Base {

      Q_OBJECT

      friend class Frequencies;
      friend class ConformerList;
      friend class Configurator::Molecule;

      // !!! some of these are no longer required to be friends
      friend class Command::AppendData;
      friend class Command::RemoveData;
      friend class Command::AddPrimitives;
      friend class Command::EditPrimitives;
      friend class Command::AddHydrogens;
      friend class Command::RemovePrimitives;
      friend class Command::MoveObjects;
      friend class Command::AddMolecule;
      friend class Command::RemoveMolecule;
      friend class Command::ChangeBondOrder;
      friend class Command::ChangeAtomType;
      friend class Handler::Build;
      friend class Handler::ReindexAtoms;

      public:
         explicit Molecule(QObject* parent = 0);
         ~Molecule();
         double radius();

		 /// Attempts to save the molecule, returning false if the operation
		 /// was unsuccessful or canceled by the user.
         bool save(bool prompt = false);

         void dropEvent(QDropEvent* event);
         void dragEnterEvent(QDragEnterEvent* event);

		 /// Appends the Layers in the DataList, but only if an existing Layer
		 /// of that kind does not exist already.
         void appendData(DataList&);
         void removeData(DataList&);

         void setFile(QString const& fileName);
         QString fileName() const { return m_inputFile.fileName(); }
         //QFileInfo fileInfo() const { return m_inputFile; }

		 /// Creates a new JobInfo object and sends it out into the ether.
		 /// This really needs to be a smart pointer because we don't delete
		 /// as it may be being used by a Process or Server.
		 JobInfo* jobInfo();
         bool jobInfoMatch(JobInfo* jobInfo) { return jobInfo == m_jobInfo; }

         void addHydrogens();
         void symmetrize(double tolerance, bool updateCoordinates = true);
         void minimizeEnergy(QString const& forcefield);
         void translateToCenter(GLObjectList const& selection);

		 /// Obtains a list of selected Primitives in the Molecule and removes
		 /// them from the lists.  The Primitives are not immediately deleted,
		 /// rather they are embedded in a RemovePrimtives Command so that the 
         /// action can be undone.
         void deleteSelection();
         
		 /// Returns a list of the selected atoms and bonds.  If dangling bonds
		 /// is true then all selected bonds are included, otherwise only bonds
		 /// with both atoms selected are included.
         PrimitiveList getSelected(bool danglingBonds = false);

         /// Locates all atoms for which there exists a path to B without going through A.
         AtomList getContiguousFragment(Atom* A, Atom* B);
         Bond* getBond(Atom*, Atom*);
         bool isModified() const { return m_modified; }

         qglviewer::Vec centerOfNuclearCharge();
         QStringList getAvailableProperties(); 
         Function3D getPropertyEvaluator(QString const& name);

		 /// Removes the specified Primitives from the molecule, but does not
		 /// delete them. 
         void takePrimitives(PrimitiveList const&);

         /// Overloaded version that adds a single primitive.
         void takePrimitive(Primitive*);

         /// Adds the specified Primitives to the molecule.
         void appendPrimitives(PrimitiveList const&);

         /// Overloaded version that adds a single primitive.
         void appendPrimitive(Primitive*);

		 /// Checks to see if a Constraint already exists involving the same
		 /// atoms. If it does, the existing Constraint is updated with the new
		 /// target value, if not the new Constraint is appended to the list.
		 void appendConstraint(Constraint*);
         void removeConstraint(Constraint*);

         /// Converts the Molecule to an XYZ format and uses OpenBabel to parse this.  
         /// Useful for, e.g., reperceiving bonds.
         QString coordinatesAsString(bool const selectedOnly = false);

      public Q_SLOTS:
         void appendDataFile(Data*);
         void appendSurface(Layer::Surface*);
         void reperceiveBonds();
         void determineSymmetry() { symmetrize(0.00001, false); }
         void setGasteigerCharges();
         void setSandersonCharges();
         void setMullikenCharges();
         void groupSelection();
         void ungroupSelection();
         void applyConstraint(Constraint*);
         void constraintUpdated();
         void deleteConstraint();
         void selectAll();

         /// Passes the remove signal on so that the ViewerModel can deal with it
         void removeMolecule() { removeMolecule(this); }

      Q_SIGNALS:
         void softUpdate(); // issue if the number of primitives does not change
         void postCommand(QUndoCommand*);
         void postMessage(QString const&);
         void appendData(QString const&, Layer::Molecule&);
         void pushAnimators(AnimatorList const&);
         void popAnimators(AnimatorList const&);
         void removeMolecule(Layer::Molecule*);

         void pointGroupAvailable(QString const&);
         void energyAvailable(double const, Info::EnergyUnit);
         void dipoleAvailable(qglviewer::Vec const& dipole, bool const estimated);
         void radiusAvailable(double const radius);
         void centerOfChargeAvailable(qglviewer::Vec const&);
         void select(QModelIndex const&, QItemSelectionModel::SelectionFlags);


      protected:
         void updateAtomScale(double const scale);
         void updateSmallerHydrogens(bool smallerHydrogens);
         void updateBondScale(double const scale);
         void updateChargeScale(double const scale);
         void updateDrawMode(Primitive::DrawMode const);
         void updateInfo();

         Atom* createAtom(unsigned int const Z, qglviewer::Vec const& position);
         Bond* createBond(Atom* begin, Atom* end, int const order = 1);
         Charge* createCharge(double const q, qglviewer::Vec const& position);

         /// Updates the atom and bond indicies after, for example, deletion.
         void reindexAtomsAndBonds();

		 /// Assigns the atom indices based on the ordering selected by 
		 /// the user via the reorderIndex variable in the Atom class.
         void updateAtomOrder();

      private Q_SLOTS:
         void jobInfoChanged();

      private:
         int totalCharge() const;
         int multiplicity() const;
         QString constraintsAsString();

         template <class T>
         void forall(boost::function<void(T&)> function, QList<T> list);
		 /// Writes the molecule to the specified file.  The format is
		 /// determined from the file extension and a Parser::IOError 
		 /// exception is thrown if there are any problems.
         void writeToFile(QString const& filePath);

		 /// Translates the coordinates of all the atoms so that the constraint is satisfied.
         void applyPositionConstraint(Constraint*);

		 /// Adjusts the coordinates of the Molecule so that the constraint is satisfied.
         void applyDistanceConstraint(Constraint*);

		 /// Adjusts the coordinates of the Molecule so that the constraint is satisfied.
         void applyAngleConstraint(Constraint*);

		 /// Adjusts the coordinates of the Molecule so that the constraint is satisfied.
         void applyTorsionConstraint(Constraint*);

		 /// Used when a ring constraint is encountered.  In this case the
		 /// atoms are not moved so that the constraint is exactly satisfied,
		 /// rather we pass the constraint to the minmizer to sort things out.
         void applyRingConstraint();

         /// Note that these create new OBMol objects which must be deleted
         OpenBabel::OBMol* toOBMol(AtomMap&, BondMap&);
         OpenBabel::OBMol* toOBMol();

		 /// Loads the molecular information from an OBMol object.  If existing
		 /// Atom and Bond Maps are specified, only the additional primitives
		 /// (e.g. added hydrogens) are returned, otherwise a complete list of
		 /// added primtives is returned.  The returned primitive list can be
         /// used for UndoCommands.  Note that this does not load all the data
         /// in the OBMol (e.g. frequecny data), for that Parser::OpenBabel
         /// should be used.
		 PrimitiveList fromOBMol(OpenBabel::OBMol*, AtomMap* = 0, BondMap* = 0);

         template <class T> void update(boost::function<void(T&)>);
         void translate(qglviewer::Vec const& displacement);
         void rotate(qglviewer::Quaternion const& rotation);
         void alignToAxis(qglviewer::Vec const& point, 
            qglviewer::Vec axis = qglviewer::Vec(0.0, 0.0, 1.0));
         void rotateIntoPlane(qglviewer::Vec const& point, 
            qglviewer::Vec const& axis = qglviewer::Vec(0.0, 0.0, 1.0),
            qglviewer::Vec const& normal = qglviewer::Vec(0.0, 1.0, 0.0));
         void clearData();
         void computeMoments();

         template <class T>
         void calculateSuperposition(Surface*);
         void calculateSID(Surface*);
         void calculateVanDerWaals(Surface*, double const scale = 1.0,
            double const solventRadius = 0.0);

         QFileInfo m_inputFile;

		 /// State variable that determines how the Primitives are drawn (e.g.
		 /// CPK or wireframe)
         Primitive::DrawMode m_drawMode;

		 /// Determines by how much the atom radius should be scaled from the
		 /// default value.
         double m_atomScale;

		 /// Determines by how much the bond radius should be scaled from the
		 /// default value.
         double m_bondScale;

		 /// Determines by how much the charge radius should be scaled from the
		 /// default value.
         double m_chargeScale;

		 /// Determines if the hydrogen atoms should be drawn smaller in the CPK
         /// model.
         bool m_smallerHydrogens;

         Configurator::Molecule m_configurator;
         
         // This is the last JobInfo we created
         JobInfo*       m_jobInfo;
         Layer::Atoms   m_atomList;
         Layer::Bonds   m_bondList;
         Layer::Charges m_chargeList;
         Layer::Data    m_surfaceList;
         Layer::Files   m_fileList;
         Layer::Info    m_info;
         Layer::Base    m_constraintList;

         bool m_modified;

         QList<SpatialProperty*> m_properties;
         PointChargePotential* m_openBabelESP;
   };

} // end namespace Layer

typedef QList<Layer::Molecule*> MoleculeList;

} // end namespace IQmol

#endif
