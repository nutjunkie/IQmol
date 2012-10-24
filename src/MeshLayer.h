#ifndef IQMOL_MESHLAYER_H
#define IQMOL_MESHLAYER_H
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

#include "GlobalLayer.h"
#include "MeshConfigurator.h"


namespace IQmol {
namespace Layer {

   /// Representation of a mesh drawn in the cartesian planes which aids visual
   /// orientation.
   class Mesh : public Global {

      Q_OBJECT

      friend class Configurator::Mesh;

      public:
         Mesh();
         void init();
         void draw();

      protected:
         bool   m_xy, m_xz, m_yz;
         bool   m_polar;    
         bool   m_useStepSize;
         int    m_ticks;
         double m_stepSize;
         double m_lineWidth;

      private:
         void drawCartesian(double const stepSize, int const nSteps);
         void drawPolar(double const stepSize, int const nSteps);
         void drawRadialLines(double const inner, double const outer);

         Configurator::Mesh m_configurator;
   };

} } // end namespace IQmol::Layer

#endif
