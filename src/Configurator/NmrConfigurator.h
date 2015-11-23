#ifndef IQMOL_CONFIGURATOR_NMR_H
#define IQMOL_CONFIGURATOR_NMR_H
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
#include <QPen>
#include <QBrush>


class QCustomPlot;

namespace Ui {
   class NmrConfigurator;
}

namespace IQmol {

namespace Data {
   class Nmr;
   class NmrReference;
}

namespace Layer {
   class Nmr;
}

namespace Configurator {

   class Nmr : public Base {

      Q_OBJECT

      public:
         Nmr(Layer::Nmr&, Data::Nmr&);
         ~Nmr();
      
      private Q_SLOTS:
         void plotSelectionChanged(bool tf);
         void on_shieldingsTable_itemSelectionChanged();
         void on_isotopeCombo_currentIndexChanged(QString const& isotope);
         void on_systemCombo_currentIndexChanged(QString const& system);
         void on_widthSlider_valueChanged(int);
         void on_impulseButton_clicked(bool);
         void on_lorentzianButton_clicked(bool);

      Q_SIGNALS:
         void updated();

      private:
         void initTable();
         void loadShifts(Data::NmrReference const*, QString const& isotope);
         QList<double> computeShifts(Data::NmrReference const*, QString const& isotope);
         void updatePlot();
         void plotImpulse(QList<double> const& data, QPair<double, double> const& range);
         void plotSpectrum(QList<double> const& data, QPair<double, double> const& range);

         QPair<double, double> standardRange(QString const& isotope);
         QString currentIsotope();
         Data::NmrReference const* currentReference();

         Layer::Nmr& m_layer;
         Data::Nmr& m_data;

         Ui::NmrConfigurator* m_ui;
         QCustomPlot* m_plot;

         QPen m_pen;
         QPen m_selectPen;
         QMap<int, QList<int> > m_graphToRows;
         QPair<double, double> m_standardDomain;
   };

} } // end namespace IQmol::Configurator

#endif
