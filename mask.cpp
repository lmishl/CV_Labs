#include "mask.h"


Mask::Mask(int h, int w):
    height(h),
    width(w),
    kernel(make_unique<float[]>(h*w))
{
}

Mask::Mask(Mask&& _r):
    height(_r.height),
    width(_r.width),
    kernel( move(*const_cast<unique_ptr<float[]>*>(&_r.kernel)) )
{
}

float Mask::get(int i, int j) const
{
    if(i<height && j<width && i>=0 && j>=0)
    {
        return kernel[i*width + j];
    }
    else
    {
        qFatal("getPixel: index out of the range");
        return -1;
    }
}

void Mask::set(int i, int j, float value)
{
    if(i<height && j<width && i>=0 && j>=0)
    {
        kernel[i*width + j] = value;
    }
    else
    {
        qFatal("setPixel: index out of the range");
    }
}

int Mask::getHeight() const
{
    return height;
}
int Mask::getWidth() const
{
    return width;
}

