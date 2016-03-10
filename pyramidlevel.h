#pragma once
#include "image.h"


class PyramidLevel
{
public:
    PyramidLevel();
    PyramidLevel(float _sigma, float _k);
    void add(shared_ptr<Image> _im);
    shared_ptr<Image> get(int i) const;
    int size() const;
    float globalSigma(int i) const;
    float localSigma(int i) const;
private:
    vector<shared_ptr<Image>> vec;
    int number;
    float sigma;
    float k;
};

