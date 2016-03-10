#include <QCoreApplication>
#include <stdio.h>
#include <memory>
#include <limits>
#include <iostream>
#include <QImage>
#include <QtDebug>
#include <math.h>
#include <time.h>
#include <image.h>
#include <mask.h>
#include <maskfactory.h>
using namespace std;

shared_ptr<Image> Sobel(const Image& _im, EdgeMode _mode)
{
    MaskFactory factory;
    shared_ptr<Image> gradX = _im.convolution(factory.SobelX(),_mode);
    shared_ptr<Image> gradY = _im.convolution(factory.SobelY(),_mode);

    int width = _im.getWidth();
    int height = _im.getHeight();
    shared_ptr<Image> result = make_shared<Image>(height, width);

    for(int i=0; i<height; i++)
        for(int j=0;j<width; j++)
        {
            float xPix = gradX->getPixel(i,j,_mode);
            float yPix = gradX->getPixel(i,j,_mode);
            float G = hypot(xPix,yPix);//sqrt(xPix*xPix + yPix*yPix);
            result->setPixel(i,j,G);
        }
    result->normalize();
    return result;
}

shared_ptr<Image> GaussFilterSep(const Image& _im, float _sigma, EdgeMode _mode)
{
    MaskFactory factory;
    auto pair = factory.GaussSeparated(2);
    return myIm->convolution(pair.first,pair.second,_mode);
}

shared_ptr<Image> GaussFilter(const Image& _im, float _sigma, EdgeMode _mode)
{
    MaskFactory factory;
    return myIm->convolution(factory.Gauss(2),_mode);
}

void BuildPyramid(const Image& _im, float _sigma0, int _numLevels, EdgeMode _mode)
{
    //досглаживаем до sigma0
    MaskFactory factory;
    float deltaSigma = sqrt(_sigma0 * _sigma0 - 0,25);
    shared_ptr<Image> curIm = GaussFilterSep(_im, deltaSigma, _mode);
    //начинаем вычислять октавы
    float k = pow(2,1./_numLevels);


}

int main()
{
    QString fileName = "C:\\1\\1.jpg";
    shared_ptr<Image> myIm = Image::fromFile(fileName);

    //shared_ptr<Image> res = Sobel(*myIm,EdgeMode::ZEROS);
    //res->toFile("C:\\1\\2n.jpg");

    MaskFactory factory;
    shared_ptr<Image> res1 = myIm->convolution(factory.Gauss(2),EdgeMode::COPY);
    res1->toFile("C:\\1\\11.jpg");
    auto pair = factory.GaussSeparated(2);
    shared_ptr<Image> res2 = myIm->convolution(pair.first,pair.second,EdgeMode::COPY);
    res2->toFile("C:\\1\\12.jpg");


    cout<<"\ngood";


    return 0;
}
