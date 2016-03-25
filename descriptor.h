#pragma once
#include "image.h"

class Descriptor
{
public:

    Descriptor(vector<float> _vec);
    float dist(Descriptor d) const;
    float get(int _num) const;
    int length() const
    {
        return vec.size();
    }


private:
    vector<float> vec;
    float magnitude() const;
    void normalize();
};

