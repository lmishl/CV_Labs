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

//vector<shared_ptr<Image>> BuildPyramid(const Image& _im, float _sigma0, int _numLevels, EdgeMode _mode)
//{
//    int minS = min(_im.getHeight(), _im.getWidth());
//    int numOktav = log2(minS) - 5;      //строим новую октаву пока изображение не станет меньше 64
//
//    //досглаживаем до sigma0
//    MaskFactory factory;
//    float deltaSigma = sqrt(_sigma0 * _sigma0 - 0.25);
//    shared_ptr<Image> curIm = GaussFilterSep(_im, deltaSigma, _mode);
//
//    //начинаем вычислять октавы
//    float k = pow(2,1./_numLevels);
//    vector<shared_ptr<Image>> vec;//= new vector<Image>();
//
//    for(int i = 0; i < numOktav; i++ )
//    {
//        vec.push_back(curIm);
//        for(int j = 0; j < _numLevels; j++)
//        {
//            curIm = GaussFilterSep(*curIm, k, _mode);
//             vec.push_back(curIm);
//        }
//        curIm = curIm->DownScale();
//    }
//
//    //вывод
//    for(int i = 0; i < vec.size(); i++)
//    {
//        float trueSigma = _sigma0 * pow(k,i  - (i  / (_numLevels+1)));
//        vec[i]->toFile(("C:\\1\\output\\" + to_string(i) + "sig-" +to_string(trueSigma) + ".jpg").c_str());
//    }
//
//    return vec;
//}



int main()
{
    QString fileName = "C:\\1\\1.jpg";
    shared_ptr<Image> myIm = Image::fromFile(fileName);

    //BuildPyramid(*myIm, 1.6, 6, EdgeMode::COPY);
    Pyramid p(*myIm, 1.3, 6, EdgeMode::COPY);
    p.output("C:\\1\\output");
    cout<<myIm->getPixel(1000,1000)<<endl;
    cout<<p.L(1000,1000,3)<<endl;
    cout<<p.L(1000,1000,6)<<endl;
    cout<<p.L(1000,1000,15)<<endl;
    cout<<"\ngood";


    return 0;
}
