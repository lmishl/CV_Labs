#include "mask.h"


Mask::Mask(int h, int w)
{
    this->height = h;
    this->width = w;
    core = make_unique<float[]>(h*w);
}

float Mask::getPixel(int i, int j) const
{
    if(i<height && j<width && i>=0 && j>=0)
    {
        return core[i*width + j];
    }
    else
    {
        qFatal("getPixel: index out of the range");
        return -1;
    }
}

void Mask::setPixel(int i, int j, float value)
{
    if(i<height && j<width && i>=0 && j>=0)
    {
        core[i*width + j] = value;
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

