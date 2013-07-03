#ifndef IQMOL_EFPFRAGMENTLISTLAYER_H
#define IQMOL_EFPFRAGMENTLISTLAYER_H
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

#include "PrimitiveLayer.h"
#include "EFPFragmentListConfigurator.h"


namespace IQmol {
namespace Layer {

   class EFPFragment;

   class EFPFragmentList : public Base {

      Q_OBJECT

      friend class Configurator::EFPFragmentList;

      public:
		 EFPFragmentList(Base* parent);
         void appendLayer(EFPFragment*);

      Q_SIGNALS:
         void updated();

      protected:
         void setAtomScale(double const scale);
         void setBondScale(double const scale);
         void setDrawMode(Primitive::DrawMode const drawMode);

         double m_atomScale;
         double m_bondScale;
         Primitive::DrawMode m_drawMode;

      private:
         Configurator::EFPFragmentList m_configurator;
   };


} } // end namespace IQmol::Layer

#endif
