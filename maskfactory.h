#ifndef MASKFACTORY_H
#define MASKFACTORY_H
#include "mask.h"

class MaskFactory
{
public:
    MaskFactory();
    shared_ptr<Mask> SobelX();
    shared_ptr<Mask> SobelY();
};

#endif // MASKFACTORY_H
