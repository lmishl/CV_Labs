#pragma once
#include "image.h"
#include "descriptor.h"
static const unsigned AnglesBinNum = 36;

class DescriptorFactory
{
public:
    DescriptorFactory(const Image& image);
    shared_ptr<Descriptor> get(KeyPoint _p);
    vector<Descriptor> get(const vector<KeyPoint> &points);
private:
    unique_ptr<Image> magnitudes;
    unique_ptr<Image> angles;
};
