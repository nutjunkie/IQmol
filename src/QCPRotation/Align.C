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

#include "Align.h"
#include "qcprot.h"
#include <QDebug>


using namespace qglviewer;

namespace IQmol {

Align::Align(QList<Vec> const& reference, QList<Vec> const& current, 
   QList<double> const& weights) : m_aligned(false)
{
   m_nAtoms    = reference.size();
   m_weights   = new double[m_nAtoms];
   m_reference = initMatrix(3, m_nAtoms);
   m_current   = initMatrix(3, m_nAtoms);

   bool noWeights(weights.isEmpty());

   for (int i = 0; i < m_nAtoms; ++i) {
       m_reference[0][i] = reference[i].x;
       m_reference[1][i] = reference[i].y;
       m_reference[2][i] = reference[i].z;

       m_current[0][i] = current[i].x;
       m_current[1][i] = current[i].y;
       m_current[2][i] = current[i].z;

       m_weights[i] = noWeights ? 1.0 : weights[i];
   }
}


Align::~Align()
{
   delete [] m_weights;
   destroyMatrix(&m_reference);
   destroyMatrix(&m_current);
}


Quaternion& Align::rotation()
{
   if (!m_aligned) computeAlignment();
   return m_rotation;
}


Vec& Align::translation()
{
   if (!m_aligned) computeAlignment();
   return m_translation;
}


bool Align::computeAlignment()
{
   Vec comA = centerCoords(m_reference);
   Vec comB = centerCoords(m_current);
   m_translation = comB;

   double A[9], rmsd;
   double rotmat[9];
   double E0 = InnerProduct(A, m_reference, m_current, m_nAtoms, m_weights);
   FastCalcRMSDAndRotation(rotmat, A, &rmsd, E0, m_nAtoms, -1);

   double m[3][3];
   for (int i = 0; i < 3; ++i) {
       for (int j = 0; j < 3; ++j) {
           m[i][j] = rotmat[i+3*j];
       }
   }

   m_rotation.setFromRotationMatrix(m);

/*
      printf("\nCoords after centering:\n");
      printCoords(m_reference);
      printCoords(m_current);

      printf("QCP rmsd: %f\n", rmsd);
      printf("QCP Rotation matrix:\n");
      print3x3Matrix(rotmat);

      double  x, y, z;
      // apply rotation matrix
      for (int i = 0; i < m_nAtoms; ++i) {
          x = rotmat[0]*m_current[0][i] + rotmat[1]*m_current[1][i] + rotmat[2]*m_current[2][i];
          y = rotmat[3]*m_current[0][i] + rotmat[4]*m_current[1][i] + rotmat[5]*m_current[2][i];
          z = rotmat[6]*m_current[0][i] + rotmat[7]*m_current[1][i] + rotmat[8]*m_current[2][i];
       
          m_current[0][i] = x;
          m_current[1][i] = y;
          m_current[2][i] = z;
      }

      // calculate euclidean distance
      double  euc_dist(0.0);
      for (int i = 0; i < m_nAtoms; ++i) {
          euc_dist += pow(m_reference[0][i]-m_current[0][i],2) + 
                      pow(m_reference[1][i]-m_current[1][i],2) + 
                      pow(m_reference[2][i]-m_current[2][i],2);
      }

      printf("\nCoords 2 after rotation:\n");
      printCoords(m_current);
      printf("\nExplicit RMSD calculated from transformed coords: %f\n\n", 
         sqrt(euc_dist/m_nAtoms));
   }
*/

   m_aligned = true;
   return m_aligned;
}


double** Align::initMatrix(const int rows, const int cols)
{
   int i;
   double** matrix(0);
   double*  matspace(0);

   matspace = (double *) calloc((rows * cols), sizeof(double));

   if (matspace == NULL) {
      perror("\n ERROR");
      printf("\n ERROR: Failure to allocate matrix space in MatInit(): (%d x %d)\n",
         rows, cols);
       exit(EXIT_FAILURE);
    }

    /* allocate room for the pointers to the rows */
    matrix = (double **) malloc(rows * sizeof(double *));
    if (matrix == NULL) {
        perror("\n ERROR");
        printf("\n ERROR: Failure to allocate room for row pointers in MatInit(): (%d)\n", 
           rows);
        exit(EXIT_FAILURE);
    }

    /*  now 'point' the pointers */
    for (i = 0; i < rows; i++) {
        matrix[i] = matspace + (i * cols);
    }

    return(matrix);
}


void Align::destroyMatrix(double*** matrix_ptr)
{
   double **matrix = *matrix_ptr;

   if (matrix != 0) {
      if (matrix[0] != NULL) {
         free(matrix[0]);
         matrix[0] = 0;
      }
      free(matrix);
      *matrix_ptr = 0;
   }
}


void Align::print3x3Matrix(double *matrix)
{
   for (int i = 0; i < 3; ++i){
       printf(" [ % 14.8f % 14.8f % 14.8f ]\n",
              matrix[3 * i],
              matrix[3 * i + 1],
              matrix[3 * i + 2]);
   }
   fflush(0);
}


void Align::printCoords(double **coords)
{
   for (int i = 0; i < m_nAtoms; ++i) {
       printf("\n % 8.3f % 8.3f % 8.3f", coords[0][i], coords[1][i], coords[2][i]);
   }
   putchar('\n');
}



Vec Align::centerCoords(double **coords)
{
    Vec offset;
    double mass(0.0);
    double* x(coords[0]);
    double* y(coords[1]);
    double* z(coords[2]);

    for (int i = 0; i < m_nAtoms; ++i) {
        offset.x += m_weights[i] * x[i];
        offset.y += m_weights[i] * y[i];
        offset.z += m_weights[i] * z[i];
        mass += m_weights[i];
    }

    offset /= mass;

    for (int i = 0; i < m_nAtoms; ++i) {
        x[i] -= offset.x;
        y[i] -= offset.y;
        z[i] -= offset.z;
    }

    return offset;
}


} // end namespace IQmol
