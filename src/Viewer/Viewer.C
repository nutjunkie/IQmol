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

#include "ShaderLibrary.h"
#include "ShaderDialog.h"
#include "CameraDialog.h"
#include "Viewer.h"
#include "ViewerModel.h"
#include "QsLog.h"
#include "MoleculeLayer.h"
#include "EfpFragmentLayer.h"
#include "Preferences.h"
#include "PovRayGen.h"
#include "ManipulatedFrameSetConstraint.h"
#include "QGLViewer/manipulatedFrame.h"
#include <QStandardItem>
#include <QDropEvent>
#include <QtDebug>
#include <QUrl>
#include <QGLFramebufferObject>
#include <QFileDialog>
#include <QMimeData>
#include <cmath>


using namespace qglviewer;

namespace IQmol {

Vec Layer::GLObject::s_cameraPosition  = Vec(0.0, 0.0, 0.0);
Vec Layer::GLObject::s_cameraDirection = Vec(0.0, 0.0, 1.0);
Vec Layer::GLObject::s_cameraPivot     = Vec(0.0, 0.0, 0.0);

const Qt::Key Viewer::s_buildKey(Qt::Key_Alt);
const Qt::Key Viewer::s_selectKey(Qt::Key_Shift);
const Qt::Key Viewer::s_manipulateSelectionKey(Qt::Key_Control);
const Qt::KeyboardModifier Viewer::s_buildModifier(Qt::AltModifier);
const Qt::KeyboardModifier Viewer::s_selectModifier(Qt::ShiftModifier);
const Qt::KeyboardModifier Viewer::s_manipulateSelectionModifier(Qt::ControlModifier);

//QFont  Viewer::s_labelFont("Helvetica", 14, QFont::Black);
QFont  Viewer::s_labelFont("Arial", 14, QFont::Black);
QFontMetrics Viewer::s_labelFontMetrics(Viewer::s_labelFont);


//! Window set up is done here
Viewer::Viewer(QGLContext* context, ViewerModel& model, QWidget* parent) : 
   QGLViewer(context, parent), 
   m_viewerModel(model), 
   m_selectionHighlighting(true),
   m_labelType(Layer::Atom::None),
   m_selectHandler(this),
   m_manipulateHandler(this),
   m_buildEfpFragmentHandler(this),
   m_buildAtomHandler(this),
   m_buildFunctionalGroupHandler(this),
   m_buildMoleculeHandler(this),
   m_reindexAtomsHandler(this),
   m_manipulateSelectionHandler(this),
   m_snapper(0),
   m_blockUpdate(false),
   m_glContext(context),
   m_shaderLibrary(0),
   m_shaderDialog(0),
   m_cameraDialog(0)
{ 
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

   setAcceptDrops(true);
   // The following two lines must be in this order
   setDefaultBuildElement(6);
   setActiveViewerMode(BuildAtom);  // this should get overwritten by the MainWindow class

   m_recordTimer.setInterval(30);  // ~15 frames per second

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   format.setAlphaBufferSize(8);
   format.setStencilBufferSize(8);
   format.setProfile(QSurfaceFormat::CoreProfile);
   QSurfaceFormat::setDefaultFormat(format);
}


Viewer::~Viewer()
{
   if (m_shaderDialog) delete m_shaderDialog;
   if (m_shaderLibrary) delete m_shaderLibrary;
   if (m_cameraDialog) delete m_cameraDialog;
}


//! The OpenGL context is not available in the constructor, so all GL setup
//! must be done here.
void Viewer::init()
{
   makeCurrent();

   setManipulatedFrame(new ManipulatedFrame());
   manipulatedFrame()->setConstraint(new ManipulatedFrameSetConstraint());

   s_labelFont.setStyleStrategy(QFont::OpenGLCompatible);
   s_labelFont.setPointSize(Preferences::LabelFontSize());

   setForegroundColor(Preferences::ForegroundColor());
   setBackgroundColor(Preferences::BackgroundColor());

   glEnable(GL_LIGHT0);
   glDisable(GL_LIGHT1);

   //setFPSIsDisplayed(true);

   const GLfloat light_ambient[4]  = {0.3, 0.3, 0.3, 1.0};
   const GLfloat light_diffuse[4]  = {0.75, 0.75, 0.75, 0.5};
   const GLfloat light_specular[4] = {1.0, 1.0, 1.0, 1.0};

   glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
   glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
   glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);

