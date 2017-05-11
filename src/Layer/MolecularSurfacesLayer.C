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

#include "MolecularSurfacesLayer.h"
#include "MoleculeLayer.h"
#include "AtomicDensity.h"
#include "SpatialProperty.h"
#include "SurfaceInfo.h"
#include "SurfaceLayer.h"
#include "SurfaceGenerator.h"
#include "GridEvaluator.h"
#include "QsLog.h"

#include <QTime>
#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

MolecularSurfaces::MolecularSurfaces(Layer::Molecule& molecule) : Base("Surfaces"), 
   m_molecule(molecule)
{
   connect(&m_configurator, SIGNAL(calculateSurface(Data::SurfaceInfo const&)),
      this, SLOT(addToQueue(Data::SurfaceInfo const&)));

   connect(&m_configurator, SIGNAL(accepted()),
      this, SLOT(processSurfaceQueue()));

   setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

   m_configurator.sync();
   setConfigurator(&m_configurator);
}


void MolecularSurfaces::addToQueue(Data::SurfaceInfo const& info) 
{ 
   m_surfaceInfoQueue.append(info); 
}


void MolecularSurfaces::processSurfaceQueue()
{
   Qt::CheckState checked(Qt::Checked);
   SurfaceInfoQueue::iterator iter;
   for (iter = m_surfaceInfoQueue.begin(); iter != m_surfaceInfoQueue.end(); ++iter) {

       Data::Surface* surfaceData(0);

       switch ((*iter).type().kind()) {
          case Data::SurfaceType::VanDerWaals:
             surfaceData = calculateVanDerWaals(*iter);
             break;
          case Data::SurfaceType::Promolecule:
             surfaceData = 
                calculateSuperposition<AtomicDensity::AtomShellApproximation>(*iter);
             break;
          case Data::SurfaceType::SID:
             surfaceData = 
                calculateSuperposition<AtomicDensity::SuperpositionIonicDensities>(*iter, true);
             break;
          default:
             QLOG_WARN() << "Unsupported surface request in Layer::MolecularSurfaces";
             break;
       }
 
       if (surfaceData) {
          Layer::Surface* surfaceLayer(new Layer::Surface(*surfaceData));
          if (surfaceLayer) {
             surfaceLayer->setCheckState(checked);
             checked = Qt::Unchecked;
             connect(surfaceLayer, SIGNAL(updated()), this, SIGNAL(updated()));
             surfaceLayer->setFrame(m_molecule.getReferenceFrame());

             QString text((*iter).type().toString());
             surfaceLayer->setText(text);
             text += "\nScale = " + QString::number((*iter).isovalue());
             surfaceLayer->setToolTip(text);

             appendLayer(surfaceLayer);
          }
       }
   }

   m_surfaceInfoQueue.clear(); 
   m_molecule.updated();
}


Data::Surface* MolecularSurfaces::calculateVanDerWaals(Data::SurfaceInfo const& surfaceInfo)
{
   // The isovalue is actually a scale factor applied to the vdW radii
   double scale(surfaceInfo.isovalue());

   QList<int> atomIndices;
   QList<AtomicDensity::VanDerWaals*> vdwAtoms;
   AtomicDensity::VanDerWaals* atom;

   AtomList atoms(m_molecule.findLayers<Atom>(Children));
   if (atoms.isEmpty()) return 0;
   AtomList::iterator iter;

   int i(0);
   for (iter = atoms.begin(); iter != atoms.end(); ++iter, ++i) {
       atom = new AtomicDensity::VanDerWaals( (*iter)->getAtomicNumber(), 
                      (*iter)->getPosition(), scale);
       vdwAtoms.append(atom);
       //atomIndices.append(i);
       atomIndices.append((*iter)->getAtomicNumber()-1);
   }

   Data::Surface* surfaceData(new Data::Surface(surfaceInfo));

   i = 0; 
   QList<AtomicDensity::VanDerWaals*>::iterator iter2;
   for (iter2 = vdwAtoms.begin(); iter2 != vdwAtoms.end(); ++iter2, ++i) {
       Data::Mesh mesh((*iter2)->generateMesh(surfaceInfo.quality(), vdwAtoms));
       mesh.setMeshIndex(atomIndices[i]);
       surfaceData->meshPositive() += mesh;
   }

   for (iter2 = vdwAtoms.begin(); iter2 != vdwAtoms.end(); ++iter2) {
       delete (*iter2);
   }

   return surfaceData;
}


template <class T>
Data::Surface* MolecularSurfaces::calculateSuperposition(Data::SurfaceInfo const& surfaceInfo, 
   bool includeCharges)
{
   QMap<int, T*> uniqueAtoms;;
   QList<AtomicDensity::Base*> atomList;
   QList<Vec> coordinates;
   int atomicNumber;
   T* atom(0);

   AtomList atoms(m_molecule.findLayers<Atom>(Children));
   if (atoms.isEmpty()) return 0;
   AtomList::iterator iter;

   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       atomicNumber = (*iter)->getAtomicNumber();

       if (includeCharges) {
          atom = new T(atomicNumber);
          atom->setCharge((*iter)->getCharge());
          uniqueAtoms.insert((*iter)->getIndex(), atom);
       }else {
          if (uniqueAtoms.contains(atomicNumber)) {
             atom = uniqueAtoms[atomicNumber];
          }else {
             atom = new T(atomicNumber);
             uniqueAtoms.insert(atomicNumber, atom);
          }
       }

       coordinates.append( (*iter)->getPosition() );
       atomList.append(atom); 
   }

   PromoleculeDensity rho("Superposition", atomList, coordinates);
   
   Vec min, max;
   rho.boundingBox(min, max);

   Data::GridSize gridSize(min, max, surfaceInfo.quality());
   Data::GridData grid(gridSize, surfaceInfo.type());

   GridEvaluator gridEvaluator(grid, rho.evaluator());
   gridEvaluator.start();
   gridEvaluator.wait();

   Grid::SurfaceGenerator surfaceGenerator(grid, surfaceInfo);
   surfaceGenerator.start();
   surfaceGenerator.wait();
   return surfaceGenerator.getSurface();

   //Promolecule d'tor deletes atoms... dodgy.
}


} } // end namespace IQmol::Layer
