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
#include "pyramid.h"
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


int main()
{
    QString fileName = "C:\\1\\lena.tif";
    shared_ptr<Image> myIm = Image::fromFile(fileName);

    vector<KeyPoint> vec = myIm->Moravec(300, 300);
    myIm->addPoints(vec).save("C:\\1\\MoravecL.tif");

    vector<KeyPoint> vecH = myIm->Harris(50000000, 300);
    myIm->addPoints(vecH).save("C:\\1\\HarrisL.tif");

//    Pyramid p(*myIm, 1.6, 6, EdgeMode::COPY);
//    p.output("C:\\1\\output");

    cout<<"\ngood";


    return 0;
}
