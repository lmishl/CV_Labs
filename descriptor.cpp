#include "descriptor.h"

Descriptor::Descriptor(vector<float> _vec):
    vec(_vec)
{
}

float Descriptor::get(int _num) const
{
    return vec[_num];
}

float Descriptor::dist(Descriptor d) const
{
    int len = length();

    if(d.length() != len)
        qFatal("Compare different descriptors");

    float sum = 0;
    for(int i = 0; i < len; i++)
    {
        sum += (d.get(i) - get(i)) * (d.get(i) - get(i));
    }

    return sqrt(sum);
}


void Descriptor::normalize()
{
    int len = length();
    float magn = magnitude();
    for(int i = 0; i < len; i++)
    {
        vec[i] /= magn;
        if(vec[i] > 0.2)
            vec[i] = 0.2;
    }

    magn = magnitude();
    for(int i = 0; i < len; i++)
    {
        vec[i] /= magn;
    }
}

float Descriptor::magnitude()   const
{
    float sum = 0;
    int len = length();
    for(int i = 0; i < len; i++)
    {
        sum += get(i) * get(i);
    }

    return sqrt(sum);
}
