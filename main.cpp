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
#include <descriptorfactory.h>
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
   return result->normalize();

}


int main()
{
    QString fileName1 = "C:\\4\\q1.jpg";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);

    QString fileName2 = "C:\\4\\q2.jpg";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);


    vector<KeyPoint> vec1 = myIm1->Harris(4, 70);
    vector<KeyPoint> vec2 = myIm2->Harris(4, 70);

    //myIm->addPoints(vec).save("C:\\1\\MoravecL.tif");

   // vector<KeyPoint> vecH = myIm->Harris(4, 300);
    //myIm->addPoints(vecH).save("C:\\1\\HarrisL.tif");


    DescriptorFactory factory1(*myIm1);
    DescriptorFactory factory2(*myIm2);

    //Находим ближайщий



    cout<<"\ngood";


    return 0;
}
