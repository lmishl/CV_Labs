#include "pyramidlevel.h"

PyramidLevel::PyramidLevel()
{

}
PyramidLevel::PyramidLevel(float _sigma, float _k, int _number)
{
    sigma = _sigma;
    k = _k;
    number = _number;
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

int PyramidLevel::getNumber() const
{
    return number;
}
float PyramidLevel::L(int _x, int _y, float _sigma) const
{
    int level = 1;
    while(globalSigma(level) < _sigma && level < vec.size())
        level++;
    level--;
    int x = _x / pow(2, number);
    int y = _y / pow(2, number);

    return vec[level]->getPixel(x,y);
}
