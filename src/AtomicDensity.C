/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "IQmol.h"
#include "AtomicDensity.h"
#include "AtomicDensityData.h"
#include <openbabel/mol.h>  // for OpenBabel::etab
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace AtomicDensity {


// Attempts to give an approximate size of the basis function for use in cutoffs
double Base::computeSignificantRadius(double const thresh) const
{
   double r1(0.0), r2(20.0);

   // Note the search is only along the z direction, which 
   // will not be valid for non-spherical densities.
   double y1(density(Vec(0.0, 0.0, r1)) - thresh);
   double y2(density(Vec(0.0, 0.0, r2)) - thresh);

   // Sanity check to make sure we bracket the root, 
   // if not give a large radius to turn off cutoffs
   return (y1 > 0.0 && y2 < 0.0) ? binarySearch(r1, r2, y1, y2, thresh) : r2;
}


double Base::binarySearch(double r1, double r2, double y1, double y2, double thresh) const
{
   if (std::abs(r2-r1) < 0.1) return r2;  // only a rough radius is required
   double r3 = (r2+r1) / 2.0;
   double y3(density(Vec(0.0, 0.0, r3)) - thresh);
   return (y3 > 0.0) ? binarySearch(r3, r2, y3, y2, thresh) : binarySearch(r1, r3, y1, y3, thresh);
}


 
// --------------- AtomicShellApproximation ---------------
AtomShellApproximation::AtomShellApproximation(unsigned int atomicNumber) : Base(atomicNumber) 
{
   m_nBasis = s_expansionLength[atomicNumber-1]; 
   m_data = s_parameters[m_atomicNumber-1];
}

double AtomShellApproximation::density(Vec const& position) const
{
   double r2( position.squaredNorm()*AngstromToBohr*AngstromToBohr );
   double p(0.0);

   for (unsigned int i = 0; i < m_nBasis; ++i) {
       p += m_data[2*i] * std::exp(-m_data[2*i+1]*r2); 
   }

   return p;
}


// --------------- Superposition Ionic Densities ---------------
double const SuperpositionIonicDensities::s_stepSize = 0.100;

SuperpositionIonicDensities::SuperpositionIonicDensities(unsigned int atomicNumber,
   double const charge) : Base(atomicNumber), m_charge(charge)
{
   // These are deterimined by the order in AtomicDensityData.h
   unsigned int anion(3*(atomicNumber-1));
   unsigned int neutral(anion+1);
   unsigned int cation(neutral+1);

   m_neutralData  = s_parameters[neutral];
   m_nNeutralData = s_expansionLength[neutral]/2;

   if (m_charge < 0.0) {
      m_chargedData  = s_parameters[anion];
      m_nChargedData = s_expansionLength[anion]/2;
   }else {
      m_chargedData  = s_parameters[cation];
      m_nChargedData = s_expansionLength[cation]/2;
   }
}

double SuperpositionIonicDensities::density(Vec const& position) const
{
   double r(position.norm()*AngstromToBohr);
   double f1(0.0), f2(0.0), d1(0.0), d2(0.0);
   double q(std::abs(m_charge));
   unsigned int index (r/s_stepSize);

   if (index < m_nNeutralData-1) {
      f1 = (1.0-q) * m_neutralData[2*index];
      d1 = (1.0-q) * m_neutralData[2*index+1];
      f2 = (1.0-q) * m_neutralData[2*index+2];
      d2 = (1.0-q) * m_neutralData[2*index+3];
   }

   if (index < m_nChargedData-1) {
      f1 += q * m_chargedData[2*index];
      d1 += q * m_chargedData[2*index+1];
      f2 += q * m_chargedData[2*index+2];
      d2 += q * m_chargedData[2*index+3];
   }

   r = (r-index*s_stepSize) / s_stepSize;
   return (1.0-r)*f1 + r*f2;

   // cubic interpolation gets messed up with the sudden changes at the origin
   if (index < 2) return (1.0-r)*f1 + r*f2;

   // cubic spline interpolation
   double a( d1*s_stepSize - (f2-f1));
   double b(-d2*s_stepSize + (f2-f1));
   return (1.0-r)*f1 + r*f2 + r*(1.0-r) * (a*(1.0-r)+b*r);
}





// --------------- VanDerWaals ---------------
#define X .525731112119133606 
#define Z .850650808352039932

GLfloat VanDerWaals::s_vertices[12][3] = {    
   { -X, 0.0,   Z}, {  X, 0.0,   Z}, { -X, 0.0,  -Z}, {  X, 0.0,  -Z},    
   {0.0,   Z,   X}, {0.0,   Z,  -X}, {0.0,  -Z,   X}, {0.0,  -Z,  -X},    
   {  Z,   X, 0.0}, {  -Z,  X, 0.0}, {  Z,  -X, 0.0}, {  -Z, -X, 0.0} 
};


unsigned int VanDerWaals::s_indices[20][3] = { 
   { 0,  4,  1}, { 0, 9,  4}, { 9,  5, 4}, {  4, 5, 8}, { 4, 8,  1},    
   { 8, 10,  1}, { 8, 3, 10}, { 5,  3, 8}, {  5, 2, 3}, { 2, 7,  3},    
   { 7, 10,  3}, { 7, 6, 10}, { 7, 11, 6}, { 11, 0, 6}, { 0, 1,  6}, 
   { 6,  1, 10}, { 9, 0, 11}, { 9, 11, 2}, {  9, 2, 5}, { 7, 2, 11}
};


VanDerWaals::VanDerWaals(unsigned int atomicNumber, Vec const& center, double const scale,
   double const solventRadius) : Base(atomicNumber)
{
   m_center[0] = center.x;
   m_center[1] = center.y;
   m_center[2] = center.z;
   m_radius = scale*OpenBabel::etab.GetVdwRad(atomicNumber)+solventRadius;
}


double VanDerWaals::computeSignificantRadius(double const) const
{
   return m_radius + 0.5;  // a bit arbitrary
}


double VanDerWaals::density(Vec const& position) const
{
   double r(position.norm());
   return r > m_radius ? 0.0 : 1.0;
}



// Creates an icosasphere around the atom with the given number of subdivisions.  If
// the atom list is not empty, the triangles are filtered so that any that are closer
// to another center are removed.
Layer::Surface::Data VanDerWaals::surfaceData(int ndiv, QList<VanDerWaals*> const& atoms) 
{
   Layer::Surface::Data data;
   for (int i = 0; i < 20; ++i) {
       data += drawTriangle(s_vertices[s_indices[i][0]], 
                            s_vertices[s_indices[i][1]], 
                            s_vertices[s_indices[i][2]], ndiv);
   }

   if (!atoms.isEmpty()) {
      m_filterLevel = 0;
      data = filterSurfaceData(data, atoms);
   }
   return data;
}


Layer::Surface::Data VanDerWaals::drawTriangle(GLfloat const* a, GLfloat const* b, 
   GLfloat const* c, int div, bool abEdge, bool bcEdge, bool caEdge) 
{
   Layer::Surface::Data data;
   if (div <= 0) {
      GLfloat da[3], db[3], dc[3];
      for (int i = 0; i < 3; ++i) {
          da[i] = m_center[i] - a[i]*m_radius;
          db[i] = m_center[i] - b[i]*m_radius;
          dc[i] = m_center[i] - c[i]*m_radius;
      }

      data << -a[0] << -a[1] << -a[2];  // normal
      data << da[0] << da[1] << da[2];  // position
      data << -b[0] << -b[1] << -b[2];
      data << db[0] << db[1] << db[2]; 
      data << -c[0] << -c[1] << -c[2];
      data << dc[0] << dc[1] << dc[2]; 

   }else {
      GLfloat ab[3], ac[3], bc[3];
      for (int i = 0; i < 3; ++i) {
          ab[i] = (a[i]+b[i]) / 2.0;
          ac[i] = (a[i]+c[i]) / 2.0;
          bc[i] = (b[i]+c[i]) / 2.0;
      }

      normalize(ab); 
      normalize(ac); 
      normalize(bc);

      // Extra triangles required for non-uniform coverage
      if (abEdge) data += drawTriangle( a, b, ab, div-1);
      if (bcEdge) data += drawTriangle( b, c, bc, div-1);
      if (caEdge) data += drawTriangle( a, c, ac, div-1);

      data += drawTriangle( a, ab, ac, div-1);
      data += drawTriangle( b, bc, ab, div-1);
      data += drawTriangle( c, ac, bc, div-1);
      data += drawTriangle(ab, bc, ac, div-1);  //<--Comment for serpinski
   }  

   return data;
}


void VanDerWaals::normalize(GLfloat *a) {
   GLfloat d(std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]));
   a[0] /= d;   a[1] /= d;   a[2] /= d;
}


