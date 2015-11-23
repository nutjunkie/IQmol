#ifndef IQMOL_CONFIGURATOR_FREQUENCIES_H
#define IQMOL_CONFIGURATOR_FREQUENCIES_H
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

#include "Configurator.h"
#include "ui_FrequenciesConfigurator.h"
#include <QPen>
#include <QBrush>

class QCustomPlot;

namespace IQmol {

namespace Layer {
   class Frequencies;
}

namespace Configurator {

   /// Configurator Dialog which displays a list of vibrational frequecies 
   /// and allows the user to select which one is visible or being animated.
   class Frequencies : public Base {

      Q_OBJECT

      friend class Layer::Frequencies;

      public:
         explicit Frequencies(Layer::Frequencies& frequencies);
         ~Frequencies();
      
      Q_SIGNALS:
         void update();

      public Q_SLOTS:
         void reset();

      protected:
         void load();

      private Q_SLOTS:
         void on_playButton_clicked(bool);
         void on_backButton_clicked(bool);
         void on_forwardButton_clicked(bool);
         void on_amplitudeSlider_valueChanged(int);
         void on_speedSlider_valueChanged(int);
         void on_frequencyTable_itemSelectionChanged();
         void on_frequencyTable_itemDoubleClicked(QTableWidgetItem*);
         void on_colorButton_clicked(bool);
         void on_loopButton_clicked(bool);

         void on_scaleFactor_valueChanged(double);
         void on_impulseButton_clicked(bool);
         void on_gaussianButton_clicked(bool);
         void on_lorentzianButton_clicked(bool);
         void on_widthSlider_valueChanged(int);
         void on_ramanCheckbox_clicked(bool);

         void plotSelectionChanged(bool);
         void contextMenuRequest(QPoint);
         void updatePlot();

      private:
         enum Profile { Gaussian, Lorentzian };

         void closeEvent(QCloseEvent*);
         void setVectorColor(QColor const& color);
         void plotImpulse(double const scaleFactor);
         void plotSpectrum(Profile const, double const scaleFactor, double const width);

         Ui::FrequenciesConfigurator m_configurator;
         Layer::Frequencies& m_frequencies;
         QCustomPlot* m_customPlot;
         QList<QPair<double, double> > m_rawData;
         QList<QPair<double, double> > m_rawRamanData;
         QList<QPair<double, double> > m_rawIrData;
         QPen m_pen;
         QPen m_selectPen;
   };

} } // end namespace IQmol::Configurator

#endif
