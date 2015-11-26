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

#include "ManipulatedFrameSetConstraint.h"
#include "GLObjectLayer.h"


using namespace qglviewer;
using namespace IQmol::Layer;

namespace IQmol {


void ManipulatedFrameSetConstraint::clearSet() 
{
   m_objects.clear();
}


void ManipulatedFrameSetConstraint::addObjectToSet(GLObject* object) 
{
   m_objects.append(object);
}


void ManipulatedFrameSetConstraint::constrainRotation(Quaternion &rotation, GLObject* object) 
{
   constrainRotation(rotation, &(object->m_frame));
}


void ManipulatedFrameSetConstraint::constrainTranslation(Vec& translation, Frame* const) 
{
   // Filter the translation
   switch (translationConstraintType()) {
      case AXIS:
         translation.projectOnAxis(translationConstraintDirection());
         break;

      case PLANE:
         translation.projectOnPlane(translationConstraintDirection());
         break;

     default:
         break;
   }

   QList<GLObject*>::iterator iter, end;
   for (iter = m_objects.begin(), end = m_objects.end(); iter != end; ++iter) {
       (*iter)->m_frame.translate(translation);
   }
}


void ManipulatedFrameSetConstraint::constrainRotation(Quaternion &rotation, Frame *const frame) 
{
   // Rotation is expressed in the frame local coordinates system. Convert it
   // back to world coordinates.
   Vec worldAxis = frame->inverseTransformOf(rotation.axis());
   Vec pos = frame->position();
   float angle = rotation.angle();

   // Filter the rotation
   switch (rotationConstraintType()) {
      case AXIS:
         worldAxis = rotationConstraintDirection();
         break;

     default:
         break;
   }

   QList<GLObject*>::iterator iter, end;
   for (iter = m_objects.begin(), end = m_objects.end(); iter != end; ++iter) {
       // Rotation has to be expressed in the object local coordinates system.
        Quaternion qObject((*iter)->m_frame.transformOf(worldAxis), angle);
        (*iter)->m_frame.rotate(qObject);

       Quaternion qWorld(worldAxis, angle);
       (*iter)->setPosition(pos + qWorld.rotate((*iter)->m_frame.position() - pos));
   }
}



} // end namespace IQmol
