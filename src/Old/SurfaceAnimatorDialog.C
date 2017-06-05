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

#include "SurfaceAnimatorDialog.h"
#include "Preferences.h"
#include "MoleculeLayer.h"
#include "CubeDataLayer.h"
#include "QVariantPointer.h"
#include "MarchingCubes.h"
#include "MeshDecimator.h"
#include "CubeData.h"
#include "SurfaceInfo.h"
#include "QsLog.h"
#include "Geometry.h"
#include "QMsgBox.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QProgressDialog>


using namespace qglviewer;


namespace IQmol {

SurfaceAnimatorDialog::SurfaceAnimatorDialog(Layer::Molecule* molecule) : 
   QDialog(QApplication::activeWindow()), m_molecule(molecule), m_updateBonds(false), 
   m_animator(0), m_sortOrder(Qt::DescendingOrder)
{
   m_dialog.setupUi(this);
   m_colorPositive = Preferences::PositiveSurfaceColor(); 
   m_colorNegative = Preferences::NegativeSurfaceColor(); 
   setPositiveColor(m_colorPositive);
   setNegativeColor(m_colorNegative);
   m_dialog.playbackBox->setEnabled(false);

   m_dialog.speedSlider->setValue(25);

   on_speedSlider_valueChanged(25);
   m_dialog.transparencySlider->setValue(100);

   connect(this, SIGNAL(pushAnimators(AnimatorList const&)),
      m_molecule, SIGNAL(pushAnimators(AnimatorList const&)));

   connect(this, SIGNAL(popAnimators(AnimatorList const&)),
      m_molecule, SIGNAL(popAnimators(AnimatorList const&)));

   connect(this, SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));
}


SurfaceAnimatorDialog::~SurfaceAnimatorDialog()
{
   m_dialog.fileList->clear();
}


void SurfaceAnimatorDialog::update()
{
   QListWidget* fileList(m_dialog.fileList);
   fileList->clear();

   QList<Layer::CubeData*> cubeFiles(m_molecule->findLayers<Layer::CubeData>(Layer::Children));
   QList<Layer::CubeData*>::iterator iter;
   for (iter = cubeFiles.begin(); iter != cubeFiles.end(); ++iter) {
       QListWidgetItem* item = new QListWidgetItem();
       item->setText((*iter)->cubeData().label());
       QVariant qvar(QVariantPointer<Layer::CubeData>::toQVariant(*iter)); 
       item->setData(Qt::UserRole, qvar);
       fileList->addItem(item);
   }

   m_dialog.isovalue2->setVisible(fileList->count() == 1);
   m_dialog.toLabel->setVisible(fileList->count() == 1);
}


void SurfaceAnimatorDialog::on_reverseButton_clicked(bool)
{
   QListWidget* fileList(m_dialog.fileList);
   if (m_sortOrder == Qt::AscendingOrder) {
      m_sortOrder = Qt::DescendingOrder;
   }else {
      m_sortOrder = Qt::AscendingOrder;
   }
   fileList->sortItems(m_sortOrder);
}


void SurfaceAnimatorDialog::on_upButton_clicked(bool)
{
   QListWidget* fileList(m_dialog.fileList);
   int row(fileList->currentRow());
   if (row == 0) return;

   QListWidgetItem* item(fileList->takeItem(row));
   fileList->insertItem(row-1, item);
   fileList->setCurrentItem(item);
}


void SurfaceAnimatorDialog::on_downButton_clicked(bool)
{
   QListWidget* fileList(m_dialog.fileList);
   int row(fileList->currentRow());
   if (row == fileList->count()-1) return;

   QListWidgetItem* item(fileList->takeItem(row));
   fileList->insertItem(row+1, item);
   fileList->setCurrentItem(item);
}


