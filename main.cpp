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
#define iter 100
using namespace std;

int* select4(int size)
{
    int selected[4];
    selected[0] = rand() % size;

    do
    {
        selected[1] = rand() % size;
    }while(selected[1] == selected[0]);

    do
    {
        selected[2] = rand() % size;
    }while(selected[2] == selected[0] || selected[2] == selected[1] );

    do
    {
        selected[3] = rand() % size;
    }while(selected[3] == selected[0] || selected[3] == selected[1] || selected[3] == selected[2]);

    return selected;
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
        int * selected = select4(size);
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

        if(inliers > matches.size() / 2)
            break;
    }

    return bestT;
}


int main()
{
    unsigned int start_time =  clock(); // начальное время
    QString fileName1 = "C:\\7\\w1.png";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);

    QString fileName2 = "C:\\7\\w2.png";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);



    vector<Descriptor> descs1 = findBlobs(*myIm1->ot0do1(), 2, "C:\\7\\blob1.tif");
    cout<<"blob1  "<< (int)clock() - start_time<<endl;

    vector<Descriptor> descs2 = findBlobs(*myIm2->ot0do1(), 2, "C:\\7\\blob2.tif");
    cout<<"blob2  "<< (int)clock() - start_time<<endl;





    vector<pair<KeyPoint, KeyPoint>> matches = FindMatches(descs1, descs2);
    DrawMatches(*myIm1, *myIm2, matches, "C:\\7\\Un.png");

    Transformation t = Ransac(matches);

    unsigned int search_time = (int)clock() - start_time; // искомое время
    cout<<"\ngood "<< search_time<<endl;
    return 0;
}
