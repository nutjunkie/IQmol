#ifndef IQMOL_VIEWER_H
#define IQMOL_VIEWER_H
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

#include "SelectHandler.h"
#include "BuildEfpFragmentHandler.h"
#include "BuildAtomHandler.h"
#include "BuildMoleculeFragmentHandler.h"
#include "BuildFunctionalGroupHandler.h"
#include "ManipulateHandler.h"
#include "ReindexAtomsHandler.h"
#include "ManipulateSelectionHandler.h"
#include "Cursors.h"
#include "AtomLayer.h"
#include "Animator.h"
#include "Snapshot.h"
#include "IQmol.h"
#include "QGLViewer/qglviewer.h"
#include "QGLViewer/manipulatedFrame.h"
#include <QFontMetrics>
#include <QStandardItemModel>
#include <QItemSelectionModel>


class QUndoCommand;
class QDropEvent;
class QDragEnterEvent;

namespace qglviewer {
   class Vec;
}

namespace IQmol {

   class ViewerModel;
   class ShaderDialog;
   class ShaderLibrary;
   class CameraDialog;

   /// An OpenGL widget based that forms the main display of IQmol.
   class Viewer : public QGLViewer {

      Q_OBJECT

      using QGLViewer::select;

      friend class Handler::Select;
      friend class Handler::Manipulate;
      friend class Handler::ReindexAtoms;
      friend class Handler::Build;
      friend class Handler::BuildAtom;
      friend class Handler::BuildFunctionalGroup;
      friend class Handler::BuildMoleculeFragment;
      friend class Handler::BuildEfpFragment;
      friend class Handler::ManipulateSelection;

      public:
         enum Mode { Manipulate, Select, ManipulateSelection, ReindexAtoms,
            BuildAtom, BuildFunctionalGroup, BuildEfp, BuildMolecule };

         Viewer(QGLContext* context, ViewerModel& model, QWidget* parent);
         ~Viewer();

         void init();
         void initShaders();

		 // Returns the number of hits for the last select() action.  Note this
		 // is not necessarily the same as the size of m_selectedObjects
         int  selectionHits() const { return m_selectionHits; }
         void setSelectionHighlighting(bool const tf) { m_selectionHighlighting = tf; }
         void editShaders();
         void editCamera();

      Q_SIGNALS:
         void activeViewerModeChanged(Viewer::Mode const);
         void clearSelection();
         void select(QModelIndex const&, QItemSelectionModel::SelectionFlags);
         void enableUpdate(bool const);
         void postCommand(QUndoCommand*);
         void openFileFromDrop(QString const& file);
         void escapeFullScreen();
         void recordingCanceled();
         void animationStep();

      public Q_SLOTS:
         void setDefaultSceneRadius() { setSceneRadius(DefaultSceneRadius); }
         void setSceneRadius(double const);
         void resetView();

         /// Provided as a slot as the base function isn't.
         void updateGL() { QGLViewer::updateGL(); }
         void resizeGL(int width, int height);
         void generatePovRay(QString const& filename);

         void displayMessage(QString const& msg) { QGLViewer::displayMessage(msg, FOREVER); }
         void setActiveViewerMode(Viewer::Mode const mode);
         void saveSnapshot();
         void setDefaultBuildElement(unsigned int element);
         void setDefaultBuildFragment(QString const& fileName, Viewer::Mode const);
         void setLabelType(int const);
         QFont labelFont() const { return s_labelFont; }
         void pushAnimators(AnimatorList const&);
         void popAnimators(AnimatorList const&);
         void popAnimator(Animator::Base*);
         void clearAnimators();
         void reindexAtoms(Layer::Molecule*);
         void setRecord(bool);
         void blockUpdate(bool tf);
         void movieMakingFinished();

      protected:
         void dropEvent(QDropEvent*);
         void dragEnterEvent(QDragEnterEvent*);

      private Q_SLOTS:
         void animate();