void SurfaceAnimatorDialog::on_calculateButton_clicked(bool)
{
   if (m_animator) {
      on_playButton_clicked(false);
      m_dialog.loopButton->setChecked(false);
      m_dialog.bounceButton->setChecked(false);
      delete m_animator;
      m_animator = 0;
   }

   QListWidget* fileList(m_dialog.fileList);
   
   if (fileList->count() == 1) {
      computeIsovalueAnimation();
   }else {
      computeMultiGridAnimation();
   }
}


void SurfaceAnimatorDialog::on_positiveColorButton_clicked(bool)
{
   setPositiveColor(QColorDialog::getColor(m_colorPositive, this));
}


void SurfaceAnimatorDialog::on_negativeColorButton_clicked(bool)
{
   setNegativeColor(QColorDialog::getColor(m_colorNegative, this));
}


void SurfaceAnimatorDialog::setPositiveColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_dialog.positiveColorButton->setStyleSheet(bg);
      Preferences::PositiveSurfaceColor(color);
   }
}


void SurfaceAnimatorDialog::setNegativeColor(QColor const& color)
{
   if (color.isValid()) {
      QString bg("background-color: ");
      bg += color.name();
      m_dialog.negativeColorButton->setStyleSheet(bg);
      Preferences::NegativeSurfaceColor(color);
   }
}


void SurfaceAnimatorDialog::on_transparencySlider_valueChanged(int value)
{
   m_alpha = value/100.0;
   if (m_animator) {
      m_animator->setAlpha(m_alpha);
      updated();
   }
}


void SurfaceAnimatorDialog::on_fillButton_clicked(bool)
{
   setDrawMode(Layer::Surface::Fill);
}


void SurfaceAnimatorDialog::on_linesButton_clicked(bool)
{
   setDrawMode(Layer::Surface::Lines);
}


void SurfaceAnimatorDialog::on_dotsButton_clicked(bool)
{
   setDrawMode(Layer::Surface::Dots);
}


void SurfaceAnimatorDialog::setDrawMode(Layer::Surface::DrawMode const mode)
{
   if (m_animator) {
      m_animator->setDrawMode(mode);
      updated();
   }
}


void SurfaceAnimatorDialog::computeIsovalueAnimation()
{
   QListWidget* fileList(m_dialog.fileList);
   QListWidgetItem* item(fileList->item(0));
   if (!item) return;

   int    interpolationFrames(m_dialog.interpolationFrames->value());
   int    nFrames(2+interpolationFrames);
   double isovalue1(m_dialog.isovalue->value());
   double isovalue2(m_dialog.isovalue2->value());
   double dIso((isovalue2-isovalue1)/(nFrames-1));

   Layer::CubeData* cube;
   cube = QVariantPointer<Layer::CubeData>::toPointer(item->data(Qt::UserRole));
   if (!cube) return;

   Data::SurfaceType surfaceType(Data::SurfaceType::CubeData);

   Layer::Surface* surface(0);
   Animator::Combo::DataList frames;
   Data::Geometry const& geom(cube->cubeData().geometry());

   unsigned quality(0);
   bool isSigned(true);
   bool simplifyMesh(m_dialog.simplifyMesh->isChecked());

   QProgressDialog* progressDialog(new QProgressDialog("Calculating surfaces", "Cancel", 0, 
      nFrames));
   progressDialog->setWindowModality(Qt::WindowModal);

   int totalProgress(0);
   progressDialog->setValue(totalProgress);

   for (int i = 0; i < nFrames; ++i) {
       if (surface) surface->setCheckState(Qt::Unchecked);

       Data::SurfaceInfo surfaceInfo(Data::SurfaceType::CubeData, quality, isovalue1,
          m_colorPositive, m_colorNegative, isSigned, simplifyMesh);

       surface = cube->calculateSurface(surfaceInfo);

       if (surface) {
          surface->setAlpha(m_alpha);
          frames.append(new Animator::Combo::Data(geom, surface));
       }
       isovalue1 += dIso;

       QApplication::processEvents();
       ++totalProgress;
       progressDialog->setValue(totalProgress);
       if (progressDialog->wasCanceled()) {
#ifndef Q_OS_WIN32
 //         delete progressDialog;
#endif
          return;
       }
   }

   //if (progressDialog) progressDialog->hide();

   m_animator = new Animator::Combo(m_molecule, frames, interpolationFrames, m_speed);
   connect(m_animator, SIGNAL(finished()), this, SLOT(animationStopped()));
   m_dialog.playbackBox->setEnabled(true); 
   m_animator->setLoopMode(m_dialog.loopButton->isChecked());
   m_animator->setBounceMode(m_dialog.bounceButton->isChecked());
#ifndef Q_OS_WIN32
//   delete progressDialog;
#endif
}


