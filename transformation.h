#pragma once
#include "image.h"
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>


class Transformation
{
public:
    Transformation(vector<pair<KeyPoint, KeyPoint>> vec);
};

