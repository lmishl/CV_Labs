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

vector<int> selectGists(float newY, float newX)
{
    //Узнаём в какую гистограмму попадает точка
    int mainGistX = newX / GistSize;
    int mainGistY = newY / GistSize;



    vector<int> activeGists;
    int mainGist = mainGistX * GistNum + mainGistY;
    activeGists.emplace_back(mainGist);
    int mainCenterX = mainGistX * GistSize + GistSize / 2;
    int mainCenterY = mainGistY * GistSize + GistSize / 2;



    if(newX >= mainCenterX)  //точка ниже середины
    {
        //низ
        if(mainGistX + 1 < GistNum)  //это была не нижняя гистограмма
        {
            activeGists.emplace_back(mainGist + GistNum);   //добавляем нижнюю гистограмму
            if(newY >= mainCenterY && mainGistY + 1 < GistNum )//точка правее середины и гистограмма не самая правая
                activeGists.emplace_back(mainGist + GistNum + 1);   //добавляем правую нижнюю гистограмму
            if(newY < mainCenterY && mainGistY > 0)//точка левее середины и гистограмма не самая левая
                activeGists.emplace_back(mainGist + GistNum - 1);   //добавляем левую нижнюю гистограмму
        }
    }
    else //точка выше середины
    {
        if(mainGistX > 0)  //это была не верхняя гистограмма
        {
            activeGists.emplace_back(mainGist - GistNum);   //добавляем верхнюю гистограмму
            if(newY >= mainCenterY && mainGistY + 1 < GistNum )//точка правее середины и гистограмма не самая правая
                activeGists.emplace_back(mainGist - GistNum + 1);   //добавляем правую верхнюю гистограмму
            if(newY < mainCenterY && mainGistY > 0)//точка левее середины и гистограмма не самая левая
                activeGists.emplace_back(mainGist - GistNum - 1);   //добавляем левую верхнюю гистограмму
        }
    }
    //бока
    if(newY >= mainCenterY && mainGistY + 1 < GistNum )//точка правее середины и гистограмма не самая правая
        activeGists.emplace_back(mainGist + 1);   //добавляем правую  гистограмму
    if(newY < mainCenterY && mainGistY > 0)//точка левее середины и гистограмма не самая левая
        activeGists.emplace_back(mainGist - 1);

    return activeGists;
}

array<float, DescriptorDims> DescriptorFactory::getFinalBins(const KeyPoint _point, int netSize, float mainAngle, int y, int x, float binSize)
{
    //НУЖНО БОЛЬШЕ КОЭФФИЦИЕНТОВ!!!!!!
    //Типа взвешивание для гистограмм
    // static float gistK[16] = {1.0/28, 3.0/56, 3.0/56, 1.0/28, 3.0/56, 3.0/28, 3.0/28, 3.0/56, 3.0/56, 3.0/28, 3.0/28, 3.0/56, 1.0/28, 3.0/56, 3.0/56, 1.0/28};



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

            float weight = magnitudes->getPixel(i, j, EdgeMode::COPY);
            float angle = angles->getPixel(i, j, EdgeMode::COPY) - mainAngle;
            if(angle < 0)
                angle += 360;


            newX -= x;
            newY -= y;

            int HistX = floor(newX / GistSize + 0.5);
            int HistY = floor(newY / GistSize + 0.5);
            int HistA = floor(angle / binSize + 0.5);       //корзина

            for(int xx = HistX - 1; xx <= HistX; xx++ )
            {
                if(xx >= GistNum || xx < 0)
                    continue;
                int centerX = GistSize * xx + GistSize / 2;
                float Lx = fabs(newX - centerX);
                float wX = 1 - Lx / GistSize;

                for(int yy = HistY - 1; yy <= HistY; yy++ )
                {
                    if(yy >= GistNum || yy < 0)
                        continue;
                    int centerY = GistSize * yy + GistSize / 2;
                    float LY = fabs(newY - centerY);
                    float wY = 1 - LY / GistSize;

                    for(int aa = HistA - 1; aa <= HistA; aa++)
                    {
                        int bin = myProc(aa, BinNum);
                        float centerA = binSize * aa + binSize / 2;
                        float LA = fabs(angle - centerA);
                        float wA = 1 - LA / binSize;
                        if(wA > 1 || wA < 0)
                            cout<<"!!!!!!!!!wA "<<wA;
                       // assert(wA <= 1 && wA >= 0);
                        int index = xx * GistSize * BinNum + yy * BinNum + bin;
                        assert(index >= 0 && index < DescriptorDims);
                        //                        if(!(index >= 0 && index < DescriptorDims))
                        //                            wA++;

                        arr[index] += weight * wX * wY * wA;

                    }


                }
            }


            //            //ЛАБА 7
            //            //Реализовать распределение значений градиентов по смежным гистограммам, добавить весовые коэффициенты(globW) исходя из расстояния до соответствующих центров.

            //            //надо считать глобальный вес,
            //            //http://www.cyberforum.ru/mathematics/thread790820.html ваще огонь формула

            //            //выбираем между какими гистограммами будем раскидывать
            //            vector<int> activeGists = selectGists(newY, newX);

            //            float sum = 0; //сумма 1/расст
            //            vector<float> dist;
            //            for(uint q = 0; q < activeGists.size(); q++)
            //            {
            //                int centerX =(activeGists[q] / GistNum) * GistSize + GistSize / 2;
            //                int centerY =(activeGists[q] % GistNum) * GistSize + GistSize / 2;

            //                float d = hypot(newX - centerX, newY - centerY);
            //                dist.emplace_back(d);
            //                sum += 1 / d;
            //            }


            //            float k = 1 / sum;
            //            for(uint q = 0; q < activeGists.size(); q++)
            //            {

            //                int curGist = activeGists[q];
            //                assert(curGist <= 31 && curGist >= 0);

            //                float globW = k / dist[q];


            //                float weight = magnitudes->getPixel(i, j, EdgeMode::COPY);
            //                float angle = angles->getPixel(i, j, EdgeMode::COPY) - mainAngle;
            //                if(angle < 0)
            //                    angle += 360;

            //                //начинаем раскидывать по корзинам
            //                int bin1 = angle / binSize;   // главная корзина
            //                float b1Center = bin1 * binSize + binSize / 2;

            //                //вычисляем соседнюю
            //                int bin2 = bin1 + 1;
            //                if(angle < b1Center)
            //                    bin2 = bin1 - 1;
            //                //обрабатываем граничные случаи
            //                bin2 = (bin2 + BinNum) % BinNum;

            //                float b1Dist = abs(angle - b1Center);
            //                float b2Dist = binSize - b1Dist;

            //                //раскидываем обратнопропорционально расстоянию
            //                float w1 = weight * (b2Dist / binSize);
            //                float w2 = weight - w1;
            //                arr[curGist * GistSize + bin1] += w1 * globW * gistK[curGist];
            //                arr[curGist * GistSize + bin2] += w2 * globW * gistK[curGist];

            //            }
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
        if(anglesArr[bin2] > 0.8 * anglesArr[bin1] && abs(myProc(bin2,BinNum) - myProc(bin1,BinNum)) > 1 && abs(myProc(bin2,BinNum) - myProc(bin1,BinNum)) < BinNum - 1)
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