// reworked version that allows for movement of the nuclei too

void SurfaceAnimatorDialog::computeMultiGridAnimation()
{
   QListWidget* fileList(m_dialog.fileList);
   m_referenceFrames = fileList->count();
   if (m_referenceFrames < 2) return;

   int interpolationFrames(m_dialog.interpolationFrames->value());
   double isovalue(m_dialog.isovalue->value());
   double delta = 1.0/(interpolationFrames+1);

   QListWidgetItem* item(fileList->item(0));
   Layer::CubeData* cube(QVariantPointer<Layer::CubeData>::toPointer(item->data(Qt::UserRole)));

   Animator::Combo::DataList frames;
   Layer::Surface* surface;
   QString label;

   int totalProgress(0);
   int totalSurfaces(m_referenceFrames + (m_referenceFrames-1)*interpolationFrames -1);
   QProgressDialog* progressDialog(new QProgressDialog("Calculating surfaces", 
      "Cancel", 0, totalSurfaces, this));
      
   progressDialog->setWindowModality(Qt::WindowModal);
   progressDialog->setValue(totalProgress);

   Data::CubeData* A(new Data::CubeData(cube->cubeData()));
   Data::CubeData* B(0);

   // loop over grids
   for (int i = 1; i < m_referenceFrames; ++i) {
       if (progressDialog->wasCanceled()) {
          delete A;
          return;
       }

       Data::Geometry const& geomA(cube->cubeData().geometry());
       item = fileList->item(i);
       cube = QVariantPointer<Layer::CubeData>::toPointer(item->data(Qt::UserRole));
       Data::Geometry const& geomB(cube->cubeData().geometry());

       // Geometry displacements
       QList<Vec> displacements;
       for (unsigned a = 0; a < geomA.nAtoms(); ++a) {
           Vec d(geomB.position(a)-geomA.position(a));
           displacements.append(d);
       }

       // Grid displacements
       B = new Data::CubeData(cube->cubeData());

       Data::GridData dAB(*A);
       Data::GridData   t(*A);
       dAB.combine(-1.0, 1.0, *B);

       surface = calculateSurface(dAB, isovalue);
       surface->setText("Difference Surface");
       cube->appendLayer(surface);

       dAB *= delta;

       // loop over interpolation Frames
       for (int j = 0; j <= interpolationFrames; ++j) {
           Data::Geometry geomT;
           for (unsigned a = 0; a < geomA.nAtoms(); ++a) {
               double step = (double)j/(double)(interpolationFrames+1);
               Vec d(geomA.position(a) + step*displacements[a]);
               geomT.append(geomA.atomicNumber(a), d);
           }

           surface = calculateSurface(t, isovalue);
           frames.append(new Animator::Combo::Data(geomT, surface));
           label = (j == 0) ? "Cube Data " + QString::number(i) 
                            : "Interpolation Frame " + QString::number(j);
           surface->setText(label);
           cube->appendLayer(surface);

           ++totalProgress;
           progressDialog->setValue(totalProgress);
           QApplication::processEvents();
           if (progressDialog->wasCanceled()) {
              delete A;
              delete B;
              return;
           }

           t += dAB;  // increment the interpolation grid
       }

       delete A;
       A = B;
   }

   // Take care of the final reference frame
   surface = calculateSurface(*B, isovalue);
   surface->setText("Cube Data " + QString::number(m_referenceFrames));
   cube->appendLayer(surface);

   frames.append(new Animator::Combo::Data(cube->cubeData().geometry(), surface));
   m_animator = new Animator::Combo(m_molecule, frames, interpolationFrames, m_speed);
   connect(m_animator, SIGNAL(finished()), this, SLOT(animationStopped()));
   m_dialog.playbackBox->setEnabled(true); 
   m_animator->setLoopMode(m_dialog.loopButton->isChecked());
   m_animator->setBounceMode(m_dialog.bounceButton->isChecked());

   progressDialog->hide();
   //   progressDialog->deleteLater();
   delete B;
}


