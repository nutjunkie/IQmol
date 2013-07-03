#ifndef IQMOL_SPATIALPROPERTY_H
#define IQMOL_SPATIALPROPERTY_H
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

#include "IQmol.h"
#include <QList>
#include "MultipoleExpansion.h"
#include "QGLViewer/vec.h"
#include "boost/bind.hpp"
#include "boost/function.hpp"


namespace IQmol {

   class Grid;

   namespace AtomicDensity {
      class Base;
   }

   namespace Layer {
      class Molecule;
   }


   /// Several classes that represent molecular properties than have a 
   /// spatially dependent value.
   class SpatialProperty {
      public:
         SpatialProperty() { }
         virtual ~SpatialProperty() { }
         virtual QString text() const = 0;
         virtual Function3D evaluator() = 0;
         virtual bool isAvailable() const = 0;
   };



   class RadialDistance : public SpatialProperty {
      public:
         QString text() const { return "Radial Distance"; }
         Function3D evaluator() {
            return boost::bind(&RadialDistance::distance, this, _1, _2, _3);
         }
         bool isAvailable() const { return true; }
   
      private:
         double distance(double const x, double const y, double const z);
   };



   // This is a generalized promolecule density which is a molecular density
   // which is made up of any atomic parts.
   class PromoleculeDensity : public SpatialProperty {

      public:
         PromoleculeDensity(QString const& label, QList<AtomicDensity::Base*> atoms,
            QList<qglviewer::Vec> coordinates);
         ~PromoleculeDensity();
         QString text() const { return m_label; }
         Function3D evaluator() {
            return boost::bind(&PromoleculeDensity::rho, this, _1, _2, _3);
         }
         bool isAvailable() const;
         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max); 
 
      private:
         static const double s_thresh;
         double rho(double const x, double const y, double const z);
         QString m_label;
         QList<AtomicDensity::Base*> m_atomicDensities;
         QList<qglviewer::Vec> m_coordinates;
   };


  
   class PointChargePotential : public SpatialProperty {
      public:
         PointChargePotential(QString const& type, Layer::Molecule* molecule) 
          : m_text(type), m_molecule(molecule) { }

         QString text() const { return m_text; }
         bool isAvailable() const { return true; }
         Function3D evaluator();

      private:
         QString m_text;
         Layer::Molecule* m_molecule;
         QList<double> m_charges;
         QList<qglviewer::Vec> m_coordinates;
         double  potential(double const x, double const y, double const z);
   };


   // !!!! Note that the evaluator call should be updating the positions and
   // orientations of the multipoles before returning.  The fact that it does
   // not means that if a DMA is loaded and the molecule is rotated before
   // evaluating the ESP, it will be incorrect.  I have left this class without
   // the update to the coordinates so that it is 'more wrong' and the bug is
   // not so subtle.
   class MultipolePotential : public SpatialProperty {

      public:
         MultipolePotential(QString type, int const order, 
            Data::MultipoleExpansionList* siteList) : m_text(type), m_order(order), 
            m_siteList(siteList) { }

         ~MultipolePotential();

         QString text() const { return m_text; }
         bool isAvailable() const { return true; }

         Function3D evaluator() {
            return boost::bind(&MultipolePotential::potential, this, _1, _2, _3);
         }

      private:
         QString m_text;
         int m_order;
         Data::MultipoleExpansionList* m_siteList;
         double  potential(double const x, double const y, double const z);
   };


   class NearestNuclearCharge : public SpatialProperty {
      public:
         NearestNuclearCharge(QList<int> nuclearCharges = QList<int>(), 
            QList<qglviewer::Vec> coordinates = QList<qglviewer::Vec>()) {
            update(nuclearCharges, coordinates);
         }

         QString text() const { return "Nearest nucleus"; }
         bool isAvailable() const { return true; }
         Function3D evaluator() {
            return boost::bind(&NearestNuclearCharge::nucleus, this, _1, _2, _3);
         }
         void update(QList<int> charges, QList<qglviewer::Vec> coordinates);

      private:
         QList<int> m_nuclearCharges;
         QList<qglviewer::Vec> m_coordinates;
         double nucleus(double const x, double const y, double const z);
   };



   class GridBased : public SpatialProperty {
      public:
         GridBased(QString type, Grid* grid) : m_text(type), m_grid(grid) { }

         QString text() const { return m_text; }
         bool isAvailable() const { return true; }
         Function3D evaluator() {
            return boost::bind(&GridBased::evaluate, this, _1, _2, _3);
         }

      private:
         QString m_text;
         Grid* m_grid;
         double evaluate(double const x, double const y, double const z);
   };

} // end namespace IQmol

#endif
