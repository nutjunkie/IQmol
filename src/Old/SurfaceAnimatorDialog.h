#ifndef IQMOL_SURFACEANIMATORDIALOG_H
#define IQMOL_SURFACEANIMATORDIALOG_H
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

#include "ui_SurfaceAnimatorDialog.h"
#include "SurfaceLayer.h"
#include "Animator.h"


namespace IQmol {

   namespace Data {
      class GridData;
   }

   namespace Layer {
      class Molecule;
   }

   /// Configurator Dialog to allow the user to change the appearance of
   /// a Surface Layer.
   class SurfaceAnimatorDialog : public QDialog {

      Q_OBJECT

      public:
         SurfaceAnimatorDialog(Layer::Molecule*);
         ~SurfaceAnimatorDialog();
         void update();

      Q_SIGNALS:
         void pushAnimators(AnimatorList const&);
         void popAnimators(AnimatorList const&);
         void updated();
         void progress(double);

      private Q_SLOTS:
         void on_reverseButton_clicked(bool);
         void on_upButton_clicked(bool);
         void on_downButton_clicked(bool);
         void on_negativeColorButton_clicked(bool);
         void on_positiveColorButton_clicked(bool);
         void on_calculateButton_clicked(bool);
         void on_transparencySlider_valueChanged(int);
         void on_fillButton_clicked(bool);
         void on_linesButton_clicked(bool);
         void on_dotsButton_clicked(bool);

         void on_playButton_clicked(bool);
         void on_backButton_clicked(bool);
         void on_forwardButton_clicked(bool);
         void on_speedSlider_valueChanged(int);

         void on_loopButton_clicked(bool);
         void on_bounceButton_clicked(bool);
         void on_updateBondsButton_clicked(bool);

         void animationStopped();
         //void surfaceCalculationCanceled() { m_surfaceCalculationCanceled = true; }

      private:
         Ui::SurfaceAnimatorDialog m_dialog;

         void setDrawMode(Layer::Surface::DrawMode const);
         void setPositiveColor(QColor const& color);
         void setNegativeColor(QColor const& color);
         void computeMultiGridAnimation();
         void computeIsovalueAnimation();
         Layer::Surface* calculateSurface(Data::GridData const& grid, double const isovalue);

         Layer::Molecule* m_molecule;
         QColor m_colorPositive;
         QColor m_colorNegative;

         double m_speed;
         double m_alpha;

         int  m_referenceFrames;
         bool m_updateBonds;
         Animator::Combo* m_animator;
         bool m_surfaceCalculationCanceled;
         Qt::SortOrder m_sortOrder;
   };

} // end namespace IQmol

#endif
