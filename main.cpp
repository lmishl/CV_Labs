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
    painter.setPen(Qt::darkCyan);

    for(int i = 0; i < _matches.size(); i++)
    {
        KeyPoint left = _matches[i].first;
        KeyPoint right = _matches[i].second;
        //кароче рисуем
        QPen qqq(QColor(rand() % 255, rand() % 255, rand() % 255));
        painter.setPen(qqq);
        painter.drawLine(QPoint(left.globY(), left.globX()), QPoint(right.globY() + _im1.getWidth(), right.globX()));
    }
    painter.end();


    unIm.save(_fileName);
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

    //рисуем сучие блобы
    QImage qim = _im.ot0do255()->addPoints(blobs);
    QPainter painter;
    painter.begin(&qim);
    painter.setPen(Qt::red);
    for(int i = 0; i < blobs.size(); i++)
    {
        //int number = log2(blobs[i].sigma / 1.6);
        //int k = pow(2, number);

        float r = blobs[i].sigma * sqrt(2) ;
        float x0 = blobs[i].globX();
        float y0 = blobs[i].globY();

        painter.drawEllipse(QPoint(y0, x0), (int)(r), (int)(r));
    }

    painter.end();


    qim.save(_fileName);




    if(blobs.size() == 0)
        return res;

    //вывод
    //_im.addPoints(blobs).save(_fileName);


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
    QString fileName1 = "C:\\6\\p1.png";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);
    //myIm1 = myIm1->GaussFilter(0.5, EdgeMode::COPY);
    QString fileName2 = "C:\\6\\p2.png";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);
    //myIm2 = myIm2->GaussFilterSep(0.5, EdgeMode::COPY);


    vector<Descriptor> descs1 = findBlobs(*myIm1->ot0do1(), 0, "C:\\6\\blob1.tif");
   // return 0;

    cout<<"blob1  "<< (int)clock() - start_time;
    vector<Descriptor> descs2 = findBlobs(*myIm2->ot0do1(), 0, "C:\\6\\blob2.tif");


    cout<<"blob2  "<< (int)clock() - start_time;


    //vector<KeyPoint> points1 = myIm1->Harris(3, 100);//Moravec(0.02, 300);//
    //vector<KeyPoint> points2 = myIm2->Harris(3, 100);//Moravec(0.02, 300);//
    //
    //
    //myIm1->addPoints(points1).save("C:\\6\\Pq1.tif");
    //myIm2->addPoints(points2).save("C:\\6\\Pq2.tif");
    //
    //DescriptorFactory factory1(*myIm1);
    //DescriptorFactory factory2(*myIm2);
    //
    ////получаем все дескрипторы
    //vector<Descriptor> descs1 = factory1.get(points1);
    //vector<Descriptor> descs2 = factory2.get(points2);
    //
    //
    vector<pair<KeyPoint, KeyPoint>> matches = FindMatches(descs1, descs2);
    DrawMatches(*myIm1, *myIm2, matches, "C:\\6\\Un.png");

    unsigned int search_time = (int)clock() - start_time; // искомое время
    cout<<"\ngood "<< search_time;
    return 0;
}
