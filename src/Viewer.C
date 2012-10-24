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

#include "Viewer.h"
#include "IQmol.h"
#include "QsLog.h"
#include "MoleculeLayer.h"
#include "Preferences.h"
#include "ManipulatedFrameSetConstraint.h"
#include <QStandardItem>
#include <QDropEvent>
#include <QtDebug>
#include <QUrl>
#include <cmath>
#include "gl2ps.h"


using namespace qglviewer;

namespace IQmol {

const Qt::Key Viewer::s_buildKey(Qt::Key_Alt);
const Qt::Key Viewer::s_selectKey(Qt::Key_Shift);
const Qt::Key Viewer::s_manipulateSelectionKey(Qt::Key_Control);
const Qt::KeyboardModifier Viewer::s_buildModifier(Qt::AltModifier);
const Qt::KeyboardModifier Viewer::s_selectModifier(Qt::ShiftModifier);
const Qt::KeyboardModifier Viewer::s_manipulateSelectionModifier(Qt::ControlModifier);

QFont  Viewer::s_labelFont("Helvetica", 14, QFont::Black);
QFontMetrics Viewer::s_labelFontMetrics(Viewer::s_labelFont);


//! Window set up is done here
Viewer::Viewer(ViewerModel& model, QWidget* parent) : QGLViewer(parent), 
   m_buildElement(6), 
   m_viewerModel(model), 
   m_selectionHighlighting(true),
   m_labelType(Layer::Atom::None),
   m_manipulateHandler(this),
   m_selectHandler(this),
   m_buildHandler(this),
   m_reindexAtomsHandler(this),
   m_manipulateSelectionHandler(this),
   m_snapper(0)
{ 
   setAcceptDrops(true);

   // Disable the default keybindings, the menu handles those we want
   setShortcut(DRAW_AXIS, 0);
   setShortcut(DRAW_GRID, 0);
   setShortcut(DISPLAY_FPS, 0);
   setShortcut(ENABLE_TEXT, 0);
   setShortcut(EXIT_VIEWER, 0);
   setShortcut(SAVE_SCREENSHOT, 0);
   setShortcut(CAMERA_MODE, 0);
   setShortcut(FULL_SCREEN, 0);
   setShortcut(STEREO, 0);
   setShortcut(ANIMATION, 0);
   setShortcut(HELP, 0);
   setShortcut(EDIT_CAMERA, 0);
   setShortcut(SNAPSHOT_TO_CLIPBOARD, 0);
   // Disable zoom selection, which makes no sense in our context
   setWheelBinding(Qt::MetaModifier, CAMERA, NO_MOUSE_ACTION);

   setActiveViewerMode(Build);  // this should get overwritten by the MainWindow class
   setSceneRadius(DefaultSceneRadius);
   resetView();
}



//! The OpenGL context is not available in the constructor, so all GL setup
//! must be done here.
void Viewer::init()
{
   makeCurrent();
   // A ManipulatedFrameSetConstraint will apply displacements to the selection
   setManipulatedFrame(new ManipulatedFrame());
   //setMouseTracking(true);

   manipulatedFrame()->setConstraint(new ManipulatedFrameSetConstraint());
   s_labelFont.setStyleStrategy(QFont::OpenGLCompatible);
   s_labelFont.setPointSize(Preferences::LabelFontSize());
   setForegroundColor(Preferences::ForegroundColor());

   // SpotLight
/* !!!
	glEnable(GL_LIGHT1);

	// Light default parameters
	const GLfloat light_ambient[4]  = {1.0, 1.0, 1.0, 1.0};
	const GLfloat light_specular[4] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat light_diffuse[4]  = {1.0, 1.0, 1.0, 1.0};

	glLightf( GL_LIGHT1, GL_SPOT_EXPONENT, 3.0);
	glLightf( GL_LIGHT1, GL_SPOT_CUTOFF,   10.0);
	glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.1f);
	glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.3f);
	glLightf( GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);
	glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);
*/
    
}


void Viewer::setSceneRadius(double const radius)
{
   float r = std::max(radius, DefaultSceneRadius);
   QGLViewer::setSceneRadius(r);
   Vec position(camera()->position());
   position = 3.0*r*position.unit();
   camera()->setPosition(position);
   updateGL();
}


void Viewer::resetView()
{
   setMouseBinding(Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
   double radius = std::max(DefaultSceneRadius, m_viewerModel.sceneRadius());

/*
   // do the interpolation in spherical coordinates to avoid slicing through the 
   // molecule. dunno how to get this working
   
   Vec p(camera()->position());
   double r(p.norm());
   double th(std::acos(p.z/r));
   double ph(std::atan2(p.y, p.x));

   int nSteps(10);
   // Interpolate to (0.0, 0.0, 3.0*radius);
   double step(1.0/nSteps);
   double dr((3.0*radius-r)*step);
   double dth((0.0-th)*step);
   double dph((0.0-ph)*step);

   Quaternion begin(camera()->orientation());
   Quaternion end;

   qDebug() << "Interpolating with steps " << dr << dth << dph;
   qDebug() << "Interpolation starting at" << p.x << p.y << p.z;
   for (int i = 1; i <= nSteps; ++i) {
       r += dr; th += dth; ph += dph;
       Vec v( r*std::sin(th)*std::cos(ph), r*std::sin(th)*std::sin(ph), r*std::cos(th));
       camera()->interpolateTo(Frame(v, Quaternion::slerp(begin, end, i*step)), 1.0);
       qDebug() << "Interpolating to" << v.x << v.y << v.z;
   }

*/

   // snapTo controls if the view is reset instantly, or via an animation.  At
   // the moment the animation pathway can be confusing, so we just snap to the
   // standard view.
   bool snapTo(true);
   if (snapTo) {
      camera()->setPosition( Vec(0.0f, 0.0, 3.0*radius) );
      camera()->setOrientation(0.0f, 0.0f);
      camera()->lookAt( Vec(0.0f, 0.0f, 0.0f) );
      updateGL();
   }else {
      camera()->interpolateTo(Frame(Vec(0.0, 0.0, 3.0*radius), Quaternion()), 1.0);
      m_viewerModel.sceneRadiusChanged(radius);
   }


}


Vec Viewer::worldCoordinatesOf(QMouseEvent* e, qglviewer::Vec const& hint)
{
   Camera* cam(camera());
   Vec v(cam->projectedCoordinatesOf(hint));
   v.setValue(e->x(), e->y(), v.z);
   v = cam->unprojectedCoordinatesOf(v);
   return v;
}


void Viewer::setLabelType(int const type)
{
   s_labelFont.setPointSize(Preferences::LabelFontSize());
   switch (type) {
      case Layer::Atom::Index:   m_labelType = Layer::Atom::Index;   break;
      case Layer::Atom::Element: m_labelType = Layer::Atom::Element; break;
      case Layer::Atom::Charge:  m_labelType = Layer::Atom::Charge;  break;
      case Layer::Atom::Mass:    m_labelType = Layer::Atom::Mass;    break;
      case Layer::Atom::Spin:    m_labelType = Layer::Atom::Spin;    break;
      case Layer::Atom::Reindex: m_labelType = Layer::Atom::Reindex; break;
      default:                   m_labelType = Layer::Atom::None;    break;
   }
   updateGL();
}


void Viewer::draw()
{
   makeCurrent();
/*
  Vec cameraPos(camera()->position());
  static const GLfloat pos[4] = {1.5, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, );
  static const GLfloat spotDir[3] = {-1.0, 0.0, 0.0};
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDir);

   bool spotLight = true;
   if (spotLight) {
      const Vec cameraPos = camera()->position();
      const GLfloat pos[4] = {cameraPos[0], cameraPos[1], cameraPos[2], 1.0};
      glLightfv(GL_LIGHT1, GL_POSITION, pos);

      // Orientate light along view direction
      glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, camera()->viewDirection());
   }
*/

   // Default material
   GLfloat mat_specular[] = { 0.9, 0.9, 0.9, 1.0 };
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
   GLfloat mat_shininess = 50.0;
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
   glShadeModel(GL_SMOOTH);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  

   // create the master object lists, do the surfaces neeed to be filtered out?
   m_objects = m_viewerModel.getVisibleObjects();
   m_selectedObjects = m_viewerModel.getSelectedObjects();

   Vec cameraPosition(camera()->position());

   drawGlobals();
   drawObjects(m_objects, cameraPosition);
   drawSelected(m_selectedObjects, cameraPosition);
   drawObjects(m_buildObjects, cameraPosition);
  
   if (m_labelType != Layer::Atom::None) drawLabels(m_objects);
   if (m_selectionMode != None) drawSelectionRectangle(m_selectHandler.region());
   displayGeometricParameter(m_selectedObjects);

   // drawLight(GL_LIGHT0);
}


void Viewer::drawObjects(GLObjectList const& objects, Vec const& cameraPosition)
{
   GLObjectList::const_iterator object;
   for (object = objects.begin(); object != objects.end(); ++object) {
       (*object)->draw(cameraPosition);
   }
}


void Viewer::drawSelected(GLObjectList const& objects, Vec const& cameraPosition)
{
   if (!m_selectionHighlighting || objects.isEmpty()) return;

   // create a stencil of the highlighted region
   glClearStencil(0x4);
   glDisable(GL_LIGHTING);
   glClear(GL_STENCIL_BUFFER_BIT);
   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS, 0x0, 0x4);
   glStencilMask(0x4);
   glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

   GLObjectList::const_iterator object;
   for (object = objects.begin(); object != objects.end(); ++object) {
       (*object)->draw(cameraPosition);
   }

   glStencilFunc(GL_EQUAL, 0x4, 0x4);
   glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

   for (object = objects.begin(); object != objects.end(); ++object) {
       glEnable(GL_BLEND);
       glBlendFunc(GL_ONE, GL_ONE);
       (*object)->drawSelected(cameraPosition);
   }

   glDisable(GL_STENCIL_TEST);
   glDisable(GL_BLEND);
   glEnable(GL_LIGHTING);
}


void Viewer::drawLabels(GLObjectList const& objects)
{
   AtomList atomList;
   Layer::Atom* atom;
   Layer::Charge* charge;
   bool selectedOnly = (m_selectedObjects.count() > 0);

   GLObjectList::const_iterator object;
   for (object = objects.begin(); object!= objects.end(); ++object) {
       if ( (atom = qobject_cast<Layer::Atom*>(*object)) ) {
          atom->drawLabel(*this, m_labelType, s_labelFontMetrics);
          if ( !selectedOnly || atom->isSelected() ) atomList.append(atom);
       }else if ( (m_labelType == Layer::Atom::Charge) && 
                  (charge = qobject_cast<Layer::Charge*>(*object)) ) {
          charge->drawLabel(*this, s_labelFontMetrics);
       }
   }

   glDisable(GL_LIGHTING);
   qglColor(foregroundColor());

   QString mesg = selectedOnly ? "Selection " : "Total ";
   AtomList::const_iterator iter;
   double value(0.0);

   if (m_labelType == Layer::Atom::Mass) {
      for (iter= atomList.begin(); iter != atomList.end(); ++iter) {
          value += (*iter)->getMass();
      }
      mesg += "mass: " + QString::number(value,'f', 3);
      drawText(width()-s_labelFontMetrics.width(mesg), height()-10, mesg);
   }else if (m_labelType == Layer::Atom::Charge) {
      for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
          value += (*iter)->getCharge();
      }
      mesg += "charge: " + QString::number(value,'f', 2);
      drawText(width()-s_labelFontMetrics.width(mesg), height()-10, mesg);
   }else if (m_labelType == Layer::Atom::Spin) {
      for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
          value += (*iter)->getSpin();
      }
      mesg += "spin: " + QString::number(value,'f', 2);
      drawText(width()-s_labelFontMetrics.width(mesg), height()-10, mesg);
   }

   glEnable(GL_LIGHTING);
}


