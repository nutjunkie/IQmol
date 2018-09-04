#ifndef IQMOL_CAMERADIALOG_H
#define IQMOL_CAMERADIALOG_H
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

#include "ui_CameraDialog.h"
#include "QGLViewer/camera.h"


namespace IQmol {

class Viewer;

class CameraDialog : public QDialog {

   Q_OBJECT

   public:
      CameraDialog(qglviewer::Camera&, Viewer* parent = 0);

   Q_SIGNALS:
      void updated();
      void resetView();
      void interpolated();

   public Q_SLOTS:
      void sync();

   private Q_SLOTS:
      void on_rValue_valueChanged(double)  { updatePosition(); }
      void on_thetaValue_valueChanged(int) { updatePosition(); }
      void on_phiValue_valueChanged(int)   { updatePosition(); }
      void on_addFrameButton_clicked(bool);
      void on_deleteFrameButton_clicked(bool);
      void on_okButton_clicked(bool);
      void on_playButton_clicked(bool);
      void on_stopButton_clicked(bool);
      void on_playButton2_clicked(bool);
      void on_resetButton_clicked(bool);
      void on_fieldOfView_valueChanged(int angle);
      void on_perspectiveButton_clicked(bool tf);
      void on_orthographicButton_clicked(bool tf);
      void on_autoRotation_valueChanged(int speed);
      void on_frameTable_itemSelectionChanged();
    
      void stopSpinning();
      void updateSpinAngle();
      void updatePosition();
      void runFrame();

   private:
      qglviewer::Camera& m_camera;
      Ui::CameraDialog m_dialog;
      bool m_emitSignals;
      double m_angleIncrement;
      double m_spinAngle;
      unsigned m_keyFrameRow;
      bool m_bounceStop;
};

} // end namespace IQmol

#endif