      protected:
         ViewerModel&  m_viewerModel;
         void setCursor(Cursors::Type const type) { QWidget::setCursor(m_cursors.get(type)); }
         qglviewer::Vec worldCoordinatesOf(QMouseEvent* e, 
            qglviewer::Vec const& hint = qglviewer::Vec(0.0, 0.0, 0.0));

      private:
         static const Qt::Key s_buildKey;
         static const Qt::Key s_selectKey;
         static const Qt::Key s_manipulateSelectionKey;
         static const Qt::KeyboardModifier s_buildModifier;
         static const Qt::KeyboardModifier s_selectModifier;
         static const Qt::KeyboardModifier s_manipulateSelectionModifier;

         static QFont s_labelFont;
         static QFontMetrics s_labelFontMetrics;

         void draw();
         void fastDraw();
         void drawGlobals();
         void drawObjects(GLObjectList const&);
         void drawSelected(GLObjectList const&);
         void drawLabels(GLObjectList const&);
         void displayGeometricParameter(GLObjectList const& selection);
         void drawWithNames(); 

         void drawSelectionRectangle(QRect const& rect) const;
         void endSelection(QPoint const&);
         void postSelection(QPoint const&);
         void addToSelection(Layer::GLObject*);
         void addToSelection(int const id);
         void removeFromSelection(int const id);
         void toggleSelection(int const id);
         void setHandler(Viewer::Mode const);

         // Event handlers
         void mousePressEvent(QMouseEvent *e);
         void mouseMoveEvent(QMouseEvent *e);
         void mouseReleaseEvent(QMouseEvent *e);
         void mouseDoubleClickEvent(QMouseEvent* e);
         void wheelEvent(QWheelEvent* e);
         void keyPressEvent(QKeyEvent *e);
         void keyReleaseEvent(QKeyEvent *e);
         void leaveEvent(QEvent*);
         void enterEvent(QEvent*);

		 /// This function is called at the start of a manipulate selection
		 /// action.  In general it gathers a list of the selected objects and
		 /// adds these to the ManipulatedFrameSetConstraint.  The
		 /// ManipulatedFrame (installed into the Viewer) then becomes the active
		 /// MouseGrabber.  If only a bond is selected then it is used to
		 /// partition the atoms into two groups, the closest of which is added
		 /// to the ManipulatedFrameSetConstraint.  The translations and rotations
         //  are then constrained to be along and around the bond, respectively.
         GLObjectList startManipulation(QMouseEvent *e);

         AnimatorList m_animatorList;
         GLObjectList m_objects;
         GLObjectList m_selectedObjects;

         // State variables
         Viewer::Mode m_activeMode;
         Viewer::Mode m_previousMode;
         Viewer::Mode m_buildMode;
         bool m_selectionHighlighting;  
         int  m_selectionHits;
         Layer::Atom::LabelType m_labelType;
         Cursors m_cursors;
         
         Handler::Base*   m_currentHandler;
         Handler::Build*  m_currentBuildHandler;

         Handler::Select                m_selectHandler;
         Handler::Manipulate            m_manipulateHandler;
         Handler::BuildEfpFragment      m_buildEfpFragmentHandler;
         Handler::BuildAtom             m_buildAtomHandler;
         Handler::BuildFunctionalGroup  m_buildFunctionalGroupHandler;
         Handler::BuildMoleculeFragment m_buildMoleculeHandler;
         Handler::ReindexAtoms          m_reindexAtomsHandler;
         Handler::ManipulateSelection   m_manipulateSelectionHandler;

         Snapshot* m_snapper;
         bool m_blockUpdate;
         bool m_shadersInit;

         QGLContext*    m_glContext;
         ShaderLibrary* m_shaderLibrary;
         ShaderDialog*  m_shaderDialog;
         CameraDialog*  m_cameraDialog;
   };


} // end namespace IQmol

#endif
