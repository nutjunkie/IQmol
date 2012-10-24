#ifndef IQMOL_VIEWER_H
#define IQMOL_VIEWER_H
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

#include "ViewerModel.h"
#include "BuildHandler.h"
#include "SelectHandler.h"
#include "ManipulateHandler.h"
#include "ReindexAtomsHandler.h"
#include "ManipulateSelectionHandler.h"
#include "Cursors.h"
#include "AtomLayer.h"
#include "Animator.h"
#include "Snapshot.h"
#include "IQmol.h"
#include "QGLViewer/qglviewer.h"
#include <QFontMetrics>


class QUndoCommand;
class QDropEvent;
class QDragEnterEvent;

namespace qglviewer {
   class Vec;
}

namespace IQmol {

   /// An OpenGL widget based that forms the main display of IQmol.
   /// 
   class Viewer : public QGLViewer {

      Q_OBJECT

      friend class Handler::Build;
      friend class Handler::Select;
      friend class Handler::Manipulate;
      friend class Handler::ReindexAtoms;
      friend class Handler::ManipulateSelection;

      public:
		 /// Enumerates the possible selection behaviours when performing an
		 /// OpenGL select.  The Click modes are distinct from the others in 
         /// that they only select the front-most object.
         enum SelectionMode { None, Add, Remove, Toggle, AddClick, RemoveClick, ToggleClick };

         Viewer(ViewerModel& model, QWidget* parent);
         ~Viewer() { }
         void init();

         void setSelectionMode(SelectionMode const mode);

      Q_SIGNALS:
         void activeViewerModeChanged(ViewerMode const);
         void clearSelection();
         void select(QModelIndex const&, QItemSelectionModel::SelectionFlags);
         void enableUpdate(bool const);
         void postCommand(QUndoCommand*);
         void openFileFromDrop(QString const& file);
         void escapeFullScreen();
         void recordingCancelled();

      public Q_SLOTS:
         void setSceneRadius(double const);
         void resetView();

         /// Provided as a slot as the base function isn't.
         void updateGL() { QGLViewer::updateGL(); }

         void displayMessage(QString const& msg) { QGLViewer::displayMessage(msg, FOREVER); }
         void setActiveViewerMode(ViewerMode const mode);
         void saveSnapshot();
         void setDefaultBuildElement(unsigned int element) { m_buildElement = element; }
         void setLabelType(int const);
         QFont labelFont() const { return s_labelFont; }
         void pushAnimators(AnimatorList const&);
         void popAnimators(AnimatorList const&);
         void popAnimator(Animator::Base*);
         void clearAnimators();
         void reindexAtoms(Layer::Molecule*);
         void setRecordingActive(bool);

      protected:
         void dropEvent(QDropEvent*);
         void dragEnterEvent(QDragEnterEvent*);

      private Q_SLOTS:
         void animate();

      protected:
         unsigned int  m_buildElement;
         ViewerModel&  m_viewerModel;
         GLObjectList  m_buildObjects;
         void setCursor(Cursors::Type const type) { QWidget::setCursor(m_cursors.get(type)); }
         qglviewer::Vec worldCoordinatesOf(QMouseEvent* e, 
            qglviewer::Vec const& hint = qglviewer::Vec(0.0, 0.0, 0.0));
         int m_selectionHits;


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
         void drawGlobals() { m_viewerModel.displayGlobals(); }
         void drawObjects(GLObjectList const&, qglviewer::Vec const& cameraPosition);
         void drawSelected(GLObjectList const&, qglviewer::Vec const& cameraPosition);
         void drawLabels(GLObjectList const&);
         void drawSurfaces(GLObjectList const&, qglviewer::Vec const& cameraPosition);
         void displayGeometricParameter(GLObjectList const& selection);
         void drawWithNames(); 

         void drawSelectionRectangle(QRect const& rect) const;
         void endSelection(QPoint const&);
         void postSelection(QPoint const&);
         void addToSelection(int const id);
         void removeFromSelection(int const id);
         void toggleSelection(int const id);

		 /// This is used to set the Viewer mode when using the hot keys.  It
		 /// updates the cursor and handler, but does not broadcast a signal.
         void setTemporaryViewerMode(ViewerMode const);

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
         ViewerMode   m_activeMode;
         ViewerMode   m_previousMode;
         bool m_selectionHighlighting;  
         Layer::Atom::LabelType m_labelType;
         SelectionMode m_selectionMode;
         
         Cursors               m_cursors;
         Handler::Base*        m_currentHandler;
         Handler::Manipulate   m_manipulateHandler;
         Handler::Select       m_selectHandler;
         Handler::Build        m_buildHandler;
         Handler::ReindexAtoms m_reindexAtomsHandler;
         Handler::ManipulateSelection m_manipulateSelectionHandler;

         Snapshot* m_snapper;
   };


} // end namespace IQmol

#endif
