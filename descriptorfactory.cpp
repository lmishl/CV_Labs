#include "descriptorfactory.h"

DescriptorFactory::DescriptorFactory(const Image &image)
{
    //auto work = image.ot0do1();
    MaskFactory factory;
    auto dx = image.convolution(factory.SobelX(), EdgeMode::COPY);
    auto dy = image.convolution(factory.SobelY(), EdgeMode::COPY);


    magnitudes = make_unique<Image>(image.getHeight(), image.getWidth());
    angles = make_unique<Image>(image.getHeight(), image.getWidth());

    for(int i = 0; i < image.getHeight(); i++)
    {
        for(int j = 0; j < image.getWidth(); j++)
        {
            angles->setPixel(i,j,atan2(dy->getPixel(i,j), dx->getPixel(i, j)) * 180 / M_PI + 180);
            magnitudes->setPixel(i, j, hypot(dx->getPixel(i,j), dy->getPixel(i,j)));
        }
    }
}

pair<int, int> findMaxPair(const  array<float, AnglesBinNum> &_arr )
{
    int res1 = 0, res2 = 1;
    float max1 = _arr[0];
    float max2 = _arr[1];
    if(max1 < max2)
    {
        swap(max1, max2);
        swap(res1, res2);
    }


    for(int i = 2; i < _arr.size(); i++)
    {
        float val = _arr[i];
        if(val > max1)     //нашли меньше обоих
        {
            swap(max1, max2);
            swap(res1, res2);
            max1 = val;
            res1 = i;
        }
        else
            if(val > max2)     //нашли меньше второго
            {
                max2 = val;
                res2 = i;
            }
    }
    return pair<int, int>(res1, res2);
}



float interpol(float x0, float x1, float x2, float y0, float y1, float y2)
{
    float a2 = (y2 - y0) / (x2 - x0) / (x2 - x1) - (y1 - y0) / (x1 - x0) / (x2 - x1);
    float a1 = (y1 - y0) / (x1 - x0) - a2 * (x1 + x0);
    float a0 = y0 - a1 * x0 - a2 * x0 * x0;

    return -a1 / 2 / a2;
}


array<float, AnglesBinNum> DescriptorFactory::findAngleBins(int x, int netSize, int y, float anglesBinSize)
{
    array<float, AnglesBinNum> anglesArr;
    anglesArr.fill(0);
    for(int i = 0; i < netSize; i++)
        for(int j = 0; j < netSize; j++)
        {
            float weight = magnitudes->getPixel(x + i, y + j, EdgeMode::COPY);
            float angle = angles->getPixel(x + i, y + j, EdgeMode::COPY);
            //начинаем раскидывать по корзинам
            int bin1 = angle / anglesBinSize;   // главная корзина
            float b1Center = bin1 * anglesBinSize + anglesBinSize / 2;

            //вычисляем соседнюю
            int bin2 = bin1 + 1;
            if(angle < b1Center)
                bin2  = bin1 - 1;
            //обрабатываем граничные случаи
            bin2 = (bin2 + AnglesBinNum) % AnglesBinNum;

            float b1Dist = abs(angle - b1Center);
            float b2Dist = anglesBinSize - b1Dist;


            //раскидываем обратнопропорционально расстоянию
            float w1 = weight * (b2Dist / anglesBinSize);
            float w2 = weight - w1;
            anglesArr[bin1] += w1;
            anglesArr[bin2] += w2;
        }

    return anglesArr;
}

