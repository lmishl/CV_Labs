#pragma once
#include "image.h"
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include<iostream>
using namespace std;


class Transformation
{
public:
    Transformation(vector<pair<KeyPoint, KeyPoint>> vec);
private:
    array<float, 9> h;
};