void Viewer::displayGeometricParameter(GLObjectList const& selection)
{
   QString msg;
   Layer::Atom *a, *b, *c, *d;
   Layer::Bond *bond;

   switch (selection.size()) {
      case 0:
         return;
         break;
      case 1: 
         if ( (bond = qobject_cast<Layer::Bond*>(selection[0])) ) {
             QChar ch(0x00c5);
             msg = "Bond length: " + QString::number(bond->length(), 'f', 5) + " " + ch;
         }else if ( (a = qobject_cast<Layer::Atom*>(selection[0])) ) {
             msg = a->getAtomicSymbol() + " atom";
         }
         break;

      case 2:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) ) {
            QChar ch(0x00c5);
            msg = "Distance: " + QString::number(Layer::Atom::distance(a,b), 'f', 5) + " " + ch;
         }
         break;

      case 3:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) &&
              (c = qobject_cast<Layer::Atom*>(selection[2])) ) {
            QChar ch(0x00b0);
            msg = "Angle: " + QString::number(Layer::Atom::angle(a,b,c), 'f', 5) + " " + ch;
         }
         break;

      case 4:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) &&
              (c = qobject_cast<Layer::Atom*>(selection[2])) &&
              (d = qobject_cast<Layer::Atom*>(selection[3])) ) {
            QChar ch(0x00b0);
            msg = "Torsion: " + QString::number(Layer::Atom::torsion(a,b,c,d), 'f', 3) + " " + ch;
         }
         break;

      default:
         break;
   }

   displayMessage(msg); 
}



