#pragma once
#include "mask.h"
#define _USE_MATH_DEFINES
#include <math.h>
#define M_E		2.7182818284590452354
#define M_PI		3.14159265358979323846


class MaskFactory
{
public:
    MaskFactory();
    static Mask SobelX();
    static Mask SobelY();
    static Mask Gauss(float sigma);
    static pair<Mask,Mask> GaussSeparated(float sigma);
};

