#pragma once

#include <memory>
#include <limits>
#include <QImage>
#include <QPainter>
#include <QtDebug>
#include <math.h>
#include <time.h>
#include <mask.h>
#include "maskfactory.h"
#include <assert.h>
using namespace std;

enum class EdgeMode
{
    ZEROS = 1,
    MIRROR = 2,
    COPY = 3
};

struct KeyPoint
{
    int x,y;
    float sigma, val;
    KeyPoint(int _x, int _y, float _val, float _sigma)
    {
        x = _x;
        y = _y;
        sigma = _sigma;
        val = _val;
    }

    float dist(KeyPoint p)
    {
        return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));

    }
};

class Image
{
public:
    Image(int h, int w);

    static shared_ptr<Image> fromFile(const QString &fileName);
    static shared_ptr<Image> fromQImage(const QImage &picture);
    shared_ptr<Image> convolution(const Mask &mask, EdgeMode mode) const;
    shared_ptr<Image> convolution(const Mask& row, const Mask& column, EdgeMode mode) const;
    shared_ptr<Image> DownScale() const;

    QImage toQImage() const;
    bool toFile(const QString &fileName)const;

    float getPixel(int i, int j, EdgeMode mode=EdgeMode::COPY) const;
    float getPixel(KeyPoint _p, EdgeMode _mode=EdgeMode::COPY) const;
    float setPixel(int i, int j, float value);

    int getHeight() const;
    int getWidth() const;

    shared_ptr<Image> normalize() const;
    shared_ptr<Image> ot0do1() const;

    shared_ptr<Image> GaussFilterSep(float _sigma, EdgeMode _mode) const;
    shared_ptr<Image> GaussFilter(float _sigma, EdgeMode _mode) const;

    vector<KeyPoint> Moravec(float _T, int _N) const;
    vector<KeyPoint> Harris(float _T, int _N) const;
    float HarrisForPoint(KeyPoint _p) const;
    QImage addPoints(vector<KeyPoint> _vec) const;

    QImage Union(const Image &rightIm) const;


    shared_ptr<Image> minus(const Image &rightIm) const;

private:
    const unique_ptr<float[]> image;
    const int height;
    const int width;
    vector<KeyPoint> FindLocalMax( float _T, int _N) const;
   // float Ix(int _i, int _j) const;
   // float Iy(int _i, int _j) const;

};

