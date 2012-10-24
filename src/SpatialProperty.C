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

#include "Grid.h"
#include "QsLog.h"
#include "SpatialProperty.h"
#include "AtomicDensity.h"
#include <openbabel/mol.h>  // for OpenBabel::etab


using namespace qglviewer;

namespace IQmol {

// --------------- RadialDistance ---------------

double RadialDistance::distance(double const x, double const y, double const z)
{
   return std::sqrt(x*x+y*y+z*z);
}



// --------------- PromoleculeDensity ---------------

double const PromoleculeDensity::s_thresh = 0.001; // for the bounding box limits

PromoleculeDensity::PromoleculeDensity(QString const& label, QList<AtomicDensity::Base*> atoms,
   QList<Vec> coordinates)
  : m_label(label), m_atomicDensities(atoms), m_coordinates(coordinates)
{
}


PromoleculeDensity::~PromoleculeDensity()
{
   QSet<AtomicDensity::Base*> uniqueAtoms;
   QList<AtomicDensity::Base*>::iterator iter;

   for (iter = m_atomicDensities.begin(); iter != m_atomicDensities.end(); ++iter) {
       uniqueAtoms.insert(*iter);
   }

   QSet<AtomicDensity::Base*>::iterator atom;
   for (atom = uniqueAtoms.begin(); atom != uniqueAtoms.end(); ++atom) {
       delete (*atom);
   }
}


bool PromoleculeDensity::isAvailable() const
{
   bool available(true);
   QList<AtomicDensity::Base*>::const_iterator iter;
   for (iter = m_atomicDensities.begin(); iter != m_atomicDensities.end(); ++iter) {
       available = available && (*iter)->isAvailable();
   }
   return available;
}


double PromoleculeDensity::rho(double const x, double const y, double const z)
{
   double density(0.0);
   Vec p(x,y,z);

   for (int i = 0; i < m_atomicDensities.size(); ++i) {
       density +=  m_atomicDensities[i]->density(p - m_coordinates[i]);
   }

   return density;
}

void PromoleculeDensity::boundingBox(Vec& min, Vec& max) 
{
    min = m_coordinates[0];
    max = m_coordinates[0];
    double r;
    Vec v;

    for (int i = 0; i < m_atomicDensities.size(); ++i) {
        r = m_atomicDensities[i]->computeSignificantRadius(s_thresh);
        v = m_coordinates[i];
        min.x = std::min(min.x, v.x-r);
        min.y = std::min(min.y, v.y-r);
        min.z = std::min(min.z, v.z-r);
        max.x = std::max(max.x, v.x+r);
        max.y = std::max(max.y, v.y+r);
        max.z = std::max(max.z, v.z+r);
   }
}


// --------------- PointChargePotential ---------------

void PointChargePotential::update(QList<double> charges, QList<qglviewer::Vec> coordinates) {
   if (charges.size() == coordinates.size()) {
      m_charges = charges;
      m_coordinates = coordinates;
   }else {
      QLOG_ERROR() << "Unequal atom list lengths passed";
   }
}


double PointChargePotential::potential(double const x, double const y, double const z)
{
   double esp(0.0);
   double d;
   Vec pos(x, y, z);

   for (int i = 0; i < m_charges.size(); ++i) {
       d = (m_coordinates[i] - pos).norm();
       esp += m_charges[i]/d;
   }
   return esp;
}


// --------------- GridBased ---------------

double GridBased::evaluate(double const x, double const y, double const z)
{
   Vec pos(x, y, z);
   // this should be re-engineered to ensure m_grid doesn't get deleted
   return m_grid ? (*m_grid)(pos) : 0.0;
}

} // end namespace IQmol
