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

#include "ColorGradient.h"
#include <cmath>


namespace IQmol {
namespace ColorGradient {

QString ToString(ColorList const& colors)
{
   QString grad;
   int nColors(colors.size());

   switch (nColors) {
      case 0:
         grad = "#000";  // black
         break;
      case 1:
         grad = colors.first().name();
         break;
      default:
         grad = "QLinearGradient(x1: 0, y1: 1, x2: 1, y2: 1, ";
         double step(1.0/(nColors-1));
         for (int i = 0; i < nColors-1; ++i) {
             grad += "stop: " + QString::number(i*step) + " " + colors[i].name() + ", ";
         }
         grad += "stop: 1.00  " + colors.last().name() + ");";
         break;
   }

   return grad;
}


// --------------- Function ---------------

Function::Function(QList<QColor> const& colors, double const min, double const max) 
  : m_colors(colors), m_min(min), m_max(max)
{
   if (m_colors.isEmpty()) m_colors << Qt::black;
}


Function& Function::operator=(Function const& that)
{
   if (this != &that) copy(that);
   return *this;
}


void Function::copy(Function const& that) 
{
   m_colors = that.m_colors; 
   m_min = that.m_min;
   m_max = that.m_max;
}


QColor Function::colorAt(double val) const
{
   // Deal with out of bounds first
   if (m_colors.size() == 1) return m_colors.first();
   if (val <= m_min ) return m_colors.first();
   if (val >= m_max ) return m_colors.last();

   val = (val-m_min) / (m_max-m_min);
   int index((m_colors.size()-1)*val);
   double step(1.0/(m_colors.size()-1));
   double delta((val-index*step)/step);

   return interpolateRGB(delta, m_colors[index], m_colors[index+1]);
}


// Performs a linear interpolation between two colors, delta \in [0.0,1.0]
QColor Function::interpolateRGB(double delta, QColor const& col1, QColor const& col2) const
{
   if (delta <= 0.0) return col1;
   if (delta >= 1.0) return col2;

   qreal r1, r2, g1, g2, b1, b2;
   col1.getRgbF(&r1, &g1, &b1);
   col2.getRgbF(&r2, &g2, &b2);

   r1 += delta*(r2-r1);
   g1 += delta*(g2-g1);
   b1 += delta*(b2-b1);

   QColor color;
   color.setRgbF(r1, g1, b1);
   return color;
}


QColor Function::interpolateHSV(double delta, QColor const& col1, QColor const& col2) const
{
   if (delta <= 0.0) return col1;
   if (delta >= 1.0) return col2;

   qreal h1, h2, s1, s2, v1, v2;
   col1.getHsvF(&h1, &s1, &v1);
   col2.getHsvF(&h2, &s2, &v2);

   if (h1 < 0.0) h1 = h2;
   if (h2 < 0.0) h2 = h1;
   if (h1 < 0.0 && h2 < 0.0) { h1 = 0.0; h2 = 0.0; }

   s1 += delta*(s2-s1);
   v1 += delta*(v2-v1);

   // For hue we need to work out which direction we are going on the wheel.
   double dh(h2-h1);
   if (dh > 0.5) {
      dh -= 1.0;
   }else if(dh < -0.5) {
      dh += 1.0;
   }

   h1 += delta*dh;
   if (h1 > 1.0) h1 -= 1.0;
   if (h1 < 0.0) h1 += 1.0;

   QColor color;
   color.setHsvF(h1, s1, v1);
   return color;
}


void Function::resample(int nColors)
{
   if (nColors == m_colors.size() || nColors < 1) return;

   ColorList newColors;
   if (nColors == 1) {
      newColors.append(colorAt(0.5));
   }else {
      double step(1.0/(nColors-1));
      for (int i = 0; i < nColors; ++i) {
         newColors.append(colorAt(i*step));
      }
   }
   m_colors = newColors;
}

} } // end namespace IQmol::ColorGradient
