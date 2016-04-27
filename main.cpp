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
#include "transformation.h"

#define eps 0.005
#define iter 5000
using namespace std;

void select4(int size, int * forFill)
{

    forFill[0] = rand() % size;

    do
    {
        forFill[1] = rand() % size;
    }while(forFill[1] == forFill[0]);

    do
    {
        forFill[2] = rand() % size;
    }while(forFill[2] == forFill[0] || forFill[2] == forFill[1] );

    do
    {
        forFill[3] = rand() % size;
    }while(forFill[3] == forFill[0] || forFill[3] == forFill[1] || forFill[3] == forFill[2]);

}

Transformation DebugRansac(const vector<pair<KeyPoint, KeyPoint>> &matches, const Image &_im1, const Image &_im2, const QString &_fileName )
{
    Transformation bestT;
    srand(time(0));
    int size = matches.size();
    int bestInliers = 0;


    if(matches.size() < 4)
        return bestT;


    for(int curIter = 0; curIter < iter; curIter++)
    {

        //выбираем 4 разные точки
        vector<pair<KeyPoint, KeyPoint>> cur;
        int selected[4];
        select4(size, selected);
        for(int i = 0; i < 4; i++)
            cur.emplace_back(matches[selected[i]]);

        //смотрим что выбралось
        DrawMatches(_im1,_im2,cur,_fileName);

        //строим модель трансформации
        Transformation t(cur);


        //считаем inliers
        int inliers = 0;
        for( auto pairK: matches)
        {
            float x0 = pairK.first.x;
            float y0 = pairK.first.y;
            float x1 = pairK.second.x;
            float y1 = pairK.second.y;

            float expectedX = (t.H(0,0) * x0 + t.H(0,1) * y0 + t.H(0,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2));
            float expectedY = (t.H(1,0) * x0 + t.H(1,1) * y0 + t.H(1,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2));

            float C = hypot(x1 - expectedX, y1 - expectedY);

            if(C < eps)
                inliers++;
        }

        if(inliers > bestInliers)
        {
            bestT = t;
            bestInliers = inliers;
        }

        //        if(inliers > matches.size() / 2)
        //            break;
    }

    return bestT;


}

Transformation Ransac(const vector<pair<KeyPoint, KeyPoint>> &matches)
{
    Transformation bestT;
    srand(time(0));
    int size = matches.size();
    int bestInliers = 0;


    if(matches.size() < 4)
        return bestT;


    for(int curIter = 0; curIter < iter; curIter++)
    {

        //выбираем 4 разные точки
        vector<pair<KeyPoint, KeyPoint>> cur;
        int selected[4];
        select4(size, selected);
        for(int i = 0; i < 4; i++)
            cur.emplace_back(matches[selected[i]]);

        //строим модель трансформации
        Transformation t(cur);

        //считаем inliers
        int inliers = 0;
        for( auto pairK: matches)
        {
            float x0 = pairK.first.x;
            float y0 = pairK.first.y;
            float x1 = pairK.second.x;
            float y1 = pairK.second.y;

            float expectedX = (t.H(0,0) * x0 + t.H(0,1) * y0 + t.H(0,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2));
            float expectedY = (t.H(1,0) * x0 + t.H(1,1) * y0 + t.H(1,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2));

            float C = hypot(x1 - expectedX, y1 - expectedY);

            if(C < eps)
                inliers++;
        }

        if(inliers > bestInliers)
        {
            bestT = t;
            bestInliers = inliers;
        }

        //        if(inliers > matches.size() / 2)
        //            break;
    }

    return bestT;
}

void DrawPanorama(const Image &_im1, const Image &_im2, Transformation t, const QString &_fileName)
{
    int wRes =  _im1.getWidth() * 2;
    int hRes =  _im1.getHeight() * 2;
    QImage result = QImage(wRes, hRes, QImage::Format_RGB32);
    result.fill(0);
    QPainter painter(&result);

    painter.drawImage(0, 0, _im1.toQImage());


    for(int i = 0; i < _im2.getHeight(); i++)
    {
        for(int j = 0; j < _im2.getWidth(); j++)
        {
            float x0 = i;
            float y0 = j;

            float newX = (t.H(0,0) * x0 + t.H(0,1) * y0 + t.H(0,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2));
            float newY = (t.H(1,0) * x0 + t.H(1,1) * y0 + t.H(1,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2));

            if(newX >= 0 && newX < hRes && newY >= 0 && newY < wRes)
            {
                int color = _im2.getPixel(i, j);
                result.setPixel(newY, newX, qRgb(color, color, color));

            }
        }
    }

    painter.end();


    result.save(_fileName);
}


int main()
{
    unsigned int start_time =  clock(); // начальное время
    QString fileName1 = "C:\\8\\1.png";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);

    QString fileName2 = "C:\\8\\2.png";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);



    vector<Descriptor> descs1 = findBlobs(*myIm1->ot0do1(), 2, "C:\\8\\blob1.tif");
    cout<<"blob1  "<< (int)clock() - start_time<<endl;

    vector<Descriptor> descs2 = findBlobs(*myIm2->ot0do1(), 2, "C:\\8\\blob2.tif");
    cout<<"blob2  "<< (int)clock() - start_time<<endl;





    vector<pair<KeyPoint, KeyPoint>> matches = FindMatches(descs1, descs2);
    DrawMatches(*myIm1, *myIm2, matches, "C:\\8\\Un.png");

    //Transformation t = Ransac(matches);
    Transformation t = DebugRansac(matches,*myIm1, *myIm2, "C:\\8\\Debug.png");

    DrawPanorama(*myIm1, *myIm2, t, "C:\\8\\Pan.png");

    unsigned int search_time = (int)clock() - start_time; // искомое время
    cout<<"\ngood "<< search_time<<endl;
    return 0;
}
