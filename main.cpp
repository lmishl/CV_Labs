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

 shared_ptr<Image> Sobel(shared_ptr<Image> im, EdgeMode mode)
{
    MaskFactory factory;
    shared_ptr<Image> gradX = im->convolution(factory.SobelX(),mode);
    shared_ptr<Image> gradY = im->convolution(factory.SobelY(),mode);

    int width = im->getWidth();
    int height = im->getHeight();
    shared_ptr<Image> result = make_shared<Image>(height, width);

    for(int i=0; i<height; i++)
        for(int j=0;j<width; j++)
        {
            float xPix = gradX->getPixel(i,j,mode);
            float yPix = gradX->getPixel(i,j,mode);
            float G = hypot(xPix,yPix);//sqrt(xPix*xPix + yPix*yPix);
            result->setPixel(i,j,G);
        }
    result->normalize();
    return result;
}

int main()
{
    QString fileName = "C:\\1\\1.jpg";
    shared_ptr<Image> myIm = Image::fromFile(fileName);

    shared_ptr<Image> res = Sobel(myIm,EdgeMode::ZEROS);
    res->toFile("C:\\1\\2n.jpg");
    cout<<"\ngood";


    return 0;
}
