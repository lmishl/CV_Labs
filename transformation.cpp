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

}

