/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
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

#include "QsLog.h"
#include "Constants.h"
#include "SpatialProperty.h"
#include "AtomicDensity.h"
#include "MoleculeLayer.h"

#include <QDebug>


using namespace qglviewer;

namespace IQmol {

// --------------- RadialDisatance ---------------
RadialDistance::RadialDistance() : SpatialProperty("Radial Distance") 
{ 
   m_function = boost::bind(&RadialDistance::distance, this, _1, _2, _3);
}


double RadialDistance::distance(double const x, double const y, double const z) const
{
   return std::sqrt(x*x+y*y+z*z);
}



// --------------- PromoleculeDensity ---------------

double const PromoleculeDensity::s_thresh = 0.001; // for the bounding box limits

PromoleculeDensity::PromoleculeDensity(QString const& label, QList<AtomicDensity::Base*> atoms,
   QList<Vec> coordinates) : SpatialProperty(label), m_atomicDensities(atoms), 
   m_coordinates(coordinates)
{
   m_function = boost::bind(&PromoleculeDensity::rho, this, _1, _2, _3);
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


void PromoleculeDensity::boundingBox(Vec& min, Vec& max) const
{
    if (m_coordinates.isEmpty()) {
       min.setValue(0.0, 0.0, 0.0);
       max.setValue(0.0, 0.0, 0.0);
       return;
    }

    min = m_coordinates[0];
    max = m_coordinates[0];
    Vec v;

    for (int i = 0; i < m_atomicDensities.size(); ++i) {
        double r(m_atomicDensities[i]->computeSignificantRadius(s_thresh));
        Vec v(m_coordinates[i]);
        min.x = std::min(double(min.x), v.x-r);
        min.y = std::min(double(min.y), v.y-r);
        min.z = std::min(double(min.z), v.z-r);
        max.x = std::max(double(max.x), v.x+r);
        max.y = std::max(double(max.y), v.y+r);
        max.z = std::max(double(max.z), v.z+r);
   }
}



// --------------- PointChargePotential ---------------
PointChargePotential::PointChargePotential(QString const& type, Layer::Molecule* molecule) 
  : SpatialProperty(type), m_molecule(molecule) 
{ 
   // call this to initialize m_function, otherwise a null function gets passed around
   evaluator();
}


Function3D const& PointChargePotential::evaluator() 
{
   // update the data first
qDebug() << "updating charges for eSP";
   m_coordinates = m_molecule->coordinates();
   m_charges = m_molecule->atomicCharges();

qDebug() << m_charges;
   if (m_charges.size() != m_coordinates.size()) {
      QLOG_ERROR() << "Unequal atom list lengths passed to PointChargePotential";
      return NullFunction3D;
   }
   m_function = boost::bind(&PointChargePotential::potential, this, _1, _2, _3);
   return m_function;
}


double PointChargePotential::potential(double const x, double const y, double const z) const
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



// --------------- MultipolePotential ---------------
MultipolePotential::MultipolePotential(QString const& type, int const order, 
   Data::MultipoleExpansionList* siteList) : SpatialProperty(type), m_order(order), 
   m_siteList(siteList) 
{ 
   m_function = boost::bind(&MultipolePotential::potential, this, _1, _2, _3);
}


MultipolePotential::~MultipolePotential() 
{ 
   delete m_siteList; 
}


double MultipolePotential::potential(double const x, double const y, double const z) const
{
   double esp(0.0);
   double tmp, R2, s, ir1, ir2, ir3, ir5, ir7;
   Vec pos(x, y, z);
   Vec R;

   Data::MultipoleExpansionList::const_iterator site;

   for (site = m_siteList->begin(); site != m_siteList->end(); ++site) {
       R   = pos-(*site)->position();
       R  *= Constants::AngstromToBohr;
       R2  = R.squaredNorm();
       ir1 = 1.0/R.norm();
       ir2 = ir1*ir1;
       ir3 = ir1*ir2;
       ir5 = ir3*ir2;
       ir7 = ir5*ir2;
       
       if (m_order >= 0) { // charge
          esp += (*site)->moment(Data::MultipoleExpansion::Q) * ir1;
       }
       if (m_order >= 1) { // dipole
          tmp  = (*site)->moment(Data::MultipoleExpansion::X) * R.x;
          tmp += (*site)->moment(Data::MultipoleExpansion::Y) * R.y;
          tmp += (*site)->moment(Data::MultipoleExpansion::Z) * R.z;
          esp += tmp * ir3;
       }
       if (m_order >= 2) { // quadrupole
          tmp  = (*site)->moment(Data::MultipoleExpansion::XX) * (3.0*R.x*R.x - R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::YY) * (3.0*R.y*R.y - R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::ZZ) * (3.0*R.z*R.z - R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::XY) * (3.0*R.x*R.y);
          tmp += (*site)->moment(Data::MultipoleExpansion::XZ) * (3.0*R.x*R.z);
          tmp += (*site)->moment(Data::MultipoleExpansion::YZ) * (3.0*R.y*R.z);
          esp += 0.5*tmp*ir5;
       } 
       if (m_order >= 3) { // octopole
          tmp  = (*site)->moment(Data::MultipoleExpansion::XYZ) * (30.0*R.x*R.y*R.z);
          s    = 5.0*R.x*R.x;
          tmp += (*site)->moment(Data::MultipoleExpansion::XXX) *     R.x*(s - 3.0*R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::XXY) * 3.0*R.y*(s -     R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::XXZ) * 3.0*R.z*(s -     R2);
          s    = 5.0*R.y*R.y;
          tmp += (*site)->moment(Data::MultipoleExpansion::XYY) * 3.0*R.x*(s -     R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::YYY) *     R.y*(s - 3.0*R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::YYZ) * 3.0*R.z*(s -     R2);
          s    = 5.0*R.z*R.z;
          tmp += (*site)->moment(Data::MultipoleExpansion::XZZ) * 3.0*R.x*(s -     R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::YZZ) * 3.0*R.y*(s -     R2);
          tmp += (*site)->moment(Data::MultipoleExpansion::ZZZ) *     R.z*(s - 3.0*R2);
          esp += 0.5*tmp*ir7;
       }
   }
       
   return esp;
}


// --------------- NearestNuclearCharge ---------------

NearestNuclearCharge::NearestNuclearCharge(QList<int> nuclearCharges,
   QList<qglviewer::Vec> coordinates) : SpatialProperty("Nearest nucleus")
{
   update(nuclearCharges, coordinates);
}


void NearestNuclearCharge::update(QList<int> nuclearCharges, QList<qglviewer::Vec> coordinates) 
{
   if (nuclearCharges.size() == coordinates.size()) {
      m_nuclearCharges = nuclearCharges;
      m_coordinates = coordinates;
      m_function = boost::bind(&NearestNuclearCharge::nucleus, this, _1, _2, _3);
   }else {
      QLOG_ERROR() << "Unequal atom list lengths passed";
   }
}


double NearestNuclearCharge::nucleus(double const x, double const y, double const z) const
{
   if (m_nuclearCharges.isEmpty())  return 1.0;  // set to hydrogen

   Vec pos(x, y, z);
   double dmin((m_coordinates[0]-pos).norm());
   double Z(m_nuclearCharges[0]);
   double d;

   for (int i = 1; i < m_nuclearCharges.size(); ++i) {
       d = (m_coordinates[i] - pos).norm();
       if (d < dmin) {
          Z = m_nuclearCharges[i];
          dmin = d;
       }
   }

   return Z;
}


// --------------- GridBased ---------------

GridBased::GridBased(QString const& type, Data::GridData const& grid) : SpatialProperty(type), 
   m_grid(grid)
{
   m_function = boost::bind(&GridBased::evaluate, this, _1, _2, _3);
}


double GridBased::evaluate(double const x, double const y, double const z) const
{
   bool ambiguousWeirdnessFUBAR(true);
   return m_grid(x, y, z, ambiguousWeirdnessFUBAR);
}

} // end namespace IQmol
