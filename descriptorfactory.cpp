#include "descriptorfactory.h"

DescriptorFactory::DescriptorFactory(const Image &image)
{
    auto work = image.ot0do1();
    MaskFactory factory;
    auto dx = work->convolution(factory.SobelX(), EdgeMode::COPY);
    auto dy = work->convolution(factory.SobelY(), EdgeMode::COPY);


    magnitudes = make_unique<Image>(image.getHeight(), image.getWidth());
    angles = make_unique<Image>(image.getHeight(), image.getWidth());

    for(int i=0; i<image.getHeight(); i++)
    {
        for(int j=0; j<image.getWidth(); j++)
        {
            //float yy = dy->getPixel(i,j);
            //float xx = dx->getPixel(i,j);
            //float dist = hypot(xx,yy);
            //xx /= dist;
            //yy /= dist;
            //float res = atan2(dy->getPixel(i,j), dx->getPixel(i, j)) * 180 / M_PI + 180;

            angles->setPixel(i,j,atan2(dy->getPixel(i,j), dx->getPixel(i, j)) * 180 / M_PI + 180);
            magnitudes->setPixel(i, j, hypot(dx->getPixel(i,j), dy->getPixel(i,j)));
        }
    }
}


shared_ptr<Descriptor> DescriptorFactory::get(KeyPoint _p)
{
    array<float, DescriptorDims> result;
    result.fill(0);
    int netSize = GistNum * GistSize;



    //левая верхняя рассматриваемая точка
    int x = _p.x - (GistNum / 2) * GistSize;
    int y = _p.y - (GistNum / 2) * GistSize;




    //Итоговое распределение по корзинам
    float binSize = 360.0 / BinNum; //размер корзины в градусах
    for(int i = 0; i < netSize; i++)
        for(int j = 0; j < netSize; j++)
        {
            //Узнаём в какую гистограмму попадает точка
            int curGistX = i / GistSize;
            int curGistY = j / GistSize;
            int curGist = curGistX * GistNum + curGistY;


            float weight = magnitudes->getPixel(x + i, y + j, EdgeMode::COPY);
            float angle = angles->getPixel(x + i, y + j, EdgeMode::COPY);

            //начинаем раскидывать по корзинам
            int bin1 = angle / binSize;   // главная корзина
            float b1Center = bin1 * binSize - binSize / 2;

            //вычисляем соседнюю
            int bin2 = bin1 + 1;
            if(angle < b1Center)
                bin2  = bin1 - 1;
            //обрабатываем граничные случаи
            bin2 = (bin2 + BinNum) % BinNum;

            float b1Dist = abs(angle - b1Center);
            float b2Dist = binSize - b1Dist;


            //раскидываем обратнопропорционально расстоянию
            result[curGist * GistSize + bin1] += weight * (1 - b1Dist / binSize) ;
            result[curGist * GistSize + bin2] += weight * (1 - b2Dist / binSize) ;

        }

    return make_shared<Descriptor>(result, _p);
    //  return shared_ptr<Descriptor>(new Descriptor(result, _p));
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

vector<Descriptor> DescriptorFactory::get(const vector<KeyPoint> &_points)
{
    int netSize = GistNum * GistSize;
    float maxDist = (netSize/2.0) * sqrt(2);    //максимально удалённая рассматриваемая точки
    float anglesBinSize = 360.0 / AnglesBinNum; //размер корзины для направления в градусах
    float binSize = 360.0 / BinNum; //размер итоговой корзины в градусах

    vector<Descriptor> res;

    for(int k = 0; k < _points.size(); k++)
    {
        array<float, DescriptorDims> arr;
        arr.fill(0);

        //левая верхняя рассматриваемая точка
        int x = _points[k].x - (GistNum / 2) * GistSize;
        int y = _points[k].y - (GistNum / 2) * GistSize;


        //найдём пиковое направление
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
        int mainAngleInd = findMaxPair(anglesArr).first;       //главное направление------------------------дописать второе
        float mainAngle = mainAngleInd * anglesBinSize  + anglesBinSize / 2;

        int width = angles->getWidth();
        int height = angles->getHeight();
        //Итоговое распределение по корзинам
        for(int i = 0; i < width; i++)
            for(int j = 0; j < height; j++)
            {
                //находим угол
                float deltaX = i -_points[k].x;
                float deltaY = _points[k].y - j;
                float dist = hypot(deltaX, deltaY);
                if (dist > maxDist)
                    continue;

                float angleQQ = acos(deltaX / dist);
                if(deltaY < 0)
                    angleQQ = (2 * M_PI - angleQQ) / M_PI * 180;
                else
                    angleQQ = angleQQ / M_PI * 180;

                float newAngle = angleQQ - mainAngle;
                if(newAngle < 0)
                    newAngle += 360;

                float newDeltaX = cos(newAngle / 180 * M_PI) * dist;
                float newDeltaY = sin(newAngle / 180 * M_PI) * dist;

                float newX = _points[k].x  + newDeltaX;
                float newY = _points[k].y  - newDeltaY;

                //Итак мы получили координаты после поворота. теперь можем отнять правую верхнюю точку и считать как раньше
                //но сначала проверим что новые координаты попали в рассматриваемую область
                if(newX < x || newX > x + netSize || newY < y || newY > y + netSize)
                    continue;

                newX -+ x;
                newY -= y;

                //Узнаём в какую гистограмму попадает точка
                int curGistX = newX / GistSize;
                int curGistY = newY / GistSize;
                int curGist = curGistX * GistNum + curGistY;

                //assert
                if(curGist >31 || curGist <0)
                    curGist = 1;

                float weight = magnitudes->getPixel(i, j, EdgeMode::COPY);
                float angle = angles->getPixel(i, j, EdgeMode::COPY) - mainAngle;

                //начинаем раскидывать по корзинам
                int bin1 = angle / binSize;   // главная корзина
                float b1Center = bin1 * binSize + binSize / 2;

                //вычисляем соседнюю
                int bin2 = bin1 + 1;
                if(angle < b1Center)
                    bin2  = bin1 - 1;
                //обрабатываем граничные случаи
                bin2 = (bin2 + BinNum) % BinNum;

                float b1Dist = abs(angle - b1Center);
                float b2Dist = binSize - b1Dist;

                //раскидываем обратнопропорционально расстоянию
                float w1 = weight * (b2Dist / binSize);
                float w2 = weight - w1;
                arr[curGist * GistSize + bin1] += w1;
                arr[curGist * GistSize + bin2] += w2;

            }

        res.emplace_back(arr, _points[k]);
    }

    return res;
}
