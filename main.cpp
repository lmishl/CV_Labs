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


pair<int, int> findClosestPair(Descriptor _p, vector<Descriptor> _vec)
{
    int res1 = 0, res2 = 1;
    float minDist1 = _p.dist(_vec[0]);
    float minDist2 = _p.dist(_vec[1]);
    if(minDist1 > minDist2)
    {
        swap(minDist1, minDist2);
        swap(res1, res2);
    }

    for(int i = 2; i < _vec.size(); i++)
    {
        float dist = _p.dist(_vec[i]);
        if(dist < minDist1)     //нашли меньше обоих
        {
            swap(minDist1, minDist2);
            swap(res1, res2);
            minDist1 = dist;
            res1 = i;
        }
        else
            if(dist < minDist2)     //нашли меньше второго
            {
                minDist2 = dist;
                res2 = i;
            }
    }
    return pair<int, int>(res1, res2);
}



int main()
{
    QString fileName1 = "C:\\4\\q1.png";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);
    myIm1 = myIm1->GaussFilterSep(0.8, EdgeMode::COPY);
    QString fileName2 = "C:\\4\\q2.png";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);
    myIm2 = myIm2->GaussFilterSep(0.8, EdgeMode::COPY);


    vector<KeyPoint> points1 = myIm1->Harris(4, 100);//Moravec(0.02, 300);//

    vector<KeyPoint> points2 = myIm2->Harris(4, 100);// Moravec(0.02, 300);//

    //myIm->addPoints(vec).save("C:\\1\\MoravecL.tif");

    // vector<KeyPoint> vecH = myIm->Harris(4, 300);
    //myIm->addPoints(vecH).save("C:\\1\\HarrisL.tif");
    myIm1->addPoints(points1).save("C:\\4\\Pq1.tif");
    myIm2->addPoints(points2).save("C:\\4\\Pq2.tif");

    DescriptorFactory factory1(*myIm1);
    DescriptorFactory factory2(*myIm2);

    //получаем все дескрипторы
    vector<Descriptor> descs1;
    for(int i = 0; i < points1.size(); i++)
    {
        descs1.emplace_back(*factory1.get(points1[i]));
    }

    vector<Descriptor> descs2;
    for(int i = 0; i < points2.size(); i++)
    {
        descs2.emplace_back(*factory2.get(points2[i]));
    }


    QImage unIm = myIm1->Union(*myIm2);//save("C:\\4\\Un.png");
    QPainter painter;
    painter.begin(&unIm);
    painter.setPen(Qt::darkCyan);
    //ищем пары
    for(int i = 0; i < descs1.size(); i++)
    {
        auto min = findClosestPair(descs1[i], descs2);
        int close1 = min.first;
        int close2 = min.second;

        float dist1 = descs1[i].dist(descs2[close1]);
        float dist2 = descs1[i].dist(descs2[close2]);

        if(dist1 / dist2 > 0.8)
            continue;       //ненадёжно


        KeyPoint left = descs1[i].getPoint();
        KeyPoint right = descs2[close1].getPoint();
        //кароче рисуем
        painter.drawLine(QPoint(left.y, left.x), QPoint(right.y + myIm1->getWidth(), right.x));


    }
    painter.end();


    unIm.save("C:\\4\\Un.png");

    cout<<"\ngood";


    return 0;
}