Layer::Surface* SurfaceAnimatorDialog::calculateSurface(Data::GridData const& grid, 
   double const isovalue)
{
   bool isSigned(true);
   bool simplifyMesh(m_dialog.simplifyMesh->isChecked());

   Data::SurfaceType type(Data::SurfaceType::CubeData);
   Data::SurfaceInfo surfaceInfo(type, 0, isovalue, m_colorPositive, m_colorNegative, 
      isSigned, simplifyMesh);

   Data::Surface* surfaceData(new Data::Surface(surfaceInfo));

   MarchingCubes mc(grid);
   mc.generateMesh( isovalue, surfaceData->meshPositive());
   mc.generateMesh(-isovalue, surfaceData->meshNegative());

   qglviewer::Vec d(grid.delta());
   double delta((d.x+d.y+d.z)/3.0);

   if (surfaceInfo.simplifyMesh()) {
      MeshDecimator posdec(surfaceData->meshPositive());
      if (!posdec.decimate(delta)) {
         QLOG_ERROR() << "Mesh decimation failed:" << posdec.error();
      }
      MeshDecimator negdec(surfaceData->meshNegative());
      if (!negdec.decimate(delta)) {
         QLOG_ERROR() << "Mesh decimation failed:" << negdec.error();
      }
   }

   Layer::Surface* surfaceLayer(new Layer::Surface(*surfaceData));

   surfaceLayer->setAlpha(m_alpha);
   surfaceLayer->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |
      Qt::ItemIsEnabled | Qt::ItemIsEditable);
   if (m_molecule) connect(surfaceLayer, SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));

   return surfaceLayer;
}


void SurfaceAnimatorDialog::on_playButton_clicked(bool play)
{
   m_dialog.playButton->setChecked(play);
   if (play) {
      if (m_animator) {
         m_animator->reset();
         AnimatorList animators;
         animators << m_animator;
         pushAnimators(animators);
      }
   }else {
      AnimatorList animators;
      animators << m_animator;
      popAnimators(animators);
      m_molecule->reperceiveBondsForAnimation();
   }
}


void SurfaceAnimatorDialog::on_backButton_clicked(bool)
{
   if (m_animator) m_animator->stepBack();
}


void SurfaceAnimatorDialog::on_forwardButton_clicked(bool)
{
   if (m_animator) m_animator->stepForward();
}


void SurfaceAnimatorDialog::on_speedSlider_valueChanged(int value)
{
   m_speed = value/1000.0;
   if (m_animator) m_animator->setSpeed(m_speed);
}


void SurfaceAnimatorDialog::on_bounceButton_clicked(bool bounce)
{
   if (m_animator) m_animator->setBounceMode(bounce);
}


void SurfaceAnimatorDialog::on_loopButton_clicked(bool loop)
{
   if (m_animator) m_animator->setLoopMode(loop);
}


void SurfaceAnimatorDialog::on_updateBondsButton_clicked(bool update)
{
   m_molecule->setReperceiveBondsForAnimation(update);
}


void SurfaceAnimatorDialog::animationStopped()
{
   on_playButton_clicked(false);
}



} // end namespace IQmol
