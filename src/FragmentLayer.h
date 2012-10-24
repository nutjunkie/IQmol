#ifndef IQMOL_FRAGMENTLAYER_H
#define IQMOL_FRAGMENTLAYER_H
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

#include "AtomLayer.h"
#include "BondLayer.h"
#include "ChargeLayer.h"
#include "QGLViewer/manipulatedFrame.h"
#include <QList>


namespace IQmol {

class Viewer;

namespace Layer {

   /// Primitive class that represents a Fragment which is a group of Primitives.
   class Fragment : public Primitive {

      Q_OBJECT

      public:
         Fragment(PrimitiveList const& primitives);

         void draw(qglviewer::Vec const& cameraPosition);
         void drawFast(qglviewer::Vec const& cameraPosition);
         void drawSelected(qglviewer::Vec const& cameraPosition);

         void select();
         void deselect();

		 /// Renumbers the atoms in the fragment starting from first.  Returns
		 /// the number of atoms in the fragment.  Note that this can be
		 /// applied to nested fragments.
         void renumberAtomsFrom(int& index);

         void group(PrimitiveList const& primitives);
         PrimitiveList ungroup();

      private Q_SLOTS:
		 /// Passes an ungroup Fragment action from the context menu to the
		 /// parent molecule.
         void ungroupAction();

      private:
         static int s_count;
         qglviewer::ManipulatedFrame m_frame;

		 // We need to keep these separated as that's how they appear in the
		 // ModelView.
         Layer::Atoms   m_atomList;
         Layer::Bonds   m_bondList;
         Layer::Charges m_chargeList;
   };

} 

typedef QList<Layer::Fragment*> FragmentList;

} // end namespace IQmol::Layer

#endif
