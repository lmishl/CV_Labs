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

#define eps 10
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

    }

    //а теперь уточним нашу модель трансф-и
    vector<pair<KeyPoint, KeyPoint>> cur;
    for( auto pairK: matches)
    {
        float x0 = pairK.first.x;
        float y0 = pairK.first.y;
        float x1 = pairK.second.x;
        float y1 = pairK.second.y;

        float expectedX = (bestT.H(0,0) * x0 + bestT.H(0,1) * y0 + bestT.H(0,2)) / (bestT.H(2,0) * x0 + bestT.H(2,1) * y0 + bestT.H(2,2));
        float expectedY = (bestT.H(1,0) * x0 + bestT.H(1,1) * y0 + bestT.H(1,2)) / (bestT.H(2,0) * x0 + bestT.H(2,1) * y0 + bestT.H(2,2));

        float C = hypot(x1 - expectedX, y1 - expectedY);

        if(C < eps)
            cur.emplace_back(pairK);
    }


    return Transformation(cur);
}

void DrawPanorama(const Image &_im1, const Image &_im2, Transformation t, const QString &_fileName)
{
    int wRes =  _im2.getWidth() * 3;
    int hRes =  _im2.getHeight() * 3;
    QImage result = QImage(wRes, hRes, QImage::Format_RGB32);
    result.fill(0);
    QPainter painter(&result);

    painter.drawImage(wRes / 3, hRes / 3, _im2.toQImage());

    for(int i = 0; i < _im1.getHeight(); i++)
    {
        for(int j = 0; j < _im1.getWidth(); j++)
        {
            float x0 = i;
            float y0 = j;

            float newX = (t.H(0,0) * x0 + t.H(0,1) * y0 + t.H(0,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2)) + hRes / 3;
            float newY = (t.H(1,0) * x0 + t.H(1,1) * y0 + t.H(1,2)) / (t.H(2,0) * x0 + t.H(2,1) * y0 + t.H(2,2)) + wRes / 3;

            if(newX >= 0 && newX < hRes && newY >= 0 && newY < wRes)
            {
                int color = _im1.getPixel(i, j);
                result.setPixel(newY, newX, qRgb(color, color, color));

            }
        }
    }

    painter.end();
    result.save(_fileName);
}

void DrawPanoramaColor(const QString &_imName1, const QString &_imName2, Transformation t, const QString &_fileName)
{
    QImage im1;
    im1.load(_imName1);
    QImage im2;
    im2.load(_imName2);

    int wRes =  im2.width() * 3;
    int hRes =  im2.height() * 3;
    QImage result = QImage(wRes, hRes, QImage::Format_RGB32);
    result.fill(0);
    QPainter painter(&result);


    QTransform transform(t.H(1,1), t.H(0,1), t.H(2,1),
                         t.H(1,0), t.H(0,0), t.H(2,0),
                         t.H(1,2), t.H(0,2), t.H(2,2));


    QTransform translationTransform(1, 0, 0, 0, 1, 0, wRes / 3, hRes / 3, 1);

    transform *= translationTransform;

    painter.drawImage(wRes / 3, hRes / 3, im2);     //рисуем 2
    painter.setTransform(transform);                //вводим трансформацию
    painter.drawImage(0, 0, im1);                   //рисуем 1

    painter.end();

    result.save(_fileName);
}


void DrawPanorama2(const Image &_im1, const Image &_im2, Transformation t, const QString &_fileName)
{
    int wRes =  _im2.getWidth() * 3;
    int hRes =  _im2.getHeight() * 3;
    QImage result = QImage(wRes, hRes, QImage::Format_RGB32);
    result.fill(0);
    QPainter painter(&result);


    // http://doc.crossplatform.ru/qt/4.7.x/qtransform.html
    //     x' = m11*x + m21*y + dx
    //     y' = m22*y + m12*x + dy
    //     if (is not affine) {
    //         w' = m13*x + m23*y + m33
    //         x' /= w'
    //         y' /= w'
    //     }

    //    QTransform transform(2, 0, 0,
    //                         0 ,1, 0,
    //                         0, 0, 1);

    //должно быть так
    //    QTransform transform(t.H(0,0), t.H(1,0), t.H(2,0),
    //                         t.H(0,1), t.H(1,1), t.H(2,1),
    //                         t.H(0,2), t.H(1,2), t.H(2,2));

    //но ведь хрен!  у меня х и у поменяны местами
    QTransform transform(t.H(1,1), t.H(0,1), t.H(2,1),
                         t.H(1,0), t.H(0,0), t.H(2,0),
                         t.H(1,2), t.H(0,2), t.H(2,2));


    QTransform translationTransform(1, 0, 0, 0, 1, 0, wRes / 3, hRes / 3, 1);

    transform*=translationTransform;

    painter.drawImage(wRes / 3, hRes / 3, _im2.toQImage());     //рисуем 2
    painter.setTransform(transform);                            //вводим трансформацию
    painter.drawImage(0, 0, _im1.toQImage());                   //рисуем 1

    painter.end();

    result.save(_fileName);
}

