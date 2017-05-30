#ifndef IQMOL_LAYER_MOLECULE_H
#define IQMOL_LAYER_MOLECULE_H
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

#include "IQmol.h"
#include "Bank.h"
#include "InfoLayer.h"
#include "FileLayer.h"
#include "AtomLayer.h"
#include "BondLayer.h"
#include "ChargeLayer.h"
#include "ContainerLayer.h"
#include "EfpFragmentListLayer.h"
#include "MoleculeConfigurator.h"
#include "MolecularSurfacesLayer.h"
#include "SurfaceAnimatorDialog.h"
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

   namespace Process {
      class  QChemJobInfo;
   }

   namespace Command {
      class AppendData;
      class RemoveData;
      class EditPrimitives;
      class AddHydrogens;
      class MoveObjects;
      class AddMolecule;
      class RemoveMolecule;
      class ChangeBondOrder;
      class ChangeAtomType;
   }

   namespace Data {
      class Geometry;
      class Bank;
      class SurfaceInfo;
   }

   class SpatialProperty;
   class PointChargePotential;
   class NearestNuclearCharge;

   namespace Layer {

      class Constraint;
      class Surface;
      class Group;
      typedef QMap<OpenBabel::OBAtom*, Atom*>  AtomMap;
      typedef QMap<OpenBabel::OBBond*, Bond*>  BondMap;
      typedef QMap<OpenBabel::OBAtom*, Group*> GroupMap;
   
      /// Container Layer for all things related to a particular molecule.
      /// This is the main data structure for a molecule.  It contains the Primitve
      /// GLObjects that visually represent the molecule along with Data Layers
      /// containing information such as checkpoint files, cube files and output
      /// files.
      class Molecule : public Base {
   
         Q_OBJECT
   
         friend class Frequencies;
         friend class GeometryList; 
         friend class Configurator::Molecule;
         friend class Animator::Combo;
         friend class SurfaceAnimatorDialog;
   
         // !!! some of these are no longer required to be friends
         friend class Command::AppendData;
         friend class Command::RemoveData;
         friend class Command::EditPrimitives;
         friend class Command::AddHydrogens;
         friend class Command::MoveObjects;
         friend class Command::AddMolecule;
         friend class Command::RemoveMolecule;
         friend class Command::ChangeBondOrder;
         friend class Command::ChangeAtomType;
   
         public:
            explicit Molecule(QObject* parent = 0);

            ~Molecule();

            double radius();
   
      		/// Attempts to save the molecule, returning false if the operation
            /// was unsuccessful or canceled by the user.
            bool save(bool prompt = false);
   
            /// Appends the Layers in the DataList, but only if an existing Layer
            /// of that kind does not exist already.
            void appendData(IQmol::Data::Bank&);
            void appendData(Layer::List&);

            void appendSurface(Data::Surface*);

            void addProperty(SpatialProperty* property) { m_properties.append(property); }
   
            void setFile(QString const& fileName);
            QString fileName() const { return m_inputFile.fileName(); }
   
            Process::QChemJobInfo qchemJobInfo();
            void qchemJobInfoChanged(Process::QChemJobInfo const&);
   
            /// Attempts to determine the best axis for the functional group when
            /// converting an atom to a functional group (click on atom event)
            qglviewer::Vec getBuildAxis(Atom*);

            void addHydrogens();

            void symmetrize(double tolerance, bool updateCoordinates = true);

            void minimizeEnergy(QString const& forcefield);

            void translateToCenter(GLObjectList const& selection);
            static void toggleAutoDetectSymmetry() { 
               s_autoDetectSymmetry = !s_autoDetectSymmetry; 
            }
   
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
            BondList getBonds(Atom*);
            bool isModified() const { return m_modified; }
   
            qglviewer::Vec centerOfNuclearCharge();
            QStringList getAvailableProperties(); 
            Function3D getPropertyEvaluator(QString const& name);
   
            /// Removes the specified Primitive(s) from the molecule, 
            /// but does not delete them. 
            void takePrimitives(PrimitiveList const&);
            void takePrimitive(Primitive*);
   
            /// Adds the specified Primitive(s) to the molecule.
            void appendPrimitives(PrimitiveList const&);
            void appendPrimitive(Primitive*);
   
            Constraint* findMatchingConstraint(AtomList const&);
            bool canAcceptConstraint(Constraint*);
      		void addConstraint(Constraint*);

			// These simpy add or remove the Constraint layer to the model view
			// and are used by the undo commands.
            void addConstraintLayer(Constraint*);
            void removeConstraintLayer(Constraint*);
   
            /// Converts the Molecule to an XYZ format and uses OpenBabel to parse this.  
            /// Useful for, e.g., reperceiving bonds.
            QString coordinatesAsString(bool const selectedOnly = false);
            QString coordinatesAsStringFsm();
            QStringList coordinatesForCubeFile();
   
            /// Assigns the atom indices based on the ordering selected by 
            /// the user via the reorderIndex variable in the Atom class.
            void updateAtomOrder();

            Atom* createAtom(unsigned int const Z, qglviewer::Vec const& position);
            Bond* createBond(Atom* begin, Atom* end, int const order = 1);
            Charge* createCharge(double const q, qglviewer::Vec const& position);
   
            QList<qglviewer::Vec> coordinates();
            QList<double> atomicCharges(Data::Type::ID type);
            void setGeometry(IQmol::Data::Geometry&);

            // There must be a better way of doing this, but this is used
            // in Layer::GeometryList for de
            bool isCurrentGeometry(Data::Geometry const* geometry) const { 
               return m_currentGeometry == geometry;
            }
   
      		 // This is needed for fchk-file based surfaces and only covers ridgid
      		 // body motions of the molecule.
            qglviewer::Frame const& getReferenceFrame() const { return m_frame; }

            void setReferenceFrame(qglviewer::Frame const& frame) { m_frame = frame; }
   
            void setReperceiveBondsForAnimation(bool tf) {
                 m_reperceiveBondsForAnimation = tf;
            }

            unsigned maxAtomicNumber() { return m_maxAtomicNumber; }
   
   
         public Q_SLOTS:
            void groupSelection();
            void ungroupSelection();
            void applyConstraint(Constraint*);
            void constraintUpdated();
            void removeConstraint();
            void selectAll();
            void selectAtoms(QList<int> const& indices);
            void createGeometryList();

            void openSurfaceAnimator();

            void reperceiveBonds() { reperceiveBonds(true); }
            void reperceiveBondsForAnimation() { 
               if (m_reperceiveBondsForAnimation) reperceiveBonds(false);
            }
   
            /// Passes the remove signal on so that the ViewerModel can deal with it
            void removeMolecule() { removeMolecule(this); }
            void detectSymmetry();
            void autoDetectSymmetry();
            void invalidateSymmetry();
            void saveToCurrentGeometry();
   
         Q_SIGNALS:
            void softUpdate(); // issue if the number of primitives does not change
            void postCommand(QUndoCommand*);
            void postMessage(QString const&);
            void pushAnimators(AnimatorList const&);
            void popAnimators(AnimatorList const&);
            void removeMolecule(Layer::Molecule*);
   
            void multiplicityAvailable(unsigned);
            void chargeAvailable(int);
            void pointGroupAvailable(QString const&);
            void energyAvailable(double const, Info::EnergyUnit);
            void dipoleAvailable(qglviewer::Vec const& dipole, bool const estimated);
            void radiusAvailable(double const radius);
            void centerOfNuclearChargeAvailable(qglviewer::Vec const&);

            void select(QModelIndex const&, QItemSelectionModel::SelectionFlags);
   
   
         protected:
            void updateAtomScale(double const scale);
            void updateSmallerHydrogens(bool smallerHydrogens);
            void updateBondScale(double const scale);
            void updateChargeScale(double const scale);
            void updateDrawMode(Primitive::DrawMode const);
            void updateInfo();
   
            /// Updates the atom and bond indicies after, for example, deletion.
            void reindexAtomsAndBonds();
            void reperceiveBonds(bool postCmd);
   
         private Q_SLOTS:
            void dumpData() { m_bank.dump(); }
            void setAtomicCharges(Data::Type::ID type);
            void updateAtomicCharges();
   
         private:
            static bool s_autoDetectSymmetry;
            int totalCharge() const;
            int multiplicity() const;
            QString constraintsAsString();
            QString scanCoordinatesAsString();
            QString efpFragmentsAsString();
            QString efpParametersAsString();
            QString externalChargesAsString();

            template <class T>
            QList<double> atomicCharges();
            qglviewer::Vec dipoleFromPointCharges();

            QList<double> zeroCharges();
            QList<double> gasteigerCharges();


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
            OpenBabel::OBMol* toOBMol(AtomMap*, BondMap*, GroupMap* = 0);
   
            /// Loads the molecular information from an OBMol object.  If existing
            /// Atom and Bond Maps are specified, only the additional primitives
            /// (e.g. added hydrogens) are returned, otherwise a complete list of
            /// added primtives is returned.  The returned primitive list can be
            /// used for UndoCommands.  Note that this does not load all the data
            /// in the OBMol (e.g. frequency data), for that, Parser::OpenBabel
            /// should be used.
   		    PrimitiveList fromOBMol(OpenBabel::OBMol*, AtomMap* = 0, BondMap* = 0, 
               GroupMap* = 0);
   
            template <class T> void update(boost::function<void(T&)>);

            void translate(qglviewer::Vec const& displacement);

            void rotate(qglviewer::Quaternion const& rotation);

            void alignToAxis(qglviewer::Vec const& point, 
               qglviewer::Vec axis = qglviewer::Vec(0.0, 0.0, 1.0));

            void rotateIntoPlane(qglviewer::Vec const& point, 
               qglviewer::Vec const& axis = qglviewer::Vec(0.0, 0.0, 1.0),
               qglviewer::Vec const& normal = qglviewer::Vec(0.0, 1.0, 0.0));

            void clearData();
   
            void deleteProperties();

            void initProperties();

            void saveToGeometry(Data::Geometry&);
   
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
   
            /// Determines if the hydrogen atoms are drawn smaller in the CPK model
            bool m_smallerHydrogens;

            bool m_modified;
            bool m_reperceiveBondsForAnimation;
   
            Configurator::Molecule m_configurator;
            IQmol::SurfaceAnimatorDialog  m_surfaceAnimator;
            
            Layer::Info      m_info;
            Layer::Container m_atomList;
            Layer::Container m_bondList;
            Layer::Container m_chargesList;
            Layer::Container m_fileList;
            Layer::Container m_surfaceList;
            Layer::Container m_constraintList;
            Layer::Container m_scanList;
            Layer::Container m_groupList;
   
            Layer::EfpFragmentList m_efpFragmentList;
            Layer::MolecularSurfaces m_molecularSurfaces;
   
            QList<SpatialProperty*> m_properties;
            qglviewer::Frame m_frame;
            Data::Bank m_bank;

            Data::Geometry* m_currentGeometry;
            Data::Type::ID m_chargeType;
            QAction* m_atomicChargesMenu;
            unsigned m_maxAtomicNumber;
            QAction* m_addGeometryMenu;;
      };
   
   } // end namespace Layer

   typedef QList<Layer::Molecule*> MoleculeList;

} // end namespace IQmol

#endif