   //glLightf( GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
   //glLightf( GL_LIGHT1, GL_SPOT_CUTOFF,   95.0);
   //glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.1f);
   //glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.3f);
   //glLightf( GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);
}


void Viewer::initShaders()
{
   makeCurrent();
   m_shaderLibrary = new ShaderLibrary(m_glContext);

   if (QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
      QLOG_INFO() << "OpenGL framebuffers are active";
      m_shaderLibrary->setFiltersAvailable(true);
   }else {
      QLOG_INFO() << "OpenGL framebuffers are unavailable";
      m_shaderLibrary->setFiltersAvailable(false);
   }
}


void Viewer::editShaders()
{
   if (!m_shaderLibrary) return;

   if (!m_shaderDialog) {
      m_shaderDialog = new ShaderDialog(*m_shaderLibrary, this);
      connect(m_shaderDialog, SIGNAL(updated()), this, SLOT(updateGL()));
      connect(m_shaderDialog, SIGNAL(generatePovRay()), this, SLOT(generatePovRay()));
   }
   m_shaderDialog->show();
   m_shaderDialog->raise();
}


void Viewer::editCamera()
{
   if (!m_cameraDialog) {
      m_cameraDialog = new CameraDialog(*camera(), this);
      connect(m_cameraDialog, SIGNAL(updated()), this, SLOT(updateGL()));
      connect(m_cameraDialog, SIGNAL(resetView()), this, SLOT(resetView()));

      //connect(m_cameraDialog, SIGNAL(interpolated()), this, SLOT(updateGL()));
      connect(m_cameraDialog, SIGNAL(interpolated()), this, SLOT(animate()));
   }
   m_cameraDialog->sync();
   m_cameraDialog->show();
   m_cameraDialog->raise();
}


void Viewer::setBackgroundColor(QColor const& color)
{
   if (m_shaderLibrary) m_shaderLibrary->broadcast("backgroundColor", color);
   QGLViewer::setBackgroundColor(color);
}



void Viewer::resizeGL(int width, int height)
{
   QGLViewer::resizeGL(width, height);
   if (!m_shaderLibrary) return;

   GLdouble m[16]; 
   camera()->getProjectionMatrix(m);
   m_shaderLibrary->resizeScreenBuffers(QSize(width, height), m);
}

void Viewer::generatePovRay()
{
   QFileInfo info(Preferences::LastFileAccessed());
   info.setFile(info.dir(), info.completeBaseName() + ".pov");

   while (1) {
      QString filter(tr("POV") + " (*.pov)");
      QStringList extensions;
      extensions << filter;

      QString fileName(QFileDialog::getSaveFileName(this, tr("Save File"), 
         info.filePath(), extensions.join(";;"), &filter));

      if (fileName.isEmpty()) {
         // This will occur if the user cancels the action.
         return;
      }else {
         QRegExp rx("\\*(\\..+)\\)");
         if (rx.indexIn(filter) > 0) { 
            filter = rx.cap(1);
            if (!fileName.endsWith(filter, Qt::CaseInsensitive)) {
               fileName += filter;
            }    
         }    
         Preferences::LastFileAccessed(fileName);
         generatePovRay(fileName);
         break;
      }    
   } 
}


void Viewer::generatePovRay(QString const& filename)
{
   // The ordering of these calls is important
   PovRayGen povRayGen(filename, m_shaderLibrary->povrayVariables());
   povRayGen.setCamera(camera());
   povRayGen.setBackground(m_viewerModel.backgroundColor());
   povRayGen.setClippingPlane(m_viewerModel.clippingPlane());

   m_objects = m_viewerModel.getVisibleObjects();
   for (int i = 0; i < int(m_objects.size()); ++i) {
       m_objects.at(i)->povray(povRayGen);
   }
}