void DrawModel(int x, int y, int a, float s, const Image &_im1, const Image &_im2, const QString &_fileName)
{
    QImage zaz = _im2.toQImage();
    QPainter painter;
    painter.begin(&zaz);
    QPen qqq(QColor(rand() % 255, rand() % 255, rand() % 255));
    painter.setPen(qqq);

    float scale = s;
    float angle = a;
    float x0 = x;
    float y0 = y;




    //теперь найдём другие 3 точки прямоугольника
    int w = _im1.getWidth() * scale;
    int h = _im1.getHeight() * scale;

    // найдём повёрнутые координаты на образце
    auto pp1 = rotate(0, 0, 0, w, -angle);

    //найдём повёрнутые координаты на изобр-и
    float x1 = x0 + pp1.first;
    float y1 = y0 + pp1.second;

    // найдём повёрнутые координаты на образце
    auto pp2 = rotate(0, 0, h, w, -angle);

    //найдём повёрнутые координаты на изобр-и
    float x2 = x0 + pp2.first;
    float y2 = y0 + pp2.second;

    // найдём повёрнутые координаты на образце
    auto pp3 = rotate(0, 0, h, 0, -angle);

    //найдём повёрнутые координаты на изобр-и
    float x3 = x0 + pp3.first;
    float y3 = y0 + pp3.second;


    painter.drawLine(y0, x0, y1, x1);
    painter.drawLine(y1, x1, y2, x2);
    painter.drawLine(y2, x2, y3, x3);
    painter.drawLine(y3, x3, y0, x0);

    painter.end();
    zaz.save(_fileName);


}


Transformation Hough(const vector<pair<KeyPoint, KeyPoint>> &_matches, const Image &_im1, const Image &_im2)
{
    //параметры
    //x, y - по 2%    angle по 18градусов, scale по log2 + 2
    int w = _im2.getWidth();
    int h = _im2.getHeight();
    //float xSize = 2, ySize = 2, aSize = 10, sSize = 0.2;
    int xBins = 50, yBins = 50, aBins = 24, sBins = 10;


    //
    int accum[xBins][yBins][aBins][sBins];
    for(int a = 0; a < xBins; a++)
        for(int b = 0; b < yBins; b++)
            for(int c = 0; c < aBins; c++)
                for(int d = 0; d < sBins; d++)
                    accum[a][b][c][d] = -1;

    vector<vector<uint>> votes;

    int curList = 0;
    for(uint i = 0; i < _matches.size(); i++)
    {
        KeyPoint p1 = _matches[i].first;
        KeyPoint p2 = _matches[i].second;


        float scale = p2.sigma / p1.sigma;

        float angle = p2.angle - p1.angle;

        //найдём левый верхний угол образца на 2ой картинке

        //1 - повернули точку образца на угол angle
        auto qq = rotate(0, 0, p1.globX(), p1.globY(), angle);

        //2 - найдём начало образца на изобр-и
        float x = p2.globX() - qq.first * scale;
        float y = p2.globY() - qq.second  * scale;


        //значения получили, раскидываем по корзинам
        int xBin = floor(x / h * xBins + 0.5);
        int yBin = floor(y / w * yBins + 0.5);
        int aBin = floor(angle / 360 * aBins + 0.5);
        int sBin = floor(log2(scale) + 2.5);

        for(int xx = xBin - 1; xx <= xBin; xx++ )
        {
            if(xx >= xBins || xx < 0)
                continue;
            for(int yy = yBin - 1; yy <= yBin; yy++ )
            {
                if(yy >= yBins || yy < 0)
                    continue;
                for(int aa = aBin - 1; aa <= aBin; aa++ )
                {
                    int ang = myProc(aa, aBins);
                    for(int ss = sBin - 1; ss <= sBin && ss < sBins && ss >= 0; ss++ )
                    {
                        if(ss >= sBins || ss < 0)
                            continue;

                        if(accum[xx][yy][ang][ss] == -1)
                        {
                            accum[xx][yy][ang][ss] = curList++;
                            vector<uint> temp;
                            temp.emplace_back(i);
                            votes.emplace_back(temp);
                        }
                        else
                            votes[accum[xx][yy][ang][ss]].emplace_back(i);
                    }
                }
            }
        }



    }

    //ищем максимум
    int maxVec = 0;
    int maxSize = votes[0].size();
    for(uint i = 1; i < votes.size(); i++)
    {
        if(votes[i].size() > maxSize)
        {
            maxSize = votes[i].size();
            maxVec = i;
        }
    }

    //быдло вставка --- рисуем максимум
    for(int a = 0; a < xBins; a++)
        for(int b = 0; b < yBins; b++)
            for(int c = 0; c < aBins; c++)
                for(int d = 0; d < sBins; d++)
                {
                    if(maxVec == accum[a][b][c][d])
                    {
                        int xx = h / xBins * (a + 0.5);
                        int yy = w / yBins * (b + 0.5);
                        int aa = 360 / aBins * (c + 0.5);
                        float ss =  pow(2, d - 1.5);// * sqrt(2);
                        cout<<xx<<endl<<yy<<endl<<aa<<endl<<ss<<endl<<"---------"<<endl;
                        DrawModel(xx, yy, aa, ss, _im1, _im2, "C:\\9\\bydlo.png");
                        cout<<a<<endl<<b<<endl<<c<<endl<<d<<endl;

                        break;
                    }
                }



    //Соберём вектор соответствий
    vector<pair<KeyPoint, KeyPoint>> vec;
    for(uint i = 1; i < votes[maxVec].size(); i++)
        vec.emplace_back(_matches[votes[maxVec][i]]);


    //строим модель


    return Transformation(vec);
}




