#ifndef IQMOL_CONFIGURATOR_GEOMETRY_LIST_H
#define IQMOL_CONFIGURATOR_GEOMETRY_LIST_H
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
#include "MoleculeLayer.h"
#include "ui_GeometryListConfigurator.h"
#include <QPen>
#include <QBrush>



namespace IQmol {

class CustomPlot;

namespace Layer {
   class GeometryList;
}

namespace Configurator {

   /// Configurator Dialog which allows the user to select from a list of
   /// different geometries and also allows animation of optimization and 
   /// reaction pathways.
   class GeometryList : public Base {

      Q_OBJECT

      friend class Layer::GeometryList;

      public:
         explicit GeometryList(Layer::GeometryList&);
         ~GeometryList();

      public Q_SLOTS:
         void reset();

      Q_SIGNALS:
         void update();

      protected:
         void load();

      private Q_SLOTS:
         void on_playButton_clicked(bool);
         void on_backButton_clicked(bool);
         void on_forwardButton_clicked(bool);
         void on_speedSlider_valueChanged(int);
         void on_bounceButton_clicked(bool tf);
         void on_loopButton_clicked(bool tf);
         void on_updateBondsButton_clicked(bool tf);
         void on_energyTable_itemSelectionChanged();
         void on_resetViewButton_clicked();
         void plotSelectionChanged(bool);
         void setSelectionRectMode(QMouseEvent* e);

      private:
         void closeEvent(QCloseEvent*);
         void plotEnergies();
         void initPlot();

         Ui::GeometryListConfigurator m_configurator;
         Layer::GeometryList& m_geometryList;
         CustomPlot* m_customPlot;
         QList<QPair<double, double> > m_rawData;
         QPen m_pen;
         QPen m_selectedPen;
   };

} } // End namespace IQmol::Configurator

#endif