// ---------------- Animation functions ---------------
void Viewer::pushAnimators(AnimatorList const& animators)
{
   //QLOG_TRACE() << "Pushing" << animators.size() << "animators";
   AnimatorList::const_iterator iter;
   for (iter = animators.begin(); iter != animators.end(); ++iter) {
       if (!m_animatorList.contains(*iter)) {
      //QLOG_TRACE() << "Pushing animator:" << (*iter);
          m_animatorList.append(*iter);
          connect(*iter, SIGNAL(finished(Animator::Base*)),
             this, SLOT(popAnimator(Animator::Base*)));
       }
   }

   if (!animationIsStarted()) {
      QLOG_DEBUG() << "Starting animation";
      startAnimation();
   }
}


void Viewer::popAnimators(AnimatorList const& animators)
{
   //QLOG_TRACE() << "Popping " << animators.size() << "animators";
   AnimatorList::const_iterator iter;
   for (iter = animators.begin(); iter != animators.end(); ++iter) {
      //QLOG_TRACE() << "Popping animator:" << (*iter);
       popAnimator(*iter);
   }
}


void Viewer::popAnimator(Animator::Base* base)
{
   disconnect(base, SIGNAL(finished(Animator::Base*)),
      this, SLOT(popAnimator(Animator::Base*)));
   int n(m_animatorList.size());
   m_animatorList.removeAll(base);
   if (m_animatorList.isEmpty() && n > 0) {
      // turn the lights out if we are last to leave
      QLOG_DEBUG() << "Stoping animation";
      stopAnimation();
      m_viewerModel.determineSymmetry();
   }
}

