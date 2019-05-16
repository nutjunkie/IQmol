#ifndef IQMOL_LAYER_DIPOLE_H
#define IQMOL_LAYER_DIPOLE_H
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

#include "GLObjectLayer.h"
#include "DipoleConfigurator.h"
#include "OpenGL.h"
#include <QColor>


//2.541746230211

namespace IQmol {
namespace Layer {

   class Molecule;

   /// Layer representing a dipole moment.  
   class Dipole : public GLObject {

      Q_OBJECT;

      friend class Configurator::Dipole;

      public:
         Dipole(qglviewer::Vec const& dipole = qglviewer::Vec(0.0, 0.0, 0.0));

         double value() const;

         qglviewer::Vec const& vector() const { return m_dipoleMoment; }

         void draw();
         void drawFast();
         void drawSelected() { }

         void setMolecule(Molecule*);

      public Q_SLOTS:
         void setRadius(double const radius); 
         void setValue(qglviewer::Vec const& dipole);

      protected:
         QColor m_color;
         double m_scale;  // user scaling [0,1] relative to sceneRadius;

      private:
         void drawPrivate();

         void drawArrow(qglviewer::Vec const& from, qglviewer::Vec const& to, 
            int const resolution = 24);
         void drawArrow(float length, int const resolution);

         Configurator::Dipole m_configurator;
         qglviewer::Vec m_dipoleMoment;
         double m_sceneRadius;
   };

} } // end namespace IQmol::Layer

#endif
