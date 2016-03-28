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

    float binSize = 360.0 / BinNum; //размер корзины в градусах

    //левая верхняя рассматриваемая точка
    int x = _p.x - (GistNum / 2) * GistSize;
    int y = _p.y - (GistNum / 2) * GistSize;

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
