#ifndef IQMOL_LAYER_GLOBAL_H
#define IQMOL_LAYER_GLOBAL_H
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
#include "Layer.h"

namespace IQmol {
namespace Layer {

   /// Abstract Base Layer for global display Layers such as the Axes, Mesh and 
   /// Background.  
   class Global : public Base {

      Q_OBJECT

      public:
         explicit Global(QString const& text = QString(), QObject* parent = 0) 
            : Base(text, parent), m_sceneRadius(DefaultSceneRadius)
         { 
            setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            setCheckState(Qt::Unchecked);
         }

         virtual ~Global() { }
         virtual void draw() = 0;

     public Q_SLOTS:
         void setSceneRadius(double radius) { m_sceneRadius = radius; }

      protected:
         double m_sceneRadius;
   };

} } // end namespace IQmol::Layer

#endif