void Viewer::animate()
{
   AnimatorList animators(m_animatorList);
   AnimatorList::iterator iter;
   for (iter = animators.begin(); iter != animators.end(); ++iter) {
       (*iter)->step();
   }

   if (m_snapper) m_snapper->capture();
}


void Viewer::clearAnimators()
{
   stopAnimation();
   m_animatorList.clear();
}


void Viewer::setRecordingActive(bool activate)
{
   if (activate) {
      if (m_snapper) {
         QLOG_WARN() << "Animation recording started with existing Snapshot object";
      }else {
         m_snapper = new Snapshot(this, Snapshot::Movie);
         if (!m_snapper->requestFileName()) {
            delete m_snapper;
            m_snapper = 0;
            recordingCancelled();
         }
      } 
   }else {
      if (m_snapper) {
         m_snapper->makeMovie();
         delete m_snapper;
      } 
      m_snapper = 0;
   }
}



// ---------------- Selection functions ---------------

void Viewer::setSelectionMode(SelectionMode const mode)
{
   m_selectionMode = mode;
}

void Viewer::drawWithNames() 
{
   Vec cameraPosition(camera()->position());

   int i;
   for (i = 0; i < int(m_objects.size()); ++i) {
       glPushName(i);
       m_objects.at(i)->draw(cameraPosition);
       glPopName();
   }

   for (int j = 0; j < int(m_buildObjects.size()); ++j) {
       glPushName(i);
       m_buildObjects.at(j)->draw(cameraPosition);
       glPopName();
       ++i;
   }
}


