#include "transformation.h"

Transformation::Transformation(vector<pair<KeyPoint, KeyPoint>> vec)
{
    //берём 4 соответствия, составляем матрицу А
    gsl_matrix *A = gsl_matrix_alloc(8,9);

    for(int i = 0; i < 4; i++ )
    {
        gsl_matrix_set(A, i*2, 0 ,(vec[i].first).x);
        gsl_matrix_set(A, i*2, 1 ,(vec[i].first).y);
        gsl_matrix_set(A, i*2, 2 ,1);
        gsl_matrix_set(A, i*2, 3 ,0);
        gsl_matrix_set(A, i*2, 4 ,0);
        gsl_matrix_set(A, i*2, 5 ,0);
        gsl_matrix_set(A, i*2, 6 , -(vec[i].second).x * (vec[i].first).x );
        gsl_matrix_set(A, i*2, 7 , -(vec[i].second).x * (vec[i].first).y );
        gsl_matrix_set(A, i*2, 8 , -(vec[i].second).x);

        //для у
        gsl_matrix_set(A, i*2+1, 0 ,0);
        gsl_matrix_set(A, i*2+1, 1 ,0);
        gsl_matrix_set(A, i*2+1, 2 ,0);
        gsl_matrix_set(A, i*2+1, 3 ,(vec[i].first).x);
        gsl_matrix_set(A, i*2+1, 4 ,(vec[i].first).y);
        gsl_matrix_set(A, i*2+1, 5 ,1);
        gsl_matrix_set(A, i*2+1, 6 , -(vec[i].second).y * (vec[i].first).x );
        gsl_matrix_set(A, i*2+1, 7 , -(vec[i].second).y * (vec[i].first).y );
        gsl_matrix_set(A, i*2+1, 8 , -(vec[i].second).y);
    }


    gsl_matrix *ATA = gsl_matrix_alloc(9,9);
    //These functions compute the matrix-matrix product and sum C = \alpha op(A) op(B) + \beta C
    //where op(A) = A, A^T, A^H for TransA = CblasNoTrans, CblasTrans, CblasConjTrans and similarly for the parameter TransB.
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, A, A, 0, ATA);

    //gsl_linalg_SV_decomp (gsl_matrix * A, gsl_matrix * V, gsl_vector * S, gsl_vector * work)
    //matrix A into the singular value decomposition A = U S V^T
    //On output the matrix A is replaced by U.
    gsl_matrix *V = gsl_matrix_alloc(9, 9);
    gsl_vector *S = gsl_vector_alloc(9);
    gsl_vector *work = gsl_vector_alloc(9);
    gsl_linalg_SV_decomp(ATA, V, S, work);
}
