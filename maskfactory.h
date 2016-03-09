#ifndef MASKFACTORY_H
#define MASKFACTORY_H
#include "mask.h"

class MaskFactory
{
public:
    MaskFactory();
    static Mask SobelX();
    static Mask SobelY();
};

#endif // MASKFACTORY_H