void Viewer::draw()
{
   if (m_blockUpdate || !m_shaderLibrary) return;

   if (m_cameraDialog) m_cameraDialog->sync();

   m_objects = m_viewerModel.getVisibleObjects();
   m_selectedObjects = m_viewerModel.getSelectedObjects();

   if (!m_shaderLibrary->filtersActive() || animationIsStarted()) return fastDraw();
   qDebug() << "Filters are on in drawNew";

   makeCurrent();
   Layer::GLObject::SetCameraPosition(camera()->position());
   Layer::GLObject::SetCameraDirection(camera()->viewDirection());
//   Layer::GLObject::SetCameraPivot(camera()->pivotPoint());

   QString shader(m_shaderLibrary->currentShader());

   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  

   // Generate normal and filter maps
   m_shaderLibrary->bindNormalMap(camera()->zNear(), camera()->zFar());
   drawObjects(m_objects);
   m_shaderLibrary->releaseNormalMap();
   m_shaderLibrary->generateFilters();

   // Redraw everything to get the transparency right
   // library.clearBuffers();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   m_shaderLibrary->bindShader(shader);
   m_shaderLibrary->bindTextures(shader);

   drawGlobals();

   drawObjects(m_objects);
   drawSelected(m_selectedObjects);
   drawObjects(m_currentBuildHandler->buildObjects());
   

   // Suspend the shader for text rendering
   m_shaderLibrary->suspend();
   m_shaderLibrary->releaseTextures();
   m_shaderLibrary->clearFrameBuffers();

   if (m_labelType != Layer::Atom::None) drawLabels(m_objects);
   if (m_currentHandler->selectionMode() != Handler::None) {
      drawSelectionRectangle(m_selectHandler.region());
   }

   // Post draw stuff really
   float color[4];
   color[0] = foregroundColor().red()   / 255.0;
   color[1] = foregroundColor().green() / 255.0;
   color[2] = foregroundColor().blue()  / 255.0;
   color[3] = 1.0;
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);

   displayGeometricParameter(m_selectedObjects);
   displayMullikenDecomposition(m_selectedObjects);
}


void Viewer::fastDraw()
{
   if (m_blockUpdate) return;

   makeCurrent();
   Layer::GLObject::SetCameraPosition(camera()->position());

   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
   glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glDepthMask (GL_TRUE);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   m_shaderLibrary->resume();
   drawGlobals();

   m_viewerModel.clippingPlane().setEquation();
   drawObjects(m_objects);
   drawObjects(m_currentBuildHandler->buildObjects());
   m_viewerModel.clippingPlane().draw();

   // suspend the shader for writing text and highlighting
   m_shaderLibrary->suspend();
   drawSelected(m_selectedObjects);

   if (m_labelType != Layer::Atom::None) drawLabels(m_objects);
   if (m_currentHandler->selectionMode() != Handler::None) {
      drawSelectionRectangle(m_selectHandler.region());
   }

   // Post draw stuff really
   qglColor(foregroundColor());
   float color[4];
   color[0] = foregroundColor().red()   / 255.0;
   color[1] = foregroundColor().green() / 255.0;
   color[2] = foregroundColor().blue()  / 255.0;
   color[3] = 1.0;
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);


   glDisable(GL_LIGHTING);
   //glDisable(GL_DEPTH_TEST);
   displayGeometricParameter(m_selectedObjects);
   displayMullikenDecomposition(m_selectedObjects);
}


void Viewer::setSceneRadius(double const radius)
{
   float r = std::max(radius, Preferences::DefaultSceneRadius());
   QGLViewer::setSceneRadius(r);
   Vec position(camera()->position());
   position = 3.0*r*position.unit();
   camera()->setPosition(position);
   updateGL();
}


