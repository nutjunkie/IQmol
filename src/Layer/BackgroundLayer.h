#ifndef IQMOL_BACKGROUNDLAYER_H
#define IQMOL_BACKGROUNDLAYER_H
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

#include "GlobalLayer.h"
#include "BackgroundConfigurator.h"


namespace IQmol {
namespace Layer {

   /// Simple Layer class representing the background.  This is implemented as a
   /// Layer so that the user can change the color via a Configurator.  Keeps
   /// things consistent.
   class Background : public Global {

      Q_OBJECT

      friend class Configurator::Background;

      public:
         Background();
         void draw();
         QColor color() const { return m_backgroundColor; }

      Q_SIGNALS:
         void foregroundColorChanged(QColor const&);

      protected:
         QColor m_backgroundColor;
         QColor m_foregroundColor;

      private:
         void drawGradient();
         void drawCircleGradient();
         Configurator::Background m_configurator;
   };

} } // end namespace IQmol::Layer

#endif