void Viewer::endSelection(const QPoint&) 
{
   glFlush();

   // Get the number of objects that were seen through the pick matrix frustum.
   m_selectionHits = glRenderMode(GL_RENDER);
   //qDebug() << "Viewer::endSelection with nhits:" << m_selectionHits << m_objects.size() ;
   //for (int i = 0; i < m_objects.size(); ++i) qDebug() << m_objects[i];
   setSelectRegionWidth(3);
   setSelectRegionHeight(3);

   if (m_selectionHits == 0) return;

   // If the user clicks, then we only select the front object
   if ( (m_selectionMode == AddClick) || (m_selectionMode == RemoveClick) ||
        (m_selectionMode == ToggleClick) ) {
      GLuint zMin = (selectBuffer())[1];
      int iMin = 0;
      for (int i = 1; i < m_selectionHits; ++i) {
          if ((selectBuffer())[4*i+1] < zMin) {
             iMin = i;
             zMin = (selectBuffer())[4*i+1];
          }
      }

      if (m_selectionMode == AddClick) {
         addToSelection((selectBuffer())[4*iMin+3]);
      }else if (m_selectionMode == RemoveClick) {
         removeFromSelection((selectBuffer())[4*iMin+3]);
      }else {
         toggleSelection((selectBuffer())[4*iMin+3]);
      }

   }else {
      // The selection rectangle is non-zero so we select all the objects
      // behind it.
   
      // Interpret results : each object created 4 values in the selectBuffer().
      // (selectBuffer())[4*i+3] is the id pushed on the stack.

	  // Temporarily switch off GL updating so the selection routines don't
	  // trigger an update which makes the slected item appear incrementally.
      enableUpdate(false);
      for (int i = 0; i < m_selectionHits; ++i) {
          switch (m_selectionMode) {
             case Add: 
                addToSelection((selectBuffer())[4*i+3]); 
                break;
             case Remove: 
                removeFromSelection((selectBuffer())[4*i+3]);  
                break;
             case Toggle: 
                toggleSelection((selectBuffer())[4*i+3]);  
                break;
             default: 
                addToSelection((selectBuffer())[4*i+3]); 
                break;
          }
      }
      enableUpdate(true);
      updateGL();
   }
}


void Viewer::drawSelectionRectangle(QRect const& rect) const {
   startScreenCoordinatesSystem();
   glBlendFunc(GL_ONE, GL_ONE);
   glDisable(GL_LIGHTING);
   glEnable(GL_BLEND);

   glColor4f(0.0, 0.0, 0.3f, 0.3f);
   glBegin(GL_QUADS);
      glVertex2i(rect.left(),  rect.top());
      glVertex2i(rect.right(), rect.top());
      glVertex2i(rect.right(), rect.bottom());
      glVertex2i(rect.left(),  rect.bottom());
   glEnd();

   glLineWidth(2.0);
   glColor4f(0.4f, 0.4f, 0.5f, 0.5f);

   glBegin(GL_LINE_LOOP);
      glVertex2i(rect.left(),  rect.top());
      glVertex2i(rect.right(), rect.top());
      glVertex2i(rect.right(), rect.bottom());
      glVertex2i(rect.left(),  rect.bottom());
   glEnd();

   glDisable(GL_BLEND);
   glEnable(GL_LIGHTING);
   stopScreenCoordinatesSystem();
}


void Viewer::postSelection(QPoint const&) {
   //displayGeometricParameter();
}


