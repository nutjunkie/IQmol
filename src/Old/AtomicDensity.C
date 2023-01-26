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

#include "Constants.h"
#include "AtomicDensity.h"
#include "AtomicDensityData.h"
#include <openbabel/elements.h>
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace AtomicDensity {

Base::Base(unsigned atomicNumber) : m_atomicNumber(atomicNumber), m_charge(0.0) 
{ 
}
             

// Attempts to give an approximate size of the basis function for use in cutoffs
double Base::computeSignificantRadius(double const thresh) const
{
   double r1(0.0), r2(20.0);

   // Note the search is only along the z direction, which 
   // will not be valid for non-spherical densities.
   double z1(density(Vec(0.0, 0.0, r1)) - thresh);
   double z2(density(Vec(0.0, 0.0, r2)) - thresh);

   // Sanity check to make sure we bracket the root, 
   // if not give a large radius to turn off cutoffs
   return (z1 > 0.0 && z2 < 0.0) ? binarySearch(r1, r2, z1, z2, thresh) : r2;
}


double Base::binarySearch(double r1, double r2, double y1, double y2, double thresh) const
{
   if (std::abs(r2-r1) < 0.1) {
      return  r2;  // only a rough radius is required
   }
   double r3 = (r2+r1) / 2.0;
   double y3(density(Vec(0.0, 0.0, r3)) - thresh);
   return (y3 > 0.0) ? binarySearch(r3, r2, y3, y2, thresh) 
                     : binarySearch(r1, r3, y1, y3, thresh);
}


 
// --------------- AtomicShellApproximation ---------------
AtomShellApproximation::AtomShellApproximation(unsigned atomicNumber) : Base(atomicNumber) 
{
   m_nBasis = s_expansionLength[atomicNumber-1]; 
   m_data = s_parameters[m_atomicNumber-1];
}

double AtomShellApproximation::density(Vec const& position) const
{
   double r2( position.squaredNorm()*Constants::AngstromToBohr*Constants::AngstromToBohr );
   double p(0.0);

   for (unsigned i = 0; i < m_nBasis; ++i) {
       p += m_data[2*i] * std::exp(-m_data[2*i+1]*r2); 
   }

   return p;
}


// --------------- Superposition Ionic Densities ---------------
double const SuperpositionIonicDensities::s_stepSize = 0.100;

SuperpositionIonicDensities::SuperpositionIonicDensities(unsigned atomicNumber)
  : Base(atomicNumber)
{
   // These are deterimined by the order in AtomicDensityData.h
   unsigned anion(3*(atomicNumber-1));
   unsigned neutral(anion+1);
   unsigned cation(neutral+1);

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
   double r(position.norm()*Constants::AngstromToBohr);
   double f1(0.0), f2(0.0), d1(0.0), d2(0.0);
   double q(std::abs(m_charge));
   unsigned index (r/s_stepSize);

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

double VanDerWaals::s_vertices[12][3] = {    
   { -X, 0.0,   Z}, {  X, 0.0,   Z}, { -X, 0.0,  -Z}, {  X, 0.0,  -Z},    
   {0.0,   Z,   X}, {0.0,   Z,  -X}, {0.0,  -Z,   X}, {0.0,  -Z,  -X},    
   {  Z,   X, 0.0}, {  -Z,  X, 0.0}, {  Z,  -X, 0.0}, {  -Z, -X, 0.0} 
};


unsigned VanDerWaals::s_indices[20][3] = { 
   { 0,  4,  1}, { 0, 9,  4}, { 9,  5, 4}, {  4, 5, 8}, { 4, 8,  1},    
   { 8, 10,  1}, { 8, 3, 10}, { 5,  3, 8}, {  5, 2, 3}, { 2, 7,  3},    
   { 7, 10,  3}, { 7, 6, 10}, { 7, 11, 6}, { 11, 0, 6}, { 0, 1,  6}, 
   { 6,  1, 10}, { 9, 0, 11}, { 9, 11, 2}, {  9, 2, 5}, { 7, 2, 11}
};


VanDerWaals::VanDerWaals(unsigned atomicNumber, Vec const& center, double const scale,
   double const solventRadius) : Base(atomicNumber), m_center(center)
{
   m_radius = scale*OpenBabel::OBElements::GetVdwRad(atomicNumber)+solventRadius;
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


Data::Mesh VanDerWaals::generateMesh(int ndiv, QList<VanDerWaals*> const& atoms) 
{
   Data::OMMesh::VertexHandle vertices[12];
   Data::Mesh mesh;

   for (unsigned i = 0; i < 12; ++i) {
       vertices[i] = mesh.addVertex(s_vertices[i][0], s_vertices[i][1], s_vertices[i][2]);
       mesh.setNormal(vertices[i], mesh.vertex(vertices[i]));
   }

   for (unsigned i = 0; i < 20; ++i) {
       generateTriangle(mesh, vertices[s_indices[i][0]], vertices[s_indices[i][1]], 
          vertices[s_indices[i][2]], ndiv);
   }
 
   // Now dilate and translate the unit sphere
   Data::OMMesh::Point vertex;
   Data::OMMesh::Point center(m_center[0], m_center[1], m_center[2]);
   Data::OMMesh::VertexIter iter;
   for (iter = mesh.vbegin(); iter != mesh.vend(); ++iter) {
       vertex = mesh.vertex(iter);
       vertex = vertex*m_radius + center;
       mesh.setPoint(iter.handle(), vertex);
   }

   QList<VanDerWaals*>::const_iterator atom;
   qglviewer::Vec A(m_center), B, BA, p;
   double r1(m_radius), r2, R, d;

   for (atom = atoms.begin(); atom != atoms.end(); ++atom) {
       if (*atom != this) {
          r2 = (*atom)->m_radius;
          B  = (*atom)->m_center;
          BA = B-A;
          R  = BA.norm();
          if (R < r1+r2) {
             d = (R*R + r1*r1 - r2*r2) / (2.0*R);
             d += 0.001;  // add a bit extra to make sure there are no holes in the surface
             p = A + BA*d/R;
             mesh.clip(-BA, p);
          }
       }
   }

   return mesh;
}



void VanDerWaals::generateTriangle(Data::Mesh& mesh, Data::OMMesh::VertexHandle const& Av,
   Data::OMMesh::VertexHandle const& Bv, Data::OMMesh::VertexHandle const& Cv, int div)
{
   if (div <= 0) {
      mesh.addFace(Cv, Bv, Av);

   }else {

      Data::OMMesh::Point Ap(mesh.vertex(Av));
      Data::OMMesh::Point Bp(mesh.vertex(Bv));
      Data::OMMesh::Point Cp(mesh.vertex(Cv));

      // create 3 new vertices at the edge midpoints
      Data::OMMesh::Point ABp((Ap+Bp)*0.5);
      Data::OMMesh::Point BCp((Bp+Cp)*0.5);
      Data::OMMesh::Point CAp((Cp+Ap)*0.5);

      // Normalize the midpoints to keep them on the sphere
      ABp.normalize();
      BCp.normalize();
      CAp.normalize();

      Data::OMMesh::VertexHandle ABv(mesh.addVertex(ABp));
      Data::OMMesh::VertexHandle BCv(mesh.addVertex(BCp));
      Data::OMMesh::VertexHandle CAv(mesh.addVertex(CAp));

      mesh.setNormal(ABv, ABp);
      mesh.setNormal(BCv, BCp);
      mesh.setNormal(CAv, CAp);

      generateTriangle(mesh, Av,  ABv, CAv, div-1);
      generateTriangle(mesh, Bv,  BCv, ABv, div-1);
      generateTriangle(mesh, Cv,  CAv, BCv, div-1);
      generateTriangle(mesh, ABv, BCv, CAv, div-1);  //<-- Remove for serpinski
   }  
}

} } // end namespace IQmol::AtomicDensity