Layer::Surface::Data VanDerWaals::filterSurfaceData(Layer::Surface::Data const& surfaceData,
   QList<VanDerWaals*> const& atoms)
{
   static int maxFilterLevel = 3;

   if (m_filterLevel > maxFilterLevel) return surfaceData;
   ++m_filterLevel;
   Layer::Surface::Data filteredData;
   const GLfloat *a, *b, *c, *p;
   GLfloat x, y, z, r;
   bool allInside;
   int aCenter, bCenter, cCenter;

   // for each triangle we have 3 vertices and 3 normals => 18
   for (int i = 0; i < surfaceData.size(); i += 18) {
       a = &surfaceData[i+3];    aCenter = -1;
       b = &surfaceData[i+9];    bCenter = -1;
       c = &surfaceData[i+15];   cCenter = -1;

       allInside = false;

       for (int j = 0; (j < atoms.size()) && !allInside; ++j) {
           if (atoms[j] != this) {
              p = &(atoms[j]->m_center[0]);
              r = atoms[j]->m_radius - 0.00001;

              x = a[0] - p[0];  y = a[1] - p[1];  z = a[2] - p[2];
              if (x*x + y*y + z*z < r*r) aCenter = j;

              x = b[0] - p[0];  y = b[1] - p[1];  z = b[2] - p[2];
              if (x*x + y*y + z*z < r*r) bCenter = j;

              x = c[0] - p[0];  y = c[1] - p[1];  z = c[2] - p[2];
              if (x*x + y*y + z*z < r*r) cCenter = j;

              allInside = (aCenter >= 0) && (bCenter >= 0) && (cCenter >= 0);
          }
       }


       if ((aCenter == -1) && (bCenter == -1) && (cCenter == -1)) {
           // no vertices are closer to another center so we keep the triangle as is.
          for (int k = 0; k < 18; ++k) {
              filteredData.append(surfaceData[i+k]);
          }

       }else if ( (aCenter == bCenter) && (bCenter == cCenter) ) {
           // triangle is completely within one atomic sphere, do nothing

       }else if ( (aCenter == -1) && (bCenter == -1) ) {
           // one vertex insdide another => chop it off
           filteredData << chopTriangle(a, b, c, atoms[cCenter], true);

       }else if ( (bCenter == -1) && (cCenter == -1) ) {
           // one vertex insdide another => chop it off
           filteredData << chopTriangle(b, c, a, atoms[aCenter], true);

       }else if ( (cCenter == -1) && (aCenter == -1) ) {
           // one vertex insdide another => chop it off
           filteredData << chopTriangle(c, a, b, atoms[bCenter], true);

       }else if ( (aCenter == -1) && (bCenter == cCenter) ) {
           // two vertices inside another
           filteredData << chopTriangle(b, c, a, atoms[bCenter], false);

       }else if ( (bCenter == -1) && (aCenter == cCenter) ) {
           // two vertices inside another
           filteredData << chopTriangle(c, a, b, atoms[cCenter], false);

       }else if ( (cCenter == -1) && (aCenter == bCenter) ) {
           // two vertices inside another
           filteredData << chopTriangle(a, b, c, atoms[aCenter], false);

/*
       }else if ( (aCenter == -1) && (bCenter != cCenter) ) {
           // two vertices inside two others
           filteredData << chopTriangle(b, c, a, atoms[bCenter], false);

       }else if ( (bCenter == -1) && (aCenter != cCenter) ) {
           // two vertices inside atwo others
           filteredData << chopTriangle(c, a, b, atoms[cCenter], false);

       }else if ( (cCenter == -1) && (aCenter != bCenter) ) {
           // two vertices inside atwo others
           filteredData << chopTriangle(a, b, c, atoms[aCenter], false);
*/

       }else {
           // things get complicated, so we just subdivide 
           // the triangle and have another go
           GLfloat da[3], db[3], dc[3];
           for (int k = 0; k < 3; ++k) {
               da[k] = m_center[k] - a[k];
               db[k] = m_center[k] - b[k];
               dc[k] = m_center[k] - c[k];
           }

           normalize(da); 
           normalize(db); 
           normalize(dc);

           filteredData << filterSurfaceData(drawTriangle(da, db, dc, 1), atoms);
       }
   }

   --m_filterLevel;
   return filteredData;
}