void Viewer::addToSelection(int id) 
{
   if (id < m_objects.size()) {
      select(m_objects[id]->index(), QItemSelectionModel::Select);
   }
}


void Viewer::removeFromSelection(int id) 
{
   if (id < m_objects.size()) {
      select(m_objects[id]->index(), QItemSelectionModel::Deselect);
   }
}


void Viewer::toggleSelection(int id) 
{
   if (id < m_objects.size()) {
      if (m_objects[id]->isSelected()) {
         select(m_objects[id]->index(), QItemSelectionModel::Deselect);
      }else {
         select(m_objects[id]->index(), QItemSelectionModel::Select);
      }
   }
}



// ---------------- Events ---------------
void Viewer::mousePressEvent(QMouseEvent* e)
{
   m_currentHandler->mousePressEvent(e);
   if (!e->isAccepted()) QGLViewer::mousePressEvent(e);
}


void Viewer::mouseMoveEvent(QMouseEvent* e)
{
   m_currentHandler->mouseMoveEvent(e);
   if (!e->isAccepted()) QGLViewer::mouseMoveEvent(e);
}     
      

void Viewer::mouseReleaseEvent(QMouseEvent* e)
{     
   m_currentHandler->mouseReleaseEvent(e);
   if (!e->isAccepted()) QGLViewer::mouseReleaseEvent(e);
}     

   
void Viewer::mouseDoubleClickEvent(QMouseEvent* e)
{     
   m_currentHandler->mouseDoubleClickEvent(e);
   if (!e->isAccepted()) QGLViewer::mouseDoubleClickEvent(e);
}     

      
void Viewer::wheelEvent(QWheelEvent* e) 
{     
   m_currentHandler->wheelEvent(e);
   if (!e->isAccepted()) QGLViewer::wheelEvent(e);
}


void Viewer::keyPressEvent(QKeyEvent *e)
{     
   e->ignore();

   // Check if we need to swtich handlers or 
   // if we are using a global QKeyEvent
   switch (e->key()) {
      case s_selectKey:
         setTemporaryViewerMode(Select);
         e->accept();
         break;
      case s_manipulateSelectionKey:
         setTemporaryViewerMode(ManipulateSelection);
         e->accept();
         break;
      case s_buildKey:
         setTemporaryViewerMode(Build);
         e->accept();
         break;
   }

   // Offload to the current handler
   if (!e->isAccepted()) m_currentHandler->keyPressEvent(e);
   if (!e->isAccepted()) QGLViewer::keyPressEvent(e);
   return;
}


void Viewer::keyReleaseEvent(QKeyEvent *e)
{
   if ( (e->key() == s_selectKey) ||
        (e->key() == s_buildKey) ||
        (e->key() == s_manipulateSelectionKey) ) {
      e->accept();
      setTemporaryViewerMode(m_activeMode);
   } else if (e->key() == Qt::Key_Escape && isFullScreen()) {
      escapeFullScreen();
   }else {
      m_currentHandler->keyReleaseEvent(e);
   }

   if (!e->isAccepted()) QGLViewer::keyReleaseEvent(e);
}


void Viewer::enterEvent(QEvent*)
{
   setActiveViewerMode(m_activeMode);
}
      
   
void Viewer::leaveEvent(QEvent*)
{
   QWidget::setCursor(QCursor(Qt::ArrowCursor));
}

void Viewer::dragEnterEvent(QDragEnterEvent* event)
{  
   if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}
   
   
void Viewer::dropEvent(QDropEvent* event)
{  
   QLOG_INFO() << "Drop Event in Viewer successful";
 
   QList<QUrl> urls(event->mimeData()->urls());
   QList<QUrl>::iterator iter;
   
   for (iter = urls.begin(); iter != urls.end(); ++iter) {
      openFileFromDrop((*iter).path());
      QLOG_DEBUG() << *iter;
   }
   event->acceptProposedAction();
} 


