#pragma once
#include "image.h"
#include "descriptor.h"
static const unsigned AnglesBinNum = 36;

class DescriptorFactory
{
public:
    DescriptorFactory(const Image& image);
    vector<Descriptor> get(const vector<KeyPoint> &points);
private:
    unique_ptr<Image> magnitudes;
    unique_ptr<Image> angles;
    array<float, AnglesBinNum> findAngleBins(int x, int netSize, int y, float anglesBinSize);
    array<float, DescriptorDims> getFinalBins(const KeyPoint _point, int netSize, float mainAngle, int y, int x, float binSize);
};
