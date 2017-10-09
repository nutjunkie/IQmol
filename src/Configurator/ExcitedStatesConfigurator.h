#ifndef IQMOL_CONFIGURATOR_EXCITEDSTATES_H
#define IQMOL_CONFIGURATOR_EXCITEDSTATES_H
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
#include "ui_ExcitedStatesConfigurator.h"
#include <QPen>


class QCPItemText;
class QCPAbstractItem;


namespace IQmol {

class CustomPlot;

namespace Data {
   class ExcitedStates;
}

namespace Layer {
   class ExcitedStates;
}

namespace Configurator {

   class ExcitedStates : public Base {

      Q_OBJECT

      public:
         explicit ExcitedStates(Layer::ExcitedStates&);
         ~ExcitedStates();

          void load(Data::ExcitedStates const&);

      private Q_SLOTS:
         void plotSelectionChanged(bool);
         void moSelectionChanged(bool);

         void on_impulseButton_clicked(bool);
         void on_gaussianButton_clicked(bool);
         void on_lorentzianButton_clicked(bool);
         void on_widthSlider_valueChanged(int);
         void on_energyTable_itemSelectionChanged();
         void on_resetZoomButton_clicked(bool);
         void setSelectionRectMode(QMouseEvent* e);

      private:
         enum Profile { Gaussian, Lorentzian };

         void initMoPlot();
         void initSpectrum();
         void updateSpectrum();
         void updateMoPlot(int const index);
         void plotImpulse();
         void plotSpectrum(Profile const, double const width);
         void clearTransitionLines();

         Layer::ExcitedStates& m_excitedStates; 
         Ui::ExcitedStatesConfigurator m_configurator;

         CustomPlot* m_moPlot;
         CustomPlot* m_spectrum;
         QCPItemText* m_label;
         QPen m_pen;
         QPen m_selectedPen;
         QList<QPair<double, double> > m_rawData;

         QPair<double, double> m_maxValues;

         QList<QCPAbstractItem*> m_transitionLines;
   };

} } // End namespace IQmol::Configurator

#endif
