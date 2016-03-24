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
            angles->setPixel(i,j,atan2(dy->getPixel(i,j), dx->getPixel(i, j))*180/M_PI + 180);

            magnitudes->setPixel(i, j, hypot(dx->getPixel(i,j), dy->getPixel(i,j)));
        }
    }
}


shared_ptr<Descriptor> DescriptorFactory::get(KeyPoint _p, int _gistSize, int _gistNum, int _basketNum)
{

    auto descriptor = make_shared<Descriptor>();

    int netSize = _gistNum * _gistSize;
    vector<float> result;

    float basketSize = 360.0 / _basketNum; //размер корзины в градусах

    for(int i =0; i < _gistNum; i++)
        for(int j =0; j < _gistNum; j++)
        {
            //левый верхний угол гистограммы
            int x = _p.x - (_gistNum / 2) * _gistSize + _gistNum * i * _gistSize;
            int y = _p.y - (_gistNum / 2) * _gistSize + _gistNum * j * _gistSize;

            float basket[_basketNum];

            for(int u = 0; u < _gistNum; u++)
                for(int v = 0; v < _gistNum; v++)
                {
                    float weight = magnitudes->getPixel(x + u, y + v, EdgeMode::COPY);
                    float angle = angles->getPixel(x + u, y + v, EdgeMode::COPY);


                    //начинаем раскидывать по корзинам
                    int basket1 = angle / basketSize;   // главная корзина
                    int basket2 = -1;                   //соседняя корзина
                    float b1Center = basket1 * basketSize - basketSize / 2;

                    //вычисляем соседнюю
                    if(angle > b1Center)
                        basket2  = basket1 + 1;
                    else
                        basket2  = basket1 - 1;

                    //обрабатываем крайние варианты
                    if(basket2 == -1)
                        basket2 =_basketNum - 1;
                    if(basket2 == _basketNum)
                        basket2 = 0;

                    //считаем расстояния до центров
                    float b2Center = basket2 * basketSize - basketSize / 2;

                    float b1Dist = abs(angle - b1Center);
                    float b2Dist = basketSize - b1Dist;

                    //раскидываем обратнопропорционально расстоянию
                    basket[basket1] += weight * (1 - b1Dist / basketSize) ;
                    basket[basket2] += weight * (1 - b2Dist / basketSize) ;


                }

            for(int k = 0; k < _basketNum; k++)
                result.emplace_back(basket[k]);


        }




}
