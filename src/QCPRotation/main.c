/*******************************************************************************
 *  -/_|:|_|_\- 
 *
 *  File:           main.c
 *
 *  Function:       Rapid calculation of the least-squares rotation using a 
 *                  quaternion-based characteristic polynomial and 
 *                  a cofactor matrix
 *
 *  Author(s):      Douglas L. Theobald
 *                  Department of Biochemistry
 *                  MS 009
 *                  Brandeis University
 *                  415 South St
 *                  Waltham, MA  02453
 *                  USA
 *
 *                  dtheobald@brandeis.edu
 *                  
 *                  Pu Liu
 *                  Johnson & Johnson Pharmaceutical Research and Development, L.L.C.
 *                  665 Stockton Drive
 *                  Exton, PA  19341
 *                  USA
 *
 *                  pliu24@its.jnj.com
 * 
 *
 *    If you use this QCP rotation calculation method in a publication, please
 *    reference:
 *
 *      Douglas L. Theobald (2005)
 *      "Rapid calculation of RMSD using a quaternion-based characteristic
 *      polynomial."
 *      Acta Crystallographica A 61(4):478-480.
 *
 *      Pu Liu, Dmitris K. Agrafiotis, and Douglas L. Theobald (2009)
 *      "Fast determination of the optimal rotational matrix for weighted 
 *      superpositions."
 *      in press, Journal of Computational Chemistry
 *
 *
 *  Copyright (c) 2009-2012, Pu Liu and Douglas L. Theobald
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted 
 *  provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this list of 
 *    conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice, this list 
 *    of conditions and the following disclaimer in the documentation and/or other materials 
 *    provided with the distribution.
 *  * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to 
 *    endorse or promote products derived from this software without specific prior written 
 *    permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 *  Source:         started anew.
 *
 *  Change History:
 *    2009/04/13      Started source
 *    2012/07/26      Added explicit check on RMSD after rotation, more info on matrices, etc.
 *  
 ******************************************************************************/

 /* Sample code to use the routine for fast RMSD & rotational matrix calculation.
    Note that we superposition frag_b onto frag_a.  
    For the example provided below, the minimum least-squares RMSD for the two
    7-atom fragments should be 0.719106 A.

    The rotation quaterion should be:

    -8.620063e-01   3.435505e-01   1.242953e-01  -3.513814e-01 

    And the corresponding 3x3 rotation matrix is:

    [     0.72216358     0.69118937    -0.02714790 ]
    [    -0.52038257     0.51700833    -0.67963547 ]
    [    -0.45572112     0.50493528     0.73304748 ]

    Compile instruction:

     make

     How to incorporate the code into your own package.

     1. copy the qcprot.h and qcprot.c into the source directory
     2. change your own code to call the fast rotational routine and include the qcprot.h
     3. change your make file to include qcprot.c
*/

#include "qcprot.h"

double **MatInit(const int rows, const int cols);

void MatDestroy(double ***matrix);

static void Mat3Print(double *matrix);

void PrintCoords(const double **coords, const int len);