void Viewer::resetView()
{
   setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
   double radius = std::max(Preferences::DefaultSceneRadius(), m_viewerModel.sceneRadius());

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
   }else {
      camera()->interpolateTo(Frame(Vec(0.0, 0.0, 3.0*radius), Quaternion()), 1.0);
   }

   m_viewerModel.sceneRadiusChanged(radius);
   updateGL();

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
      case Layer::Atom::None:      m_labelType = Layer::Atom::None;      break;
      case Layer::Atom::Index:     m_labelType = Layer::Atom::Index;     break;
      case Layer::Atom::Element:   m_labelType = Layer::Atom::Element;   break;
      case Layer::Atom::Charge:    m_labelType = Layer::Atom::Charge;    break;
      case Layer::Atom::Mass:      m_labelType = Layer::Atom::Mass;      break;
      case Layer::Atom::Spin:      m_labelType = Layer::Atom::Spin;      break;
      case Layer::Atom::Reindex:   m_labelType = Layer::Atom::Reindex;   break;
      case Layer::Atom::NmrShift:  m_labelType = Layer::Atom::NmrShift;  break;
      default:
         QLOG_DEBUG() << "Unimplemented atom label type:" << type;
         m_labelType = Layer::Atom::None;    
         break;
   }
   updateGL();
}


void Viewer::setDefaultBuildElement(unsigned int element)
{
   m_buildAtomHandler.setBuildElement(element);
   m_currentBuildHandler = &m_buildAtomHandler;
}


void Viewer::setDefaultBuildFragment(QString const& filePath, Viewer::Mode const mode)
{
   switch (mode) {
      case BuildFunctionalGroup:
         m_currentBuildHandler = &m_buildFunctionalGroupHandler;
         break;
      case BuildEfp:
         m_currentBuildHandler = &m_buildEfpFragmentHandler;
         break;
      case BuildMolecule:
         m_currentBuildHandler = &m_buildMoleculeHandler;
         break;
      default:
         QLOG_WARN() << "Invalid Viewer::Mode in setDefaultBuildFragment";
         return;
   }

   m_currentBuildHandler->setBuildFile(filePath);
}


void Viewer::drawGlobals() 
{ 
   m_viewerModel.displayGlobals(); 
}


void Viewer::drawObjects(GLObjectList const& objects)
{
   GLObjectList::const_iterator object;
   for (object = objects.begin(); object != objects.end(); ++object) {
       (*object)->draw();
   }
}


void Viewer::drawSelected(GLObjectList const& objects)
{
   //qDebug() << "drawSelected called with" << objects.size() << "objects";
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
       (*object)->draw();
   }

   glStencilFunc(GL_EQUAL, 0x4, 0x4);
   glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

   for (object = objects.begin(); object != objects.end(); ++object) {
       glEnable(GL_BLEND);
       glBlendFunc(GL_ONE, GL_ONE);
       (*object)->drawSelected();
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

   glDisable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);

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

   
   qglColor(foregroundColor());

   QString msg = selectedOnly ? "Selection " : "Total ";
   AtomList::const_iterator iter;
   double value(0.0);

   if (m_labelType == Layer::Atom::Mass) {
      for (iter= atomList.begin(); iter != atomList.end(); ++iter) {
          value += (*iter)->getMass();
      }
      msg += "mass: " + QString::number(value,'f', 3);

   }else if (m_labelType == Layer::Atom::Charge) {
      for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
          value += (*iter)->getCharge();
      }
      msg += "charge: " + QString::number(value,'f', 2);

   }else if (m_labelType == Layer::Atom::Spin) {
      for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
          value += (*iter)->getSpin();
      }
      msg += "spin: " + QString::number(value,'f', 2);

   }else if (m_labelType == Layer::Atom::NmrShift) {
      iter = atomList.begin();
      if (iter == atomList.end()) {
         msg.clear();
      }else if ((*iter)->haveNmrShift()) {
         msg = "Chemical shifts";
      }else {
         msg = "Nuclear shieldings";
      }

   }else {
      msg.clear();
   }

   if (!msg.isEmpty()) {
      drawText(width()-s_labelFontMetrics.width(msg), height()-10, msg);
   }

   glDisable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}


