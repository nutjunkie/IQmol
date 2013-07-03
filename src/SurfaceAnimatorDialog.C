/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
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
#include "Grid.h"
#include "Geometry.h"
#include "QMsgBox.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QProgressDialog>


namespace IQmol {

SurfaceAnimatorDialog::SurfaceAnimatorDialog(Layer::Molecule* molecule) : 
   QDialog(QApplication::activeWindow()), m_molecule(molecule), m_loop(false), 
   m_bounce(false), m_updateBonds(false), m_animator(0)
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
       QFileInfo info((*iter)->filePath());
       QListWidgetItem* item = new QListWidgetItem();
       item->setText(info.fileName());
       QVariant qvar(QVariantPointer<Layer::CubeData>::toQVariant(*iter)); 
       item->setData(Qt::UserRole, qvar);
       fileList->addItem(item);
   }

   m_dialog.isovalue2->setVisible(fileList->count() == 1);
   m_dialog.toLabel->setVisible(fileList->count() == 1);
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

   int nFrames(m_dialog.interpolationFrames->value());
   double isovalue1(m_dialog.isovalue->value());
   double isovalue2(m_dialog.isovalue2->value());
   double dIso((isovalue2-isovalue1)/nFrames);

   Layer::CubeData* cube;
   cube = QVariantPointer<Layer::CubeData>::toPointer(item->data(Qt::UserRole));
   if (!cube) return;

   Grid::DataType data(Grid::DataType::CubeData);
   Layer::Surface* surface(0);
   Animator::Combo::DataList frames;
   Data::Geometry const& geom(cube->geometry());

   for (int i = 0; i < nFrames; ++i) {
       if (surface) surface->setCheckState(Qt::Unchecked);
       surface = new Layer::Surface(data, -1, isovalue1, m_colorPositive, m_colorNegative);
       surface->setCubeIsSigned(true);
       cube->calculateSurface(surface);
       surface->setAlpha(m_alpha);
       frames.append(new Animator::Combo::Data(geom, surface));
       isovalue1 += dIso;
   }

   m_animator = new Animator::Combo(m_molecule, frames, nFrames, m_speed);
   m_dialog.playbackBox->setEnabled(true); 
}


// reworked version that allows for movement of the nuclei too

void SurfaceAnimatorDialog::computeMultiGridAnimation()
{
   QListWidget* fileList(m_dialog.fileList);
   m_referenceFrames = fileList->count();
   if (m_referenceFrames < 2) return;

   int interpolationFrames(m_dialog.interpolationFrames->value());
   double isovalue(m_dialog.isovalue->value());
   double delta = 1.0/interpolationFrames;

   QListWidgetItem* item(fileList->item(0));
   Layer::CubeData* cube(QVariantPointer<Layer::CubeData>::toPointer(item->data(Qt::UserRole)));

   Grid* A(cube->grid());
   Grid* B(0);

   Grid::Size size(A->size());
   Grid::DataType dataType(Grid::DataType::CubeData);
   Grid  dAB(dataType, size);
   Grid  t(dataType, size);

   Animator::Combo::DataList frames;
   Layer::Surface* surface;

   QString label;

   int totalSurfaces((m_referenceFrames-1)*interpolationFrames);
   QProgressDialog progressDialog("Calculating surfaces", "Cancel", 0, 
      totalSurfaces, this);
   progressDialog.setWindowModality(Qt::WindowModal);

   int totalProgress(0);
   progressDialog.setValue(totalProgress);

   // loop over grids
   for (int i = 1; i < m_referenceFrames; ++i) {
       if (progressDialog.wasCanceled()) return;

       Data::Geometry const& geomA(cube->geometry());
       item = fileList->item(i);
       cube = QVariantPointer<Layer::CubeData>::toPointer(item->data(Qt::UserRole));
       Data::Geometry const& geomB(cube->geometry());

//qDebug() << "first geom";
//geomA.dump();
//qDebug() << "second geom";
//geomB.dump();

       // Geometry displacements
       QList<Vec> displacements;
       for (int a = 0; a < geomA.nAtoms(); ++a) {
           Vec d(geomB.position(a)-geomA.position(a));
           displacements.append(d);
       }

       // Grid displacements
       B    = cube->grid();
       t    = (*A);
       dAB  = (*B);
       dAB -= t;

       surface = calculateSurface(&dAB, isovalue);
       surface->setText("Difference Surface");
       cube->appendLayer(surface);

       dAB *= delta;

       // loop over interpolation Frames
       for (int j = 0; j < interpolationFrames; ++j) {
           Data::Geometry geomT;
           for (int a = 0; a < geomA.nAtoms(); ++a) {
               double step = (double)j/(double)interpolationFrames;
               Vec d(geomA.position(a) + step*displacements[a]);
               geomT.append(geomA.atomicNumber(a), d);
           }

           surface = calculateSurface(&t, isovalue);
           frames.append(new Animator::Combo::Data(geomT, surface));
           label = (j == 0) ? "Cube Data " + QString::number(i) 
                            : "Interpolation Frame " + QString::number(j);
           surface->setText(label);
           cube->appendLayer(surface);

           ++totalProgress;
           progressDialog.setValue(totalProgress);
           if (progressDialog.wasCanceled()) return;

           t += dAB;  // increment the interpolation grid
       }
       A = B;
   }

   // Take care of the final reference frame
   surface = calculateSurface(B, isovalue);
   surface->setText("Cube Data " + QString::number(m_referenceFrames));
   cube->appendLayer(surface);

   frames.append(new Animator::Combo::Data(cube->geometry(), surface));
   m_animator = new Animator::Combo(m_molecule, frames, interpolationFrames, m_speed);
   connect(m_animator, SIGNAL(finished()), this, SLOT(animationStopped()));
   m_dialog.playbackBox->setEnabled(true); 
}


Layer::Surface* SurfaceAnimatorDialog::calculateSurface(Grid* grid, double const isovalue)
{
   Grid::DataType data(Grid::DataType::CubeData);
   Layer::Surface* surface = 
      new Layer::Surface(data, -1, isovalue, m_colorPositive, m_colorNegative);
   surface->setCubeIsSigned(true);

   MarchingCubes mc(grid);
   surface->setSurfaceData(mc.generateSurface( isovalue), Layer::Surface::Positive);
   surface->setSurfaceData(mc.generateSurface(-isovalue), Layer::Surface::Negative);
   
   surface->setAlpha(m_alpha);
   surface->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |
      Qt::ItemIsEnabled | Qt::ItemIsEditable);
   if (m_molecule) connect(surface, SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));

   return surface;
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
   m_bounce = bounce;
   if (m_animator) m_animator->setBounceMode(m_bounce);
}


void SurfaceAnimatorDialog::on_loopButton_clicked(bool loop)
{
   m_loop = loop;
   int cycles(m_loop ? -1.0 : m_referenceFrames);
   if (m_bounce) cycles *= 2;
   if (m_animator) m_animator->setCycles(cycles);
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
