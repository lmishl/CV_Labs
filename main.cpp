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
            float yPix = gradY->getPixel(i,j,_mode);
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

    for(uint i = 2; i < _vec.size(); i++)
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

vector<pair<KeyPoint, KeyPoint>> FindMatches(vector<Descriptor> descs1, vector<Descriptor> descs2)
{
    vector<pair<KeyPoint, KeyPoint>> res;
    for(int i = 0; i < descs1.size(); i++)
    {
        auto min = findClosestPair(descs1[i], descs2);
        int close1 = min.first;
        int close2 = min.second;

        float dist1 = descs1[i].dist(descs2[close1]);
        float dist2 = descs1[i].dist(descs2[close2]);

        if(dist1 / dist2 > 0.8)
            continue;       //ненадёжно

        res.emplace_back(descs1[i].getPoint(), descs2[close1].getPoint());
    }
    return res;
}

void DrawMatches(const Image &_im1, const Image &_im2, vector<pair<KeyPoint, KeyPoint>> _matches, const QString &_fileName)
{
    QImage unIm = _im1.Union(_im2);//save("C:\\4\\Un.png");
    QPainter painter;
    painter.begin(&unIm);

    int addW = _im1.getWidth();

    for(int i = 0; i < _matches.size(); i++)
    {
        KeyPoint left = _matches[i].first;
        KeyPoint right = _matches[i].second;
        //кароче рисуем
        QPen qqq(QColor(rand() % 255, rand() % 255, rand() % 255));
        painter.setPen(qqq);
        painter.drawLine(QPoint(left.globY(), left.globX()), QPoint(right.globY() + addW, right.globX()));

        //рисуем окрестность и направление
        int side1 = GistSize * GistNum * pow(2,left.numberOctave);
        int halfSide1 = side1/2;
        pair<float, float> p0 = rotate(left.globX(), left.globY(), left.globX() - halfSide1, left.globY() - halfSide1, left.angle);
        pair<float, float> p1 = rotate(left.globX(), left.globY(), left.globX() - halfSide1, left.globY() + halfSide1, left.angle);
        pair<float, float> p2 = rotate(left.globX(), left.globY(), left.globX() + halfSide1, left.globY() + halfSide1, left.angle);
        pair<float, float> p3 = rotate(left.globX(), left.globY(), left.globX() + halfSide1, left.globY() - halfSide1, left.angle);
        painter.drawLine(p0.second, p0.first, p1.second, p1.first);
        painter.drawLine(p1.second, p1.first, p2.second, p2.first);
        painter.drawLine(p2.second, p2.first, p3.second, p3.first);
        painter.drawLine(p3.second, p3.first, p0.second, p0.first);
        pair<float, float> pA = rotate(left.globX(), left.globY(), left.globX() - halfSide1, left.globY(), left.angle);
        painter.drawLine(pA.second, pA.first, left.globY(), left.globX());
       // painter.drawRect(left.globY()- razmer1 / 2, left.globX()- razmer1 / 2, razmer1, razmer1);
        int side2 = GistSize * GistNum * pow(2, right.numberOctave);
        int halfSide2 = side2/2;
        pair<float, float> p10 = rotate(right.globX(), right.globY(), right.globX() - halfSide2, right.globY() - halfSide2, right.angle);
        pair<float, float> p11 = rotate(right.globX(), right.globY(), right.globX() - halfSide2, right.globY() + halfSide2, right.angle);
        pair<float, float> p12 = rotate(right.globX(), right.globY(), right.globX() + halfSide2, right.globY() + halfSide2, right.angle);
        pair<float, float> p13 = rotate(right.globX(), right.globY(), right.globX() + halfSide2, right.globY() - halfSide2, right.angle);
        painter.drawLine(p10.second + addW, p10.first, p11.second + addW, p11.first);
        painter.drawLine(p11.second + addW, p11.first, p12.second + addW, p12.first);
        painter.drawLine(p12.second + addW, p12.first, p13.second + addW, p13.first);
        painter.drawLine(p13.second + addW, p13.first, p10.second + addW, p10.first);
        pair<float, float> p1A = rotate(right.globX(), right.globY(), right.globX() - halfSide2, right.globY(), right.angle);
        painter.drawLine(p1A.second+ addW, p1A.first, right.globY()+ addW, right.globX());
    }
    painter.end();


    unIm.save(_fileName);
}

void DrawBlobs(const Image& _im, const vector<KeyPoint>& _blobs, const QString &_fileName)
{
    //рисуем сучие блобы
    QImage qim = _im.ot0do255()->addPoints(_blobs);
    QPainter painter;
    painter.begin(&qim);
    painter.setPen(Qt::red);
    for(int i = 0; i < _blobs.size(); i++)
    {
        float r = _blobs[i].sigma * sqrt(2) ;
        float x0 = _blobs[i].globX();
        float y0 = _blobs[i].globY();

        painter.drawEllipse(QPoint(y0, x0), (int)(r), (int)(r));
    }

    painter.end();

    qim.save(_fileName);
}



vector<Descriptor> findBlobs(const Image& _im, float T,  const QString &_fileName)
{
    vector<Descriptor> res;


    Pyramid pyr(_im);
    vector<KeyPoint> blobs = pyr.getDOG()->findExtemums();

    for(int i = 0; i < blobs.size(); i++)
    {

        if(pyr.getImage(blobs[i].sigma)->HarrisForPoint(blobs[i]) < T)
        {
            blobs.erase(blobs.begin() + i);
            i--;
        }
    }

    DrawBlobs(_im, blobs, _fileName);       //рисование

    if(blobs.size() == 0)
        return res;

    //теперь найдём дескрипторы к оставшимся блобам

    float curSigma = blobs[0].sigma;
    shared_ptr<Image> img = pyr.getImage(curSigma);
    shared_ptr<DescriptorFactory> factory = make_shared<DescriptorFactory>(*img);
    vector<KeyPoint> cur;
    for(int i = 0; i < blobs.size(); i++)
    {
        if(blobs[i].sigma != curSigma)
        {
            //переходим на следующую сигму, перед этим надо посчитать дескрипторы для накопленных точек в cur
            vector<Descriptor> temp = factory->get(cur);
            for(int q = 0; q < temp.size(); q++ )
                res.emplace_back(temp[q]);
            //res.insert(res.end(),temp.begin(),temp.end());      //Не работает !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



            //переходим на следующую сигму
            cur.clear();
            curSigma = blobs[i].sigma;
            shared_ptr<Image> img = pyr.getImage(curSigma);
            factory = make_shared<DescriptorFactory>(*img);
        }
        cur.push_back(blobs[i]);


    }


    return res;
}


int main()
{
    unsigned int start_time =  clock(); // начальное время
    QString fileName1 = "C:\\7\\w1.png";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);

    QString fileName2 = "C:\\7\\w2.png";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);



    vector<Descriptor> descs1 = findBlobs(*myIm1->ot0do1(), 1, "C:\\7\\blob1.tif");

    cout<<"blob1  "<< (int)clock() - start_time<<endl;
    vector<Descriptor> descs2 = findBlobs(*myIm2->ot0do1(), 1, "C:\\7\\blob2.tif");


    cout<<"blob2  "<< (int)clock() - start_time<<endl;



    vector<pair<KeyPoint, KeyPoint>> matches = FindMatches(descs1, descs2);
    DrawMatches(*myIm1, *myIm2, matches, "C:\\7\\Un.png");

    unsigned int search_time = (int)clock() - start_time; // искомое время
    cout<<"\ngood "<< search_time<<endl;
    return 0;
}