int main()
{  
    double          rmsd, x, y, z, euc_dist;
    double        **frag_a, **frag_b;
    int             len = 7;
    double          rotmat[9];

    frag_a = MatInit(3, len);
    frag_b = MatInit(3, len);

    frag_a[0][0] =  -2.803;
    frag_a[1][0] = -15.373;
    frag_a[2][0] =  24.556;
    frag_a[0][1] =   0.893;
    frag_a[1][1] = -16.062;
    frag_a[2][1] =  25.147;
    frag_a[0][2] =   1.368;
    frag_a[1][2] = -12.371;
    frag_a[2][2] =  25.885;
    frag_a[0][3] =  -1.651;
    frag_a[1][3] = -12.153;
    frag_a[2][3] =  28.177;
    frag_a[0][4] =  -0.440;
    frag_a[1][4] = -15.218;
    frag_a[2][4] =  30.068;
    frag_a[0][5] =   2.551;
    frag_a[1][5] = -13.273;
    frag_a[2][5] =  31.372;
    frag_a[0][6] =   0.105;
    frag_a[1][6] = -11.330;
    frag_a[2][6] =  33.567;

    frag_b[0][0] = -14.739;
    frag_b[1][0] = -18.673;
    frag_b[2][0] =  15.040;
    frag_b[0][1] = -12.473;
    frag_b[1][1] = -15.810;
    frag_b[2][1] =  16.074;
    frag_b[0][2] = -14.802;
    frag_b[1][2] = -13.307;
    frag_b[2][2] =  14.408;
    frag_b[0][3] = -17.782;
    frag_b[1][3] = -14.852;
    frag_b[2][3] =  16.171;
    frag_b[0][4] = -16.124;
    frag_b[1][4] = -14.617;
    frag_b[2][4] =  19.584;
    frag_b[0][5] = -15.029;
    frag_b[1][5] = -11.037;
    frag_b[2][5] =  18.902;
    frag_b[0][6] = -18.577;
    frag_b[1][6] = -10.001;
    frag_b[2][6] =  17.996;

    printf("\nCoords before centering:\n");

    PrintCoords((const double **) frag_a, len);
    PrintCoords((const double **) frag_b, len);

    rmsd = CalcRMSDRotationalMatrix((double **) frag_a, (double **) frag_b, len, rotmat, NULL);

    printf("\nCoords after centering:\n");

    PrintCoords((const double **) frag_a, len);
    PrintCoords((const double **) frag_b, len);

    printf("\nQCP rmsd: %f\n", rmsd);

    printf("\nQCP Rotation matrix:\n");
    Mat3Print(rotmat);

    /* apply rotation matrix */
    for (int i = 0; i < len; ++i) 
    {
        x = rotmat[0]*frag_b[0][i] + rotmat[1]*frag_b[1][i] + rotmat[2]*frag_b[2][i];
        y = rotmat[3]*frag_b[0][i] + rotmat[4]*frag_b[1][i] + rotmat[5]*frag_b[2][i];
        z = rotmat[6]*frag_b[0][i] + rotmat[7]*frag_b[1][i] + rotmat[8]*frag_b[2][i];
        
        frag_b[0][i] = x;
        frag_b[1][i] = y;
        frag_b[2][i] = z;
    }

    /* calculate euclidean distance */
    euc_dist = 0.0;
    for (int i = 0; i < len; ++i)
    {
        euc_dist += pow(frag_a[0][i]-frag_b[0][i],2) + pow(frag_a[1][i]-frag_b[1][i],2) + pow(frag_a[2][i]-frag_b[2][i],2);
    }

    printf("\nCoords 2 after rotation:\n");
    PrintCoords((const double **) frag_b, len);

    printf("\nExplicit RMSD calculated from transformed coords: %f\n\n", sqrt(euc_dist / len));

    MatDestroy(&frag_a);
    MatDestroy(&frag_b);

    exit(EXIT_SUCCESS);
}


double **MatInit(const int rows, const int cols)
{
    int             i;
    double        **matrix = NULL;
    double         *matspace = NULL;

    matspace = (double *) calloc((rows * cols), sizeof(double));
    if (matspace == NULL)
    {
        perror("\n ERROR");
        printf("\n ERROR: Failure to allocate matrix space in MatInit(): (%d x %d)\n", rows, cols);
        exit(EXIT_FAILURE);
    }

    /* allocate room for the pointers to the rows */
    matrix = (double **) malloc(rows * sizeof(double *));
    if (matrix == NULL)
    {
        perror("\n ERROR");
        printf("\n ERROR: Failure to allocate room for row pointers in MatInit(): (%d)\n", rows);
        exit(EXIT_FAILURE);
    }

    /*  now 'point' the pointers */
    for (i = 0; i < rows; i++)
        matrix[i] = matspace + (i * cols);

    return(matrix);
}


void MatDestroy(double ***matrix_ptr)
{
    double **matrix = *matrix_ptr;

    if (matrix != NULL)
    {
        if (matrix[0] != NULL)
        {
            free(matrix[0]);
            matrix[0] = NULL;
        }

        free(matrix);
        *matrix_ptr = NULL;
    }
}


static void Mat3Print(double *matrix)
{
    int             i;

    printf("\n");
    for (i = 0; i < 3; ++i)
    {
        printf(" [ % 14.8f % 14.8f % 14.8f ]\n",
               matrix[3 * i],
               matrix[3 * i + 1],
               matrix[3 * i + 2]);
    }

    fflush(NULL);
}


void PrintCoords(const double **coords, const int len)
{
    int             i;

    for (i = 0; i < len; ++i)
        printf("\n % 8.3f % 8.3f % 8.3f", coords[0][i], coords[1][i], coords[2][i]);
    putchar('\n');
}


