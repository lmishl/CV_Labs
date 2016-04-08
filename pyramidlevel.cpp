#include "pyramidlevel.h"

PyramidLevel::PyramidLevel(float _sigma, float _k, int _number)
{
    sigma = _sigma;
    k = _k;
    number = _number;
}


void PyramidLevel::add(const shared_ptr<Image> &_im)
{
    vec.emplace_back(_im);
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
    return localSigma(i) * pow(2, number);
}

float PyramidLevel::localSigma(int i) const
{
    return sigma * pow(k,i);
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

shared_ptr<PyramidLevel> PyramidLevel::getDOGLevel() const
{
    shared_ptr<PyramidLevel> res = make_shared<PyramidLevel>(sigma, k, number);
    int resSize = vec.size() - 1;

    for(int i = 0; i < resSize; i++)
    {
        res->add(vec[i]->minus(*vec[i + 1]));
    }

    return res;
}

vector<KeyPoint> PyramidLevel::findExtemums() const
{
    int w = vec[0]->getWidth();
    int h = vec[0]->getHeight();
    vector<KeyPoint> res;

    for(int k = 1; k < vec.size() - 1; k++)
    {
        for(int i = 1; i < w - 1; i++)
        {
            for(int j = 1; j < h - 1; j++)
            {
                if(isExtremum(k, i, j))
                    res.emplace_back()
            }
        }
    }
}


bool isExtremum(int _k, int _i, int _j)
{

}
