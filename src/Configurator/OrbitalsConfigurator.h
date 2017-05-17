#ifndef IQMOL_CONFIGURATOR_ORBITALS_H
#define IQMOL_CONFIGURATOR_ORBITALS_H
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
#include "ui_OrbitalsConfigurator.h"
#include "SurfaceInfo.h"
#include <QPen>


class QMouseEvent;

namespace IQmol {

class CustomPlot;

namespace Layer {
   class Orbitals;
}

namespace Configurator {

   /// Dialog that allows the user to select orbitals (and densities) for plotting.
   class Orbitals : public Base {

      Q_OBJECT

      public:
//         enum { AlphaOrbital, BetaOrbital, TotalDensity, SpinDensity, 
//            AlphaDensity, BetaDensity, CustomDensity };

         explicit Orbitals(Layer::Orbitals&);
         ~Orbitals();

      public Q_SLOTS:
         void init();

      Q_SIGNALS:
         void queueSurface(Data::SurfaceInfo const&);
         void calculateSurfaces();
         void clearQueue();

      private Q_SLOTS:
         void on_surfaceType_currentIndexChanged(int);
         void on_addToQueueButton_clicked(bool);
         void on_calculateButton_clicked(bool);
         void on_cancelButton_clicked(bool);
         void on_positiveColorButton_clicked(bool);
         void on_negativeColorButton_clicked(bool);
         void plotSelectionChanged(bool tf);
         void clearSelectedOrbitals(int);

      private:
         void enableOrbitalSelection(bool);
         void enableNegativeColor(bool);
         void updateOrbitalRange(int nElectrons);
         void updateOrbitalRange(int nElectrons, QComboBox*);
         void setPositiveColor(QColor const& color);
         void setNegativeColor(QColor const& color);
         void initPlot();

         Layer::Orbitals& m_orbitals;
         Ui::OrbitalsConfigurator m_configurator;

         unsigned int m_nAlpha;
         unsigned int m_nBeta;
         unsigned int m_nOrbitals;
	 unsigned int m_AlphaHOMO;
	 unsigned int m_BetaHOMO;
         CustomPlot* m_customPlot;
         QPen m_pen;
         QPen m_selectedPen;
   };

} } // End namespace IQmol::Configurator

#endif