void Viewer::displayGeometricParameter(GLObjectList const& selection)
{
   QString msg;
   Layer::Atom *a, *b, *c, *d;
   Layer::Primitive *A, *B;
   Layer::Bond *bond;
   Layer::EfpFragment *efp;

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
         }else if ( (efp = qobject_cast<Layer::EfpFragment*>(selection[0])) ) {
             msg = efp->text() + " EFP";
         }
         break;

      case 2:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) ) {
            QChar ch(0x00c5);
            msg = "Distance: " + 
                  QString::number(Layer::Primitive::distance(a,b), 'f', 5) + " " + ch;
         }else if ( (A = qobject_cast<Layer::Primitive*>(selection[0])) &&
                    (B = qobject_cast<Layer::Primitive*>(selection[1])) ) {
            QChar ch(0x00c5);
            msg = "C.O.M. Distance: " + 
                  QString::number(Layer::Primitive::distance(A,B),'f',5) + " " + ch;
         }
         break;

      case 3:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) &&
              (c = qobject_cast<Layer::Atom*>(selection[2])) ) {
            QChar ch(0x00b0);
            msg = "Angle: " + 
                  QString::number(Layer::Primitive::angle(a,b,c), 'f', 5) + " " + ch;
         }
         break;

      case 4:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) &&
              (c = qobject_cast<Layer::Atom*>(selection[2])) &&
              (d = qobject_cast<Layer::Atom*>(selection[3])) ) {
            QChar ch(0x00b0);
            msg = "Torsion: " + 
                  QString::number(Layer::Primitive::torsion(a,b,c,d), 'f', 3) + " " + ch;
         }
         break;

      default:
         return;
         break;
   }

   // We cannot use displayMessage here as it triggers an update
   //displayMessage(""); 

   QString space(" ");
   drawText(10, height()-10, space.repeated(message_.length()));
   drawText(10, height()-10, msg);
   //if (message_.isEmpty()) drawText(10, height()-10, msg);
}



void Viewer::displayMullikenDecomposition(GLObjectList const& selection)
{
   if (selection.isEmpty()) {
      //qDebug() << "Selection is empty";
      return;
   }
   MoleculeList parents(selection.first()->findLayers<Layer::Molecule>(Layer::Parents));
   if (parents.isEmpty()) {
      //qDebug() << "No parent molecule found";
      return;
   }

   Layer::Molecule* molecule(parents.first());
   if (!molecule->hasMullikenDecompositions()) {
      //qDebug() << "No Mulliken Decompositon available";
      return;
   }

   Layer::Atom *a(0), *b(0);
   Layer::Bond *bond;
   int ia(0), ib(0);

   switch (selection.size()) {
      case 1: 
         if ( (bond = qobject_cast<Layer::Bond*>(selection[0])) ) {
            ia = bond->beginAtom()->getIndex();
            ib = bond->beginAtom()->getIndex();
         }else if ( (a = qobject_cast<Layer::Atom*>(selection[0])) ) {
            ia = ib = a->getIndex();
         }
         break;

      case 2:
         if ( (a = qobject_cast<Layer::Atom*>(selection[0])) &&
              (b = qobject_cast<Layer::Atom*>(selection[1])) ) {
            ia = a->getIndex();
            ib = b->getIndex();
         }
         break;

      default:
         return;
         break;
   }
   
   QString msg("Mullliken Decomposition ");
   msg += QString::number(ia) + "-" + QString::number(ib) + ": ";
   msg += QString::number(molecule->mullikenDecomposition(ia,ib), 'f', 4);

   // We cannot use displayMessage here as it triggers an update
   drawText(width()-s_labelFontMetrics.width(msg), height()-10, msg);
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

   updateGL();
   //draw();
   animationStep();
}


void Viewer::clearAnimators()
{
   stopAnimation();
   m_animatorList.clear();
}


