#ifndef IQMOL_MANIPULATEDFRAMESETCONSTRAINT_H
#define IQMOL_MANIPULATEDFRAMESETCONSTRAINT_H
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

#include "QGLViewer/constraint.h"


namespace IQmol {

   namespace Layer {
      class GLObject;
   }

   /// Grouping class which allows a set of GLObjects to be manipulated
   /// independent of the world frame.
   class ManipulatedFrameSetConstraint : public qglviewer::AxisPlaneConstraint {
      public:
         void clearSet();
         void addObjectToSet(Layer::GLObject* object);
         void constrainRotation(qglviewer::Quaternion &rotation, Layer::GLObject* object);

         virtual void constrainTranslation(qglviewer::Vec &translation, 
            qglviewer::Frame *const frame);
         virtual void constrainRotation(qglviewer::Quaternion &rotation, 
            qglviewer::Frame *const frame);

      private:
         QList<Layer::GLObject*> m_objects;  // Objects in the frame set
   };

} // end namespace IQmol

#endif