// --------------- Handler Modes ---------------
void Viewer::setActiveViewerMode(ViewerMode const mode)
{
   m_previousMode = m_activeMode;
   m_activeMode = mode;
   m_selectionHighlighting = true;
   m_selectionMode = None;
   setTemporaryViewerMode(m_activeMode);
   activeViewerModeChanged(m_activeMode);
}


void Viewer::setTemporaryViewerMode(ViewerMode const mode)
{  
   switch (mode) {
      case Manipulate:
         m_currentHandler = &m_manipulateHandler; 
         break;
      case Select:
         m_currentHandler = &m_selectHandler; 
         break;
      case Build:
         m_currentHandler = &m_buildHandler; 
         break;
      case ManipulateSelection:
         m_currentHandler = &m_manipulateSelectionHandler; 
         break;
      case ReindexAtoms:
         m_currentHandler = &m_reindexAtomsHandler; 
         break;
   }

   setCursor(m_currentHandler->cursorType());
}


// This is the only entry point for the ReindexAtom mode
void Viewer::reindexAtoms(Layer::Molecule* molecule)
{
   QLOG_INFO() << "Reindexing Atoms 0";
   setActiveViewerMode(ReindexAtoms);
   m_reindexAtomsHandler.reset(molecule);
   setLabelType(Layer::Atom::Reindex);
   displayMessage("Hit Return to commit changes, Esc to cancel");
}



GLObjectList Viewer::startManipulation(QMouseEvent* event) 
{
   ManipulatedFrameSetConstraint* mfsc =
      (ManipulatedFrameSetConstraint*)(manipulatedFrame()->constraint());
   mfsc->clearSet();

   GLObjectList selection;
   GLObjectList::iterator iter;

   for (iter = m_objects.begin(); iter != m_objects.end(); ++iter) {
/// !!! if cast to fragment, 
       if ( (*iter)->isSelected()) selection.append(*iter); 
   }
   
   Layer::Bond* bond;

   if ((selection.size() == 1) &&
      (bond = qobject_cast<Layer::Bond*>(selection.first())) ) {

      // Determine the parent molecule
      MoleculeList parents(bond->findLayers<Layer::Molecule>(Layer::Parents));
      Layer::Molecule* molecule;

      if (parents.isEmpty()) {
         return GLObjectList();
      }else {
         molecule = parents.first();
      }

      Layer::Atom* begin(bond->beginAtom());
      Layer::Atom* end(bond->endAtom());

      Vec a(camera()->projectedCoordinatesOf(begin->getPosition()));
      Vec b(camera()->projectedCoordinatesOf(end->getPosition()));
      Vec e(event->x(), event->y(), 0.0);

      if ((a-e).norm() < (b-e).norm()) {
         Layer::Atom* tmp(begin);
         begin = end;
         end   = tmp;
      }

      AtomList fragment(molecule->getContiguousFragment(begin, end));

      selection.clear();
      AtomList::iterator iter;
      for (iter = fragment.begin(); iter != fragment.end(); ++iter) {
          mfsc->addObjectToSet(*iter);
          selection.append(*iter);
      }
      mfsc->addObjectToSet(end);

      Vec axis(end->getPosition() - begin->getPosition());
      mfsc->setTranslationConstraint(AxisPlaneConstraint::AXIS, axis);
      mfsc->setRotationConstraint(AxisPlaneConstraint::AXIS, axis);
      manipulatedFrame()->setPosition(end->getPosition());

   }else if (selection.size() > 0) {

      mfsc->setTranslationConstraintType(AxisPlaneConstraint::FREE);
      mfsc->setRotationConstraintType(AxisPlaneConstraint::FREE);
      Vec averagePosition;

      GLObjectList::const_iterator iter;
      for (iter = selection.begin(); iter != selection.end(); ++iter) {
          mfsc->addObjectToSet(*iter);
          averagePosition += (*iter)->getPosition();
      }
      manipulatedFrame()->setPosition(averagePosition / selection.size());
   }

   return selection;
}



// --------------- Save Snapshot ---------------
void Viewer::saveSnapshot()
{ 
   QGLViewer::saveSnapshot(false); 
   //Snapshot snap(this);
   //snap.capture();
}


} // end namespace IQmol