void Viewer::setRecord(bool activate)
{
   if (activate) {
      if (m_snapper) {
         QLOG_WARN() << "Animation recording started with existing Snapshot object";
      }else {
         m_snapper = new Snapshot(this, Snapshot::Movie);
         // For continuous snapping:
         //connect(&m_recordTimer, SIGNAL(timeout()), m_snapper, SLOT(capture()));
         connect(this, SIGNAL(animationStep()), m_snapper, SLOT(capture()));
         connect(m_snapper, SIGNAL(movieFinished()), this, SLOT(movieMakingFinished()));
         if (!m_snapper->requestFileName()) {
            delete m_snapper;
            m_snapper = 0;
            recordingCanceled();
         }else {
            m_recordTimer.start();
         }
      } 
   }else {
       m_recordTimer.stop();
       disconnect(&m_recordTimer, SIGNAL(timeout()), m_snapper, SLOT(capture()));
      //if (m_snapper) m_snapper->makeFfmpegMovie();
      if (m_snapper) m_snapper->makeMovie();
   }
}


void Viewer::movieMakingFinished()
{
   if (m_snapper) {
      delete m_snapper;
      m_snapper = 0;
   }else {
      QLOG_WARN() << "movieMakingFinished called with null snapshot taker";
   }
}


void Viewer::blockUpdate(bool tf)
{ 
   if (tf) {
      qDebug() << "Viewer update blocked";
   }else {
      qDebug() << "Viewer update enabled";
      
   }
   m_blockUpdate = tf; 
}


// ---------------- Selection functions ---------------
void Viewer::drawWithNames() 
{
   glInitNames(); 

   int i;
   for (i = 0; i < int(m_objects.size()); ++i) {
       glPushName(i);
       m_objects.at(i)->draw();
       glPopName();
   }

   GLObjectList buildObjects(m_currentBuildHandler->buildObjects());
   for (int j = 0; j < int(buildObjects.size()); ++j) {
       glPushName(i);
       buildObjects.at(j)->draw();
       glPopName();
       ++i;
   }
}


