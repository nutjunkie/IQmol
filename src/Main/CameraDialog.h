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

class CameraDialog : public QDialog {

   Q_OBJECT

   public:
      CameraDialog(qglviewer::Camera&, QWidget* parent = 0);

   Q_SIGNALS:
      void updated();
      void resetView();

   public Q_SLOTS:
      void sync();

   private Q_SLOTS:
      void on_okButton_clicked(bool);
      void on_rValue_valueChanged(double)  { updatePosition(); }
      void on_thetaValue_valueChanged(int) { updatePosition(); }
      void on_phiValue_valueChanged(int)   { updatePosition(); }
      void on_resetButton_clicked(bool)    { resetView(); }

      void updatePosition();

      void on_perspectiveButton_clicked(bool tf);
      void on_orthographicButton_clicked(bool tf);

   private:
      qglviewer::Camera& m_camera;
      Ui::CameraDialog m_dialog;
      bool m_emitSignals;
};

} // end namespace IQmol

#endif
