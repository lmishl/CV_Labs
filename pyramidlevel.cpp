#include "pyramidlevel.h"

PyramidLevel::PyramidLevel()
{

}
PyramidLevel::PyramidLevel(float _sigma, float _k)
{
    sigma = _sigma;
    k = _k;
}


void PyramidLevel::add(shared_ptr<Image> _im)
{
    vec.push_back(_im);
}

shared_ptr<Image> PyramidLevel::get(int i) const
{
    return vec[i];
}

int PyramidLevel::size() const
{
    return vec.size();
}

float PyramidLevel::globalSigma(int i) const
{
    return sigma * pow(k,i);
}

float PyramidLevel::localSigma(int i) const
{
    return pow(k,i);
}
