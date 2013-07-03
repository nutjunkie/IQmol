#ifndef IQMOL_CONFORMERLISTLAYER_H
#define IQMOL_CONFORMERLISTLAYER_H
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

#include "QGLViewer/vec.h"
#include "DataLayer.h"
#include "AtomLayer.h"
#include "Animator.h"
#include "ConformerListConfigurator.h"


namespace IQmol {
namespace Layer {

   class ConformerList : public Data {

      Q_OBJECT

      friend class Configurator::ConformerList;

      public:
         ConformerList(QList<Conformer*> const& conformers, Conformer* defaultConformer = 0);
         void setMolecule(Molecule*);
         ~ConformerList();

      Q_SIGNALS:
         void pushAnimators(AnimatorList const&);
         void popAnimators(AnimatorList const&);
         void update();

      public Q_SLOTS:
         void configure();
         void setActiveConformer(Conformer const&);
         void setDefaultConformer();

      protected:
         void setPlay(bool const play);
         void setLoop(bool const loop);
         void setBounce(bool const bounce);
         void setSpeed(double const speed);
         void reperceiveBonds(bool const tf);

      private:
         void makeAnimators();
         void deleteAnimators();
         Configurator::ConformerList m_configurator;
         AnimatorList m_animatorList;
         Conformer* m_defaultConformer;
         double m_speed;
         bool m_reperceiveBonds;
         int  m_nConformers;
         bool m_bounce;
         bool m_loop;
   };


   /// The Conformer class is to agregate data objects which are valid for a
   /// particular geometry.  These include such things as coordinates, energy
   /// and partial charges. 
   class Conformer : public Base {

      Q_OBJECT

      friend class ConformerList;

      public:
         Conformer(AtomList const&, double const energy = 0.0);
         Conformer(QList<qglviewer::Vec> const& coordinates,  double const energy = 0.0);

         bool updateAtoms(AtomList&) const;
         void setCharges(QList<double> const& partialCharges, 
            QList<double> const& spinDensities = QList<double>());
         void setMultipoles(QList<double> const& multipoleMoments);
         double getEnergy() const { return m_energy; }
         qglviewer::Vec const& getDipole() const { return m_dipole; }
         void setEnergy(double const energy);
         unsigned int numberOfAtoms() const { return m_coordinates.size(); }

      protected:
         double m_energy;
         QList<qglviewer::Vec> m_coordinates;

      private:
         void computeMoments();  // NYI - something to think about

         QList<double> m_partialCharges;
         QList<double> m_spinDensities;

         double m_charge;
         qglviewer::Vec m_dipole;
         QList<double>  m_quadrupole;
   };


} } // end namespace IQmol::Layer

#endif
