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
#include "Sanderson.h"
#include "Eigen/Dense"

#include <QtDebug>
#include <iostream>


using namespace Eigen;

namespace IQmol {

const unsigned int s_maxAtomicNumber = 18;

const double Sanderson::s_energies[][3] = {
//          anion          neutral           cation
   {   -0.5000811987,   -0.4998179156,    0.0000000000 },   // H
   {   -2.7168574497,   -2.8729830896,   -1.9982215126 },   // He
   {   -7.4547324742,   -7.4449250306,   -7.2483476302 },   // Li
   {  -14.5836039380,  -14.6103767743,  -14.2906680250 },   // Be
   {  -24.5637591296,  -24.5685314319,  -24.2829851531 },   // B
   {  -37.7643005262,  -37.7303848283,  -37.3336150028 },   // C
   {  -54.4118608434,  -54.4475943945,  -53.9269625161 },   // N
   {  -74.9159266420,  -74.8810225183,  -74.4105175484 },   // O
   {  -99.6263442144,  -99.5080767789,  -98.8883194293 },   // F
   { -128.4060927016, -128.6785669612, -127.8954172521 },   // Ne
   { -161.9858131241, -161.9767087173, -161.7938120519 },   // Na
   { -199.7328023391, -199.7530930299, -199.4908738485 },   // Mg
   { -242.0123759252, -242.0115775029, -241.8141103406 },   // Al
   { -289.0228443534, -288.9855594148, -288.7073830677 },   // Si
   { -340.8419438949, -340.8439429294, -340.4766925942 },   // P
   { -397.6874153756, -397.6357755911, -397.2922581627 },   // S
   { -459.6656439217, -459.5499412033, -459.1034823004 },   // Cl
   { -526.7693482845, -526.9021041158, -526.3443438855 }};  // Ar


QList<double> Sanderson::solve()
{
   int nAtoms(m_atomicNumbers.size()), Z;

   MatrixXd M(nAtoms+1, nAtoms+1);
   VectorXd v(nAtoms+1);

double Eh2eV(27.211399);
   for (int i = 0; i < nAtoms; ++i) {
       Z = m_atomicNumbers[i];
       v(i)   = -electronegativity(Z);
       M(i,i) = 2*hardness(Z);
qDebug() << "Z =" << Z << "chi =" << electronegativity(Z)*Eh2eV << "eta =" << hardness(Z)*Eh2eV;

       M(i,nAtoms) =  1.0;
       M(nAtoms,i) = -1.0;

       for (int j = i+1; j < nAtoms; ++j) {
           M(i,j) = BohrToAngstrom/(m_coordinates[i]-m_coordinates[j]).norm();
           M(j,i) = M(i,j);
       }
   }

   M(nAtoms, nAtoms) = 0;
   v(nAtoms) = m_molecularCharge;

   //std::cout << "Here is the matrix M:\n" << M << std::endl;
   std::cout << "Here is the vector v:\n" << v << std::endl;
   MatrixXd q = M.fullPivLu().solve(v);
   std::cout << "The solution is:\n" << q << std::endl;

   QList<double> charges;
   for (int i = 0; i < nAtoms; ++i) {
       charges.append(q(i));
   }

   return charges;
}


double Sanderson::electronegativity(int const Z) const
{
   double chi;

   switch (Z) {
      // Effective values taken from JPCA 106, 7889, 2002
      case 1:  chi = 0.036749;  break;
      case 6:  chi = 0.192934;  break;
      case 7:  chi = 0.323394;  break;
      case 8:  chi = 0.540950;  break;
      case 9:  chi = 0.551240;  break;

      default:
         chi = 0.5*(s_energies[Z-1][2] - s_energies[Z-1][0]);
   }

         //chi = 0.5*(s_energies[Z-1][2] - s_energies[Z-1][0]);

   if (0) {
      double Eh2eV(27.211399);
      switch (Z) {
         // Experimental values taken from JACS 105, 7512, 1983
         case 1:  chi = 7.17 / Eh2eV;  break;
         case 6:  chi = 6.27 / Eh2eV;  break;
         case 8:  chi = 7.53 / Eh2eV;  break;
         default: break;
      }
   }

   return chi;
}


double Sanderson::hardness(int const Z) const
{
   double eta;

   switch (Z) {
      // Effective values taken from JPCA 106, 7889, 2002
      case 1:  eta = 0.659650;  break;
      case 6:  eta = 0.330744;  break;
      case 7:  eta = 0.345076;  break;
      case 8:  eta = 0.526985;  break;
      case 9:  eta = 0.726534;  break;

      default:
         eta = 0.5*(s_energies[Z-1][0] - s_energies[Z-1][2]) - s_energies[Z-1][1];
   }

         //eta = 0.5*(s_energies[Z-1][0] - s_energies[Z-1][2]) - s_energies[Z-1][1];

   if (0) {
      double Eh2eV(27.211399);
      switch (Z) {
         // Experimental values taken from JACS 105, 7512, 1983
         case 1:  eta = 6.42 / Eh2eV;  break;
         case 6:  eta = 5.00 / Eh2eV;  break;
         case 8:  eta = 6.08 / Eh2eV;  break;
         default: break;
      }
   }


   return eta;
}

} // end namespace IQmol
