#ifndef IQMOL_SURFACECONFIGURATOR_H
#define IQMOL_SURFACECONFIGURATOR_H
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

#include "Configurator.h"
#include "ui_SurfaceConfigurator.h"
#include "Gradient.h"
#include <QColor>


namespace IQmol {

namespace Layer {
   class Surface;
}

namespace Configurator{

   /// Configurator Dialog to allow the user to change the appearance of
   /// a Surface Layer.
   class Surface : public Base {

      Q_OBJECT

      public:
         explicit Surface(Layer::Surface&);

      public Q_SLOTS:
         void sync();

      private Q_SLOTS:
         void on_positiveColorButton_clicked(bool);
         void on_negativeColorButton_clicked(bool);
         void on_swapColorsButton_clicked(bool);
         void on_propertyCombo_currentIndexChanged(int);
         void on_fillButton_clicked(bool);
         void on_linesButton_clicked(bool);
         void on_dotsButton_clicked(bool);
         void on_transparencySlider_valueChanged(int);
         void editGradientColors(bool);

      private:
         Ui::SurfaceConfigurator m_surfaceConfigurator;

         void setPositiveColor(QColor const& color);
         void setPositiveColor(QList<QColor> const& colors);
         void setNegativeColor(QColor const& color);

         Layer::Surface& m_surface;
         QList<QColor> m_gradientColors;
         bool m_initialized;

         static Gradient::ColorList s_atomColors;
         Gradient::ColorList const& atomColorGradient();
   };

} } // end namespace IQmol::Configurator

#endif
