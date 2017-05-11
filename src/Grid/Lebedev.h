#ifndef IQMOL_GRID_LEBEDEV_H
#define IQMOL_GRID_LEBEDEV_H
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

namespace qglviewer {
   class Vec;
}

namespace IQmol {

   /// Wrapper class around the routines for generating quadrature grids on
   /// the unit sphere.  The routines are designed more for flexibilty rather than 
   /// efficiency.  Original code by Jonh Burkardt and Dmitri Laikov, September 2010.  
   /// Reworked and converted to class structure August 2011.
   /// 
   /// Reference:
   ///    Vyacheslav Lebedev, Dmitri Laikov,
   ///    A quadrature formula for the sphere of the 131st
   ///    algebraic order of accuracy,
   ///    Russian Academy of Sciences Doklady Mathematics,
   ///    Volume 59, Number 3, 1999, pages 477-481.
   class Lebedev {

      public:
         Lebedev(unsigned rule);
         ~Lebedev();

         static bool isAvailable(unsigned const rule) {
            return (rule < s_maxRules)  &&  (s_available[rule] == 1);
         }

         int order() const { return (2*m_rule + 1); }
         int numberOfPoints() const { return m_numberOfPoints; }
         qglviewer::Vec point(unsigned n);
         double weight(unsigned n);
         void dump() const;

      private:
         static unsigned const s_maxRules;
         static int const s_available[];
         static int const s_numberOfPoints[];

         unsigned m_rule;
         unsigned m_numberOfPoints; 
         double *m_x, *m_y, *m_z, *m_w;

         static int generateOh(int const code, double a, double b, double v, 
            double *x, double *y, double *z, double *w);

         typedef void (*Loader)(double*, double*, double*, double*);
         static const Loader s_loaders[];

         static void na( double*, double*, double*, double*) { }
         static void ld0006( double *x, double *y, double *z, double *w );
         static void ld0014( double *x, double *y, double *z, double *w );
         static void ld0026( double *x, double *y, double *z, double *w );
         static void ld0038( double *x, double *y, double *z, double *w );
         static void ld0050( double *x, double *y, double *z, double *w );
         static void ld0074( double *x, double *y, double *z, double *w );
         static void ld0086( double *x, double *y, double *z, double *w );
         static void ld0110( double *x, double *y, double *z, double *w );
         static void ld0146( double *x, double *y, double *z, double *w );
         static void ld0170( double *x, double *y, double *z, double *w );
         static void ld0194( double *x, double *y, double *z, double *w );
         static void ld0230( double *x, double *y, double *z, double *w );
         static void ld0266( double *x, double *y, double *z, double *w );
         static void ld0302( double *x, double *y, double *z, double *w );
         static void ld0350( double *x, double *y, double *z, double *w );
         static void ld0434( double *x, double *y, double *z, double *w );
         static void ld0590( double *x, double *y, double *z, double *w );
         static void ld0770( double *x, double *y, double *z, double *w );
         static void ld0974( double *x, double *y, double *z, double *w );
         static void ld1202( double *x, double *y, double *z, double *w );
         static void ld1454( double *x, double *y, double *z, double *w );
         static void ld1730( double *x, double *y, double *z, double *w );
         static void ld2030( double *x, double *y, double *z, double *w );
         static void ld2354( double *x, double *y, double *z, double *w );
         static void ld2702( double *x, double *y, double *z, double *w );
         static void ld3074( double *x, double *y, double *z, double *w );
         static void ld3470( double *x, double *y, double *z, double *w );
         static void ld3890( double *x, double *y, double *z, double *w );
         static void ld4334( double *x, double *y, double *z, double *w );
         static void ld4802( double *x, double *y, double *z, double *w );
         static void ld5294( double *x, double *y, double *z, double *w );
         static void ld5810( double *x, double *y, double *z, double *w );
   }; 

} // end namespace IQmol


#endif
