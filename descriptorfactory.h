#pragma once
#include "image.h"

class DescriptorFactory
{
public:
    DescriptorFactory(const Image& image);
private:
    unique_ptr<Image> magnitudes;
    unique_ptr<Image> angles;
};
