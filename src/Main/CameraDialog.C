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

#include "CameraDialog.h"
#include "Numerical.h"
#include "QsLog.h"
#include <cmath>
#include <QtDebug>

using namespace qglviewer;


namespace IQmol {

CameraDialog::CameraDialog(qglviewer::Camera& camera, QWidget* parent) : QDialog(parent),
   m_camera(camera), m_emitSignals(true)
{
   m_dialog.setupUi(this);
   short theta(952);
   short phi(981);
   m_dialog.thetaLabel->setText(QChar(theta));
   m_dialog.phiLabel->setText(QChar(phi));
}


void CameraDialog::sync()
{
   Vec pos(m_camera.position());

   double r(pos.norm());
   double theta(std::acos(pos.z/r));
   double phi(std::atan2(pos.y, pos.x));

   if (theta < 0) theta += M_PI;
   if (phi   < 0) phi   += 2*M_PI;

   theta *= 180.000/M_PI;
   phi   *= 180.000/M_PI;

/*
   qDebug() ;
   qDebug() << "Syncing camera from viewer" << pos.x << pos.y << pos.z  
                                   << "   " << r << theta << phi;
*/

   m_dialog.fieldOfView->setValue();

   m_emitSignals = false;
   m_dialog.rValue->setValue(r);
   m_dialog.thetaValue->setValue(Util::round(theta));
   m_dialog.phiValue->setValue(Util::round(phi));
   m_emitSignals = true;
}


void CameraDialog::updatePosition()
{
   if (!m_emitSignals) return;
   double r(m_dialog.rValue->value());
   double theta(m_dialog.thetaValue->value());
   double phi(m_dialog.phiValue->value());

   theta *= M_PI/180.00000;
   phi   *= M_PI/180.00000;

   double x(r*std::sin(theta)*std::cos(phi));
   double y(r*std::sin(theta)*std::sin(phi));
   double z(r*std::cos(theta));

   m_camera.setPosition(Vec(x,y,z));
   m_camera.lookAt(Vec(0,0,0));
   updated();
}


void CameraDialog::on_fieldOfView_valueChanged(int angle)
{
   m_camera.setFieldOfView(angle);
}



void CameraDialog::on_perspectiveButton_clicked(bool tf)
{
   m_camera.setType(Camera::PERSPECTIVE);
   m_dialog.fieldOfViewLabel->enable();
   m_dialog.fieldOfView->enable();
   updated();
}


void CameraDialog::on_orthographicButton_clicked(bool tf)
{
   m_camera.setType(Camera::ORTHOGRAPHIC);
   m_dialog.fieldOfViewLabel->disable();
   m_dialog.fieldOfView->disable();
   updated();
}


void CameraDialog::on_okButton_clicked(bool)
{
   accept();
}

} // end namespace IQmol
