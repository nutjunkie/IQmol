#ifndef IQMOL_SPATIALPROPERTY_H
#define IQMOL_SPATIALPROPERTY_H
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

#include "Function.h"
#include "Data.h"
#include "GridData.h"
#include "MultipoleExpansion.h"
#include "QGLViewer/vec.h"
#include <QList>


namespace IQmol {

   namespace AtomicDensity {
      class Base;
   }

   namespace Layer {
      class Molecule;
   }

   /// Base class for scalar properties that are defined over all space.
   class SpatialProperty {
      public:
         SpatialProperty(QString const& text = QString()) : m_text(text) { }

         virtual ~SpatialProperty() { }

         virtual QString text() const { return m_text; }
         void setText(QString const& text) { m_text = text; }

         virtual bool isAvailable() const { return true; }

         virtual Function3D const& evaluator() { return m_function; }

      protected:
         Function3D m_function;

      private:
         QString m_text;
   };



   /// Returns the distance from the origin, mainly for debugging purposes.
   class RadialDistance : public SpatialProperty {
      public:
         RadialDistance();

      private:
         double distance(double const x, double const y, double const z) const;
   };



   // This is a generalized promolecule density which is a molecular density
   // made up of any atomic parts.
   class PromoleculeDensity : public SpatialProperty {

      public:
         PromoleculeDensity(QString const& label, QList<AtomicDensity::Base*> atoms,
            QList<qglviewer::Vec> coordinates);

         ~PromoleculeDensity();

         bool isAvailable() const;

         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max) const; 
 
      private:
         static const double s_thresh;
         double rho(double const x, double const y, double const z);
         QList<AtomicDensity::Base*> m_atomicDensities;
         QList<qglviewer::Vec> m_coordinates;
   };



   class PointChargePotential : public SpatialProperty {
      public:
         PointChargePotential(Data::Type::ID type, QString const& label, 
            Layer::Molecule* molecule);

         Function3D const& evaluator();

      private:
         Layer::Molecule* m_molecule;
         Data::Type::ID m_type;
         QList<double> m_charges;
         QList<qglviewer::Vec> m_coordinates;
         double potential(double const x, double const y, double const z) const;
   };



   // !!!! Note that the evaluator call should be updating the positions and
   // orientations of the multipoles before returning.  The fact that it does
   // not means that if a DMA is loaded and the molecule is rotated before
   // evaluating the ESP, it will be incorrect.  I have left this class without
   // the update to the coordinates so that it is more obviously wrong until I
   // figure out how to rotate things properly.
   class MultipolePotential : public SpatialProperty {

      public:
         MultipolePotential(QString const& type, int const order, 
            Data::MultipoleExpansionList const& siteList);

      private:
         int m_order;
         Data::MultipoleExpansionList const& m_siteList;
         double potential(double const x, double const y, double const z) const;
   };



   class NearestNuclearCharge : public SpatialProperty {
      public:
         NearestNuclearCharge(QList<int> nuclearCharges = QList<int>(), 
            QList<qglviewer::Vec> coordinates = QList<qglviewer::Vec>());

         void update(QList<int> charges, QList<qglviewer::Vec> coordinates);

      private:
         QList<int> m_nuclearCharges;
         QList<qglviewer::Vec> m_coordinates;
         double nucleus(double const x, double const y, double const z) const;
   };


   class MeshIndex : public SpatialProperty {
      public:
         MeshIndex(QString const&);

      private:
         double index(double const x, double const y, double const z) const;
   };




   class GridBased : public SpatialProperty {
      public:
         GridBased(QString const& type, Data::GridData const& grid);

      private:
         Data::GridData const& m_grid;
         double evaluate(double const x, double const y, double const z) const;
   };

} // end namespace IQmol

#endif