// Vertex c is closer to atom
Layer::Surface::Data VanDerWaals::chopTriangle(GLfloat const* a, GLfloat const* b, 
   GLfloat const* c, VanDerWaals const* atom, bool trimCorner)
{
   Vec P(atom->m_center[0], atom->m_center[1], atom->m_center[2]);
   Vec A(a[0], a[1], a[2]);
   Vec B(b[0], b[1], b[2]);
   Vec C(c[0], c[1], c[2]);

   double ra((A-P).norm());
   double rb((B-P).norm());
   double rc((C-P).norm());
   double r(atom->m_radius);
   double x;

   x = (r-ra) / (rc-ra);
   GLfloat ac[3];
   ac[0] = m_center[0] - (x*C.x + (1.0-x)*A.x);
   ac[1] = m_center[1] - (x*C.y + (1.0-x)*A.y);
   ac[2] = m_center[2] - (x*C.z + (1.0-x)*A.z);

   x = (r-rb) / (rc-rb);
   GLfloat bc[3];
   bc[0] = m_center[0] - (x*C.x + (1.0-x)*B.x);
   bc[1] = m_center[1] - (x*C.y + (1.0-x)*B.y);
   bc[2] = m_center[2] - (x*C.z + (1.0-x)*B.z);

   normalize(ac);
   normalize(bc);

   Layer::Surface::Data data;

   if (trimCorner) {
      GLfloat ab[3];
      ab[0] = m_center[0] - 0.5*(A.x + B.x);
      ab[1] = m_center[1] - 0.5*(A.y + B.y);
      ab[2] = m_center[2] - 0.5*(C.z + B.z);
      normalize(ab);

      GLfloat na[3];
      na[0] = m_center[0] - a[0];
      na[1] = m_center[1] - a[1];
      na[2] = m_center[2] - a[2];
      normalize(na);

      GLfloat nb[3];
      nb[0] = m_center[0] - b[0];
      nb[1] = m_center[1] - b[1];
      nb[2] = m_center[2] - b[2];
      normalize(nb);

      data += drawTriangle(na, ab, ac, 0);
      data += drawTriangle(nb, bc, ab, 0);
      data += drawTriangle(ab, bc, ac, 0);
      data += drawTriangle(na, nb, ab, 0);

   }else {
      GLfloat nc[3];
      nc[0] = m_center[0] - c[0];
      nc[1] = m_center[1] - c[1];
      nc[2] = m_center[2] - c[2];
      normalize(nc);
      data = drawTriangle(ac, bc, nc, 0);
   }

   return data;
}



/*
Layer::Surface::Data VanDerWaals::filterSurfaceData(Layer::Surface::Data const& surfaceData,
   QList<VanDerWaals*> const& atoms)
{
   Layer::Surface::Data newData;

   QList<VanDerWaals*>const_iterator iter;
   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       if ( (*iter) != this) {
       }
   }

   return newData;
}

*/


} } // end namespace IQmol::AtomicDensity
