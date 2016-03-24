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

