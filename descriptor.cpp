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

    return sum;
}
