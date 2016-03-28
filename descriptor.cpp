#include "descriptor.h"

Descriptor::Descriptor(const array<float, DescriptorDims> &_vec, KeyPoint _point):
    point(_point)
{
    vec = _vec;
    normalize();
}

float Descriptor::get(int _num) const
{
    return vec[_num];
}

float Descriptor::dist(const Descriptor &d) const
{


    float sum = 0;
    for(int i = 0; i < DescriptorDims; i++)
    {
        sum += (d.get(i) - get(i)) * (d.get(i) - get(i));
    }

    return sqrt(sum);
}


void Descriptor::normalize()
{
    float magn = magnitude();
    for(int i = 0; i < DescriptorDims; i++)
    {
        vec[i] /= magn;
        if(vec[i] > 0.2)
            vec[i] = 0.2;
    }

    magn = magnitude();
    for(int i = 0; i < DescriptorDims; i++)
    {
        vec[i] /= magn;
    }
}

float Descriptor::magnitude()   const
{
    float sum = 0;
    for(int i = 0; i < DescriptorDims; i++)
    {
        sum += get(i) * get(i);
    }

    return sqrt(sum);
}