void DrawModels(shared_ptr<Image> myIm1, shared_ptr<Image> myIm2, const vector<pair<KeyPoint, KeyPoint>> &matches, const QString &_fileName)
{

    QImage zaz = myIm2->toQImage();
    QPainter painter;
    painter.begin(&zaz);


    for(uint i = 0; i < matches.size(); i++)
    {
        QPen qqq(QColor(rand() % 255, rand() % 255, rand() % 255));
        painter.setPen(qqq);

        KeyPoint p1 = matches[i].first;
        KeyPoint p2 = matches[i].second;


        float scale = p2.sigma / p1.sigma;//pow(2, p2.numberOctave - p1.numberOctave) * ();

        float angle = p2.angle - p1.angle;

        //найдём левый верхний угол образца на 2ой картинке

        //1 - повернули точку образца на угол angle
        auto pp0 = rotate(0, 0, p1.globX(), p1.globY(), -angle);

        //2 - найдём начало образца на изобр-и
        float x0 = p2.globX() - pp0.first * scale;
        float y0 = p2.globY() - pp0.second * scale;




        //теперь найдём другие 3 точки прямоугольника
        int w = myIm1->getWidth() * scale;
        int h = myIm1->getHeight() * scale;

        // найдём повёрнутые координаты на образце
        auto pp1 = rotate(0, 0, 0, w, -angle);

        //найдём повёрнутые координаты на изобр-и
        float x1 = x0 + pp1.first;
        float y1 = y0 + pp1.second;

        // найдём повёрнутые координаты на образце
        auto pp2 = rotate(0, 0, h, w, -angle);

        //найдём повёрнутые координаты на изобр-и
        float x2 = x0 + pp2.first;
        float y2 = y0 + pp2.second;

        // найдём повёрнутые координаты на образце
        auto pp3 = rotate(0, 0, h, 0, -angle);

        //найдём повёрнутые координаты на изобр-и
        float x3 = x0 + pp3.first;
        float y3 = y0 + pp3.second;


        painter.drawLine(y0, x0, y1, x1);
        painter.drawLine(y1, x1, y2, x2);
        painter.drawLine(y2, x2, y3, x3);
        painter.drawLine(y3, x3, y0, x0);


    }
    painter.end();
    zaz.save(_fileName);

}

int main()
{
    unsigned int start_time =  clock(); // начальное время
    QString fileName1 = "C:\\9\\1.png";
    shared_ptr<Image> myIm1 = Image::fromFile(fileName1);

    QString fileName2 = "C:\\9\\2.png";
    shared_ptr<Image> myIm2 = Image::fromFile(fileName2);



    vector<Descriptor> descs1 = findBlobs(*(myIm1->ot0do1()), 1, "C:\\9\\blob1.tif");
    cout<<"blob1  "<< (int)clock() - start_time<<endl;

    vector<Descriptor> descs2 = findBlobs(*(myIm2->ot0do1()), 1, "C:\\9\\blob2.tif");
    cout<<"blob2  "<< (int)clock() - start_time<<endl;





    vector<pair<KeyPoint, KeyPoint>> matches = FindMatches(descs1, descs2);
   // DrawMatches(*myIm1, *myIm2, matches, "C:\\9\\Un.png");

  //  DrawModels(myIm1, myIm2, matches, "C:\\9\\temp.png");
    Transformation t = Hough(matches, *myIm1, *myIm2);

    DrawPanoramaColor(fileName1, fileName2, t, "C:\\9\\Pan2.png");

    unsigned int search_time = (int)clock() - start_time; // искомое время
    cout<<"\ngood "<< search_time<<endl;
    return 0;
}
