#ifndef IQMOL_GRADIENT_H
#define IQMOL_GRADIENT_H
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

#include "ui_GradientDialog.h"
#include "Preferences.h"
#include <QColor>
#include <QList>
#include <QVariant>


namespace IQmol {
namespace Gradient {

   typedef QList<QColor> ColorList;

   QString ToString(ColorList const&);

   /// Class used to represent a simple color gradient.  The reason for this
   /// is that QGradient does not allow access to the color at an arbitrary 
   /// point on the gradient.  Gradients are made up of equispaced colors 
   /// and the gradient is in the range [0,1].  A Gradient can have a single
   /// color, but that wouldn't be very interesting, would it?
   class Function {

      public:
         Function(ColorList const& colors = Preferences::DefaultGradientColors(), 
            double const min = 0.0, double const max = 1.0);
         Function(Function const& that) { copy(that); }
         Function& operator=(Function const& that);

         QColor colorAt(double val) const;
         ColorList colors() const { return m_colors; }
         void resample(int nColors);

      private:
         void copy(Function const& that);
         QColor interpolateRGB(double delta, QColor const& col1, QColor const& col2) const;
         QColor interpolateHSV(double delta, QColor const& col1, QColor const& col2) const;
         ColorList m_colors;
         double m_min;
         double m_max;
   };



   class Dialog : public QDialog {

      Q_OBJECT

      public:
         enum StandardGradient { Default, Spectrum, Custom }; // linked to GradientDialog.ui
         Dialog(ColorList const&, QWidget* parent = 0);
         Dialog(StandardGradient const, QWidget* parent = 0);
         ColorList colors() const { return m_colors; }
         StandardGradient type() const;

      private Q_SLOTS:
         void on_gradientCombo_currentIndexChanged(int);
         void on_stopsSpin_valueChanged(int);
         void colorChanged(int const index, QColor const&);

      private:
         void setCustom();
         void updateGradient();
         void clearStops();

         Ui::GradientDialog m_dialog;
         ColorList m_colors;
   };



   class StopButton : public QToolButton {

      Q_OBJECT

      public:
         StopButton(QWidget* parent, int const index, QColor const& color);
         void setColor(QColor const&);
         int index() const { return m_index; }

      Q_SIGNALS:
         void colorChanged(int const index, QColor const&);

      private Q_SLOTS:
         void getNewColor();

      private:
         QColor m_color;
         int    m_index;
   };

} 


Gradient::ColorList GetGradient(Gradient::ColorList const&, QWidget* parent = 0);
Gradient::ColorList GetGradient(Gradient::Dialog::StandardGradient g = Gradient::Dialog::Default,
   QWidget* parent = 0);

} // end namespace IQmol::Gradient

#endif
