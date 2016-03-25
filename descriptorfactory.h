#pragma once
#include "image.h"
#include "descriptor.h"

class DescriptorFactory
{
public:
    DescriptorFactory(const Image& image);
    shared_ptr<Descriptor> get(KeyPoint _p);
private:
    unique_ptr<Image> magnitudes;
    unique_ptr<Image> angles;
};
