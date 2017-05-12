#ifndef IQMOL_LAYER_EFPFRAGMENTLIST_H
#define IQMOL_LAYER_EFPFRAGMENTLIST_H
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

#include "PrimitiveLayer.h"
#include "EfpFragmentListConfigurator.h"


namespace IQmol {
namespace Layer {

   class EfpFragment;

   class EfpFragmentList : public Base {

      Q_OBJECT

      using Base::appendLayer;

      friend class Configurator::EfpFragmentList;

      public:
         explicit EfpFragmentList(Layer::Base* parent);
         void appendLayer(EfpFragment*);

      protected:
         void setAtomScale(double const scale);
         void setBondScale(double const scale);
         void setDrawMode(Primitive::DrawMode const drawMode);

         double m_atomScale;
         double m_bondScale;
         Primitive::DrawMode m_drawMode;

      private:
         Configurator::EfpFragmentList m_configurator;
   };


} } // end namespace IQmol::Layer

#endif
