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
#include "Viewer.h"
#include "QsLog.h"
#include "QVariantPointer.h"
#include "QGLViewer/manipulatedCameraFrame.h"
#include <cmath>
#include <QtDebug>
#include <QTimer>

using namespace qglviewer;


namespace IQmol {

CameraDialog::CameraDialog(qglviewer::Camera& camera, Viewer* viewer) : QDialog(viewer),
   m_camera(camera), m_emitSignals(true), m_spinAngle(0.0), m_bounceStop(false)
{
   m_dialog.setupUi(this);
   short theta(952);
   short phi(981);
   m_dialog.thetaLabel->setText(QChar(theta));
   m_dialog.phiLabel->setText(QChar(phi));

   QTableWidget* table(m_dialog.frameTable);
   table->setColumnWidth(0,30);
   table->horizontalHeaderItem(0)->setText("Time (s)");
   table->horizontalHeaderItem(1)->setText("Camera Position");
   table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   table->setSelectionBehavior(QAbstractItemView::SelectRows);
   table->setSelectionMode(QAbstractItemView::SingleSelection);
}


void CameraDialog::sync()
{
   Vec pos(m_camera.position());

   double r(pos.norm());
   double theta(std::acos(pos.z/r));
   double phi(std::atan2(pos.y, pos.x));

   if (theta < 0) theta += M_PI;
   if (phi   < 0) phi   += 2*M_PI;

   theta *= 180.0/M_PI;
   phi   *= 180.0/M_PI;

   double fov(m_camera.fieldOfView());
   m_dialog.fieldOfView->setValue(Util::round(fov*180.0/M_PI));

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
   m_camera.setFieldOfView(angle*M_PI/180.0);
   updated();
}


void CameraDialog::on_perspectiveButton_clicked(bool)
{
   m_camera.setType(Camera::PERSPECTIVE);
   m_dialog.fieldOfViewLabel->setEnabled(true);
   m_dialog.fieldOfView->setEnabled(true);
   updated();
}


void CameraDialog::on_orthographicButton_clicked(bool)
{
   m_camera.setType(Camera::ORTHOGRAPHIC);
   m_dialog.fieldOfViewLabel->setEnabled(false);
   m_dialog.fieldOfView->setEnabled(false);
   updated();
}


void CameraDialog::on_autoRotation_valueChanged(int speed)
{
   if (speed == 0) {
      m_spinAngle  = 0.0;
      m_bounceStop = false;
      m_camera.frame()->stopSpinning();
      disconnect(m_camera.frame(), SIGNAL(spun()), this, SIGNAL(interpolated()));
      disconnect(m_camera.frame(), SIGNAL(spun()), this, SLOT(updateSpinAngle()));
   }else {
      m_angleIncrement = 0.01*speed;
      m_camera.frame()->setSpinningQuaternion(Quaternion(Vec(0,1,0), -m_angleIncrement));

      if (!m_camera.frame()->isSpinning()) {
         connect(m_camera.frame(), SIGNAL(spun()), this, SIGNAL(interpolated()));
         connect(m_camera.frame(), SIGNAL(spun()), this, SLOT(updateSpinAngle()));
         m_camera.frame()->startSpinning(40); // msec
      }
   }
   updated();
}


void CameraDialog::updateSpinAngle()
{
   const double bounceAngle(0.25*M_PI);

   bool bounce(m_dialog.bounceCheckBox->isChecked());
   bool loop(m_dialog.loopCheckBox->isChecked());

   m_spinAngle += m_angleIncrement;

   if (bounce) {
      if (std::abs(m_spinAngle) >= bounceAngle) {
         m_angleIncrement = -m_angleIncrement;
         m_bounceStop = (m_angleIncrement > 0.0);
         m_camera.frame()->setSpinningQuaternion(Quaternion(Vec(0,1,0), -m_angleIncrement));
      }

      if (!loop && m_bounceStop && m_spinAngle >= 0) stopSpinning();
   }else {
      if (!loop && std::abs(m_spinAngle) >= 2.0*M_PI) stopSpinning();
   }
}


void CameraDialog::stopSpinning()
{
   m_dialog.autoRotation->setValue(0);
}


void CameraDialog::on_addFrameButton_clicked(bool)
{
   double time(m_dialog.time->value());
   Frame* cameraFrame(new Frame(*(m_camera.frame())));
   Vec pos(cameraFrame->position());

   QString sTime(QString::number(time,'f',2));
   QString sPos("(");
   sPos += QString::number(pos.x,'f',2) + ",";
   sPos += QString::number(pos.y,'f',2) + ",";
   sPos += QString::number(pos.z,'f',2) + ")";

   QTableWidget* table(m_dialog.frameTable);
   QTableWidgetItem* item;

   table->insertRow(table->rowCount());

   item = new QTableWidgetItem();
   item->setData(Qt::DisplayRole, time);
   table->setItem(table->rowCount()-1, 0, item);

   item = new QTableWidgetItem(sPos);
   QVariant v(QVariantPointer<Frame>::toQVariant(cameraFrame));
   item->setData(Qt::UserRole, v);
   table->setItem(table->rowCount()-1, 1, item);

   table->sortByColumn(0, Qt::AscendingOrder);

   m_dialog.time->setValue(time+1.0);
}


void CameraDialog::on_playButton_clicked(bool)
{
   return on_playButton2_clicked(true);

   // Concatinating the frames leads to some weird interpolation effects
   QTableWidget* table(m_dialog.frameTable);
   qDebug() << "Playing" << table->rowCount() << "camera key frames";
   if (table->rowCount() == 0) return;

   KeyFrameInterpolator* keyFrameInterpolator(m_camera.keyFrameInterpolator(0));
   if (!keyFrameInterpolator) {
      keyFrameInterpolator = new KeyFrameInterpolator(m_camera.frame()); 
      m_camera.setKeyFrameInterpolator(0, keyFrameInterpolator);
      connect(keyFrameInterpolator, SIGNAL(interpolated()), this, SIGNAL(interpolated()));
   }

   keyFrameInterpolator->deletePath();

   bool ok;
   double time;
   QTableWidgetItem* item;

   for (int row = 0; row < table->rowCount(); ++row) {
       item = table->item(row,0);
       time = item->text().toDouble(&ok);
       item = table->item(row,1);
       Frame* frame = QVariantPointer<Frame>::toPointer(item->data(Qt::UserRole));
       if (frame && ok) {
          qDebug() << "Adding frame" << frame << "time" << time;
          keyFrameInterpolator->addKeyFrame(*frame, time);
       }
   }

   //keyFrameInterpolator->drawPath();
   updated();

   if (keyFrameInterpolator) {
      qDebug() << "keyFrameInterpolator" << keyFrameInterpolator << "activating";
      qDebug() << "Duration            " << keyFrameInterpolator->duration();
      qDebug() << "Number of frames    " << keyFrameInterpolator->numberOfKeyFrames();
      qDebug() << "Interpolation period" << keyFrameInterpolator->interpolationPeriod();
      qDebug() << "Interpolation frame " << keyFrameInterpolator->frame();
      qDebug() << "Camera frame        " << m_camera.frame();
   }

   keyFrameInterpolator->setInterpolationTime(0.0);
   m_camera.playPath(0);
}

void CameraDialog::on_stopButton_clicked(bool)
{
   KeyFrameInterpolator* keyFrameInterpolator(m_camera.keyFrameInterpolator(0));
   if (keyFrameInterpolator) keyFrameInterpolator->stopInterpolation();
}

void CameraDialog::on_playButton2_clicked(bool)
{
   QTableWidget* table(m_dialog.frameTable);
   qDebug() << "Playing" << table->rowCount() << "camera key frames";
   if (table->rowCount() < 2) return;

   KeyFrameInterpolator* keyFrameInterpolator(m_camera.keyFrameInterpolator(0));
   if (!keyFrameInterpolator) {
      keyFrameInterpolator = new KeyFrameInterpolator(m_camera.frame()); 
      m_camera.setKeyFrameInterpolator(0, keyFrameInterpolator);
      connect(keyFrameInterpolator, SIGNAL(interpolated()), this, SIGNAL(interpolated()));
      connect(keyFrameInterpolator, SIGNAL(endReached()),   this, SLOT(runFrame()));
   }   

   keyFrameInterpolator->stopInterpolation();

   m_keyFrameRow = 0;
   runFrame();
}


void CameraDialog::runFrame()
{
   QTableWidget* table(m_dialog.frameTable);
   if (m_keyFrameRow+1 >= table->rowCount()) return;

   KeyFrameInterpolator* keyFrameInterpolator(m_camera.keyFrameInterpolator(0));
   keyFrameInterpolator->deletePath();

   bool ok; 
   double start, end; 
   Frame* frame;
   QTableWidgetItem* item;

   // First frame
   item  = table->item(m_keyFrameRow,0);
   start = item->text().toDouble(&ok);
   item  = table->item(m_keyFrameRow,1);
   frame = QVariantPointer<Frame>::toPointer(item->data(Qt::UserRole));

   if (!ok || !frame) return;
   qDebug() << "Adding first  key frame row" << m_keyFrameRow;
   keyFrameInterpolator->addKeyFrame(*frame, 0.0);

   // Second frame
   item  = table->item(m_keyFrameRow+1,0);
   end   = item->text().toDouble(&ok);
   item  = table->item(m_keyFrameRow+1,1);
   frame = QVariantPointer<Frame>::toPointer(item->data(Qt::UserRole));

   if (!ok || !frame) return;
   qDebug() << "Adding second key frame row" << m_keyFrameRow+1 << "duration" << end-start;
   keyFrameInterpolator->addKeyFrame(*frame, end-start);

   keyFrameInterpolator = m_camera.keyFrameInterpolator(0);
   keyFrameInterpolator->setInterpolationTime(0.0);
   m_camera.playPath(0);
   ++m_keyFrameRow;
}


void CameraDialog::on_deleteFrameButton_clicked(bool)
{
   QTableWidget* table(m_dialog.frameTable);
   QList<QTableWidgetItem*> selection(table->selectedItems());
   if (selection.isEmpty()) return;

   int row(selection.first()->row());
   table->removeRow(row);
   updated();
}


void CameraDialog::on_frameTable_itemSelectionChanged()
{
   QTableWidget* table(m_dialog.frameTable);

   QList<QTableWidgetItem*> selection(table->selectedItems());
   if (selection.isEmpty()) return;

   int row(selection.first()->row());
   qDebug() << "Item selection changed" << row;

   QTableWidgetItem* item(table->item(row,1));
   Frame* frame = QVariantPointer<Frame>::toPointer(item->data(Qt::UserRole));
   if (frame) {
      m_camera.setPosition(frame->position());
      m_camera.setOrientation(frame->orientation());
   }
   updated();
}


void CameraDialog::on_resetButton_clicked(bool)
{ 
   QTableWidget* table(m_dialog.frameTable);
   QTableWidgetItem* item;
   for (int row = 0; row < table->rowCount(); ++row) {
       item = table->item(row,1);
       Frame* frame = QVariantPointer<Frame>::toPointer(item->data(Qt::UserRole));
       if (frame) delete frame;
   }

   table->setRowCount(0);
   m_dialog.autoRotation->setValue(0);
   m_dialog.time->setValue(0.00);
   //resetView(); 
}


void CameraDialog::on_okButton_clicked(bool)
{
   accept();
}

} // end namespace IQmol
