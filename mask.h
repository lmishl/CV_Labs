#pragma once
#include <memory>
#include <QtGlobal>
using namespace std;

class Mask
{
public:
    Mask(int h, int w);
    Mask(Mask&& _r);

    float get(int i, int j) const;
    void set(int i, int j, float value);

    int getHeight() const;
    int getWidth() const;
private:
    const unique_ptr<float[]> kernel;
    const int height;
    const int width;
};
