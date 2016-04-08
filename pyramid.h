#pragma once
#include "pyramidlevel.h"
#include "image.h"
#include "maskfactory.h"
static const double Sigma0 = 1.6;
static const unsigned NumLevels = 5;

class Pyramid
{
public:
    Pyramid(const Image& _im);
    Pyramid(int _numOctave, float _k);
    void output(const QString &dirName) const;
    float L(int _x, int _y, float _sigma) const;
    shared_ptr<Pyramid> getDOG() const;
    vector<KeyPoint> findExtemums() const;


private:   
    vector<PyramidLevel> vec;
    int numOctave;
    float k;     //для очередной сигмы
};
