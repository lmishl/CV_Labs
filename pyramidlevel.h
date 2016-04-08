#pragma once
#include "image.h"


class PyramidLevel
{
public:
    PyramidLevel(float _sigma, float _k, int _number);
    void add(const shared_ptr<Image> &_im);
    shared_ptr<Image> get(int i) const;
    int size() const;
    float globalSigma(int i) const;
    float localSigma(int i) const;
    int getNumber() const;
    float L(int _x, int _y, float _sigma) const;
    shared_ptr<PyramidLevel> getDOGLevel() const;
    vector<KeyPoint> findExtemums() const;
private:
    vector<shared_ptr<Image>> vec;
    int number;     //номер октавы
    float sigma;
    float k;
};

