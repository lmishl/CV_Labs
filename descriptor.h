#pragma once
#include "image.h"
#include <assert.h>
static const unsigned GistSize = 4;
static const unsigned GistNum = 4;
static const unsigned BinNum = 8;
static const unsigned DescriptorDims = GistSize * GistNum * BinNum;

class Descriptor
{
public:

    Descriptor(const array<float, DescriptorDims> &_vec, KeyPoint _point);
    float dist(const Descriptor &d) const;
    float get(int _num) const;

    KeyPoint getPoint()
    {
        return point;
    }



private:
    array<float, DescriptorDims> vec;
    float magnitude() const;
    void normalize();
    const KeyPoint point;
};