void Viewer::endSelection(const QPoint&) 
{
   glFlush();

   // Get the number of objects that were seen through the pick matrix frustum.
   m_selectionHits = glRenderMode(GL_RENDER);
   setSelectRegionWidth(5);
   setSelectRegionHeight(5);

   if (m_selectionHits == -1) {
      QLOG_WARN() << "Selection overflow";
      setSelectedName(-1);
      return;
   } else if (m_selectionHits == 0) {
      setSelectedName(-1);
      return;
   }


   // If the user clicks, then we only select the front object
   Handler::SelectionMode selectionMode(m_currentHandler->selectionMode());
   if ( (selectionMode == Handler::AddClick) || (selectionMode == Handler::RemoveClick) ||
        (selectionMode == Handler::ToggleClick) ) {

      GLuint zMin = (selectBuffer())[1];
      int iMin = 0;

      for (int i = 1; i < m_selectionHits; ++i) {
          if ((selectBuffer())[4*i+1] < zMin) {
             iMin = i;
             zMin = (selectBuffer())[4*i+1];
          }
      }

      GLuint name = (selectBuffer())[4*iMin+3];

/*
      QLOG_TRACE() << "Viewer::endSelection with nhits:" 
                   << m_selectionHits << m_objects.size();
      if (name < m_objects.size()) {
         QLOG_TRACE() << "  Selection made:" << iMin << name << m_objects[name]->index();
      }else {
         QLOG_TRACE() << "  Selection made:" << iMin << name << "Out of range!";
      }
*/

      enableUpdate(false);
      setSelectedName(4*iMin+3);

      if (selectionMode == Handler::AddClick) {
         addToSelection(name);
      }else if (selectionMode == Handler::RemoveClick) {
         removeFromSelection(name);
      }else {
         toggleSelection(name);
      }

      enableUpdate(true);
      updateGL();

   }else {
      // The selection rectangle is non-zero so we select all the objects
      // behind it.
   
      // Interpret results : each object created 4 values in the selectBuffer().
      // (selectBuffer())[4*i+3] is the id pushed on the stack.

	  // Temporarily switch off GL updating so the selection routines don't
	  // trigger an update which makes the slected item appear incrementally.
      enableUpdate(false);
      for (int i = 0; i < m_selectionHits; ++i) {
          switch (m_currentHandler->selectionMode()) {
             case Handler::Add: 
                addToSelection((selectBuffer())[4*i+3]); 
                break;
             case Handler::Remove: 
                removeFromSelection((selectBuffer())[4*i+3]);  
                break;
             case Handler::Toggle: 
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


void Viewer::addToSelection(Layer::GLObject* object) 
{
   select(object->index(), QItemSelectionModel::Select);
}


void Viewer::addToSelection(int id) 
{
   if (id < m_objects.size()) select(m_objects[id]->index(), QItemSelectionModel::Select);
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
         setHandler(Select);
         e->accept();
         break;
      case s_manipulateSelectionKey:
         setHandler(ManipulateSelection);
         e->accept();
         break;
      case s_buildKey:
         m_currentHandler = m_currentBuildHandler;
         setCursor(m_currentHandler->cursorType());
         e->accept();
         break;
      case Qt::Key_A:
         m_viewerModel.toggleAxes(); 
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
      setHandler(m_activeMode);
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
   QLOG_DEBUG() << "Drop Event in Viewer successful";
 
   QList<QUrl> urls(event->mimeData()->urls());
   QList<QUrl>::iterator iter;
   
   for (iter = urls.begin(); iter != urls.end(); ++iter) {
       QString filePath((*iter).path());
       while (filePath.endsWith("/")) {
          filePath.chop(1);
       }
       openFileFromDrop(filePath);
   }
   event->acceptProposedAction();
} 


// --------------- Handler Modes ---------------
void Viewer::setActiveViewerMode(Viewer::Mode const mode)
{
   m_previousMode = m_activeMode;
   m_activeMode = mode;
   setHandler(m_activeMode);
   activeViewerModeChanged(m_activeMode);
}


void Viewer::setHandler(Viewer::Mode const mode)
{
   switch (mode) {
      case Manipulate:
         m_currentHandler = &m_manipulateHandler; 
         break;

      case Select:
         m_currentHandler = &m_selectHandler; 
         break;

      case ManipulateSelection:
         m_currentHandler = &m_manipulateSelectionHandler; 
         break;

      case ReindexAtoms:
         m_currentHandler = &m_reindexAtomsHandler; 
         break;

      case BuildAtom:
         m_currentHandler      = &m_buildAtomHandler;
         m_currentBuildHandler = &m_buildAtomHandler;
         break;

      case BuildFunctionalGroup:
         m_currentHandler      = &m_buildFunctionalGroupHandler;
         m_currentBuildHandler = &m_buildFunctionalGroupHandler;
         break;

      case BuildEfp:
         m_currentHandler      = &m_buildEfpFragmentHandler;
         m_currentBuildHandler = &m_buildEfpFragmentHandler;
         break;

      case BuildMolecule:
         m_currentHandler      = &m_buildMoleculeHandler;
         m_currentBuildHandler = &m_buildMoleculeHandler;
         break;
   }

   m_selectionHighlighting = true;
   setCursor(m_currentHandler->cursorType());
}


// This is the only entry point for the ReindexAtom mode
void Viewer::reindexAtoms(Layer::Molecule* molecule)
{
   setActiveViewerMode(ReindexAtoms);
   m_reindexAtomsHandler.reset(molecule);
   setLabelType(Layer::Atom::Reindex);
   displayMessage("Hit Return to commit changes, Esc to cancel");
}



GLObjectList Viewer::startManipulation(QMouseEvent* event) 
{
   ManipulatedFrame* frame(manipulatedFrame());
   /// The value of 100 effectively turns off auto rotation for the selection
   frame->setSpinningSensitivity(100.0);
   ManipulatedFrameSetConstraint* mfsc = (ManipulatedFrameSetConstraint*)(frame->constraint());
   mfsc->clearSet();

   GLObjectList selection;
   GLObjectList::iterator iter;
   selection = m_selectedObjects;
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


void Viewer::saveSnapshot()
{ 
   QGLViewer::saveSnapshot(false); 
   //Snapshot snap(this);
   //snap.capture();
}


} // end namespace IQmol