array<float, DescriptorDims> DescriptorFactory::getFinalBins(const KeyPoint _point, int netSize, float mainAngle, int y, int x, float binSize)
{
    array<float, DescriptorDims> arr;
    arr.fill(0);

    int maxDist = (netSize/2.0) * sqrt(2) + 1;    //максимально удалённая рассматриваемая точки
    for(int i = _point.x - maxDist; i < _point.x + maxDist; i++)
        for(int j = _point.y - maxDist; j < _point.y + maxDist; j++)
        {
            auto temp = rotate(_point.x, _point.y, i, j, mainAngle);

            float newX = temp.first;
            float newY = temp.second;

            //Итак мы получили координаты после поворота. теперь можем отнять правую верхнюю точку и считать как раньше
            //но сначала проверим что новые координаты попали в рассматриваемую область
            if(newX < x || newX > x + netSize || newY < y || newY > y + netSize)
                continue;

            newX -= x;
            newY -= y;

            //Узнаём в какую гистограмму попадает точка
            int curGistX = newX / GistSize;
            int curGistY = newY / GistSize;



            //ЛАБА 7
            //Реализовать распределение значений градиентов по смежным гистограммам, добавить весовые коэффициенты(globW) исходя из расстояния до соответствующих центров.

            //надо считать глобальный вес,
            //http://www.cyberforum.ru/mathematics/thread790820.html ваще огонь формула

            array <float,GistSize*GistSize> dist;
            float sum = 0; //сумма 1/расст
            for(int u = curGistX - 1; u <= curGistX + 1; u++)
                for(int v = curGistY - 1; v <= curGistY + 1; v++)
                {
                    if(u < 0 || u >= GistSize || v < 0 || v>= GistSize)
                        continue;
                    int curGist = u * GistNum + v;
                     assert(curGist <= 31 && curGist >= 0);

                    int centerX = u * GistSize + GistSize / 2;
                    int centerY = v * GistSize + GistSize / 2;

                    dist[curGist] = hypot(newX - centerX, newY - centerY);
                    sum+=dist[curGist];


                }

            float k = 1 / sum;
            for(int u = curGistX - 1; u <= curGistX + 1; u++)
                for(int v = curGistY - 1; v <= curGistY + 1; v++)
                {
                    if(u < 0 || u >= GistSize || v < 0 || v>= GistSize)
                        continue;
                    int curGist = u * GistNum + v;
                     assert(curGist <= 31 && curGist >= 0);

                    float globW = k / dist[curGist];


                    float weight = magnitudes->getPixel(i, j, EdgeMode::COPY);
                    float angle = angles->getPixel(i, j, EdgeMode::COPY) - mainAngle;
                    if(angle < 0)
                        angle += 360;

                    //начинаем раскидывать по корзинам
                    int bin1 = angle / binSize;   // главная корзина
                    float b1Center = bin1 * binSize + binSize / 2;

                    //вычисляем соседнюю
                    int bin2 = bin1 + 1;
                    if(angle < b1Center)
                        bin2 = bin1 - 1;
                    //обрабатываем граничные случаи
                    bin2 = (bin2 + BinNum) % BinNum;

                    float b1Dist = abs(angle - b1Center);
                    float b2Dist = binSize - b1Dist;

                    //раскидываем обратнопропорционально расстоянию
                    float w1 = weight * (b2Dist / binSize);
                    float w2 = weight - w1;
                    arr[curGist * GistSize + bin1] += w1 * globW;
                    arr[curGist * GistSize + bin2] += w2 * globW;

                }




        }

    return arr;
}

vector<Descriptor> DescriptorFactory::get(const vector<KeyPoint> &_points)
{
    const int netSize = GistNum * GistSize;

    float anglesBinSize = 360.0 / AnglesBinNum; //размер корзины для направления в градусах
    float binSize = 360.0 / BinNum; //размер итоговой корзины в градусах

    vector<Descriptor> res;

    for(int k = 0; k < _points.size(); k++)
    {
        KeyPoint curPoint = _points[k];
        //левая верхняя рассматриваемая точка
        int x = curPoint.x - (GistNum / 2) * GistSize;
        int y = curPoint.y - (GistNum / 2) * GistSize;


        //найдём пиковое направление

        //найдём гистограмму углов
        array<float, AnglesBinNum> anglesArr = findAngleBins(x, netSize, y, anglesBinSize);


         //Обработка главной корзины
        auto mainBins = findMaxPair(anglesArr);
        int bin1 = mainBins.first;
        //теперь интерполируем чтобы найти главное направление
        float angle0 = bin1 * anglesBinSize - anglesBinSize / 2;
        float angle1 = bin1 * anglesBinSize + anglesBinSize / 2;
        float angle2 = bin1 * anglesBinSize + anglesBinSize * 3 / 2;

        float mainAngle = interpol(angle0,angle1,angle2, anglesArr[myProc(bin1 - 1,BinNum)], anglesArr[bin1],anglesArr[myProc(bin1 + 1,BinNum)] );
        curPoint.angle = mainAngle;


        //Вычисляем наконец дескриптор- Итоговое распределение по корзинам
        array<float, DescriptorDims> arr = getFinalBins(curPoint, netSize, mainAngle, y, x, binSize);
        res.emplace_back(arr, curPoint);


        //обработка второй по величине корзины
        //она должна быть больше 0.8 первой и не быть соседней
        int bin2 = mainBins.second;
        if(anglesArr[bin2] > 0.8 * anglesArr[bin1] && abs(myProc(bin2,BinNum) - myProc(bin1,BinNum)) > 1)
        {
            float angle0 = bin2 * anglesBinSize - anglesBinSize / 2;
            float angle1 = bin2 * anglesBinSize + anglesBinSize / 2;
            float angle2 = bin2 * anglesBinSize + anglesBinSize * 3 / 2;

            float mainAngle = interpol(angle0,angle1,angle2, anglesArr[myProc(bin2 - 1,BinNum)], anglesArr[bin2],anglesArr[myProc(bin2 + 1,BinNum)] );

            //костыль
            assert(mainAngle < 360 && mainAngle > 0);


            curPoint.angle = mainAngle;
            //Итоговое распределение по корзинам
            array<float, DescriptorDims> arr = getFinalBins(curPoint, netSize, mainAngle, y, x, binSize); //здесь поебень
            res.emplace_back(arr, curPoint);
        }
    }

    return res;
}
