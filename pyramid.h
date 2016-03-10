#pragma once
#include "pyramidlevel.h"
#include "image.h"
#include "maskfactory.h"

class Pyramid
{
public:
    Pyramid(const Image& _im, float _sigma0, int _numLevels, EdgeMode _mode);
    void output(const QString &dirName) const;
    float L(int _x, int _y, float _sigma) const;

private:
   vector<PyramidLevel> vec;
   //Image original;      нужна ли?
   float sigma0;
   int numLevels;
   int numOctave;
   float k;     //для очередной сигмы
};
