#include "image.h"

Image::Image(int h, int w)
{
    image = make_unique<float[]>(h*w);
    height = h;
    width = w;
}

shared_ptr<Image> Image::fromFile(const QString& fileName)
{
    QImage picture;
    bool isLoaded = picture.load(fileName);
    if(isLoaded)
    {
        qDebug() << "Load image from file: " << fileName;
        return fromQImage(picture);
    }
    else
    {
        qWarning() << "Failed to load image from file";
        return nullptr;
    }
}

shared_ptr<Image> Image::fromQImage(QImage picture)
{
    shared_ptr<Image> result = make_shared<Image>(picture.height(), picture.width());
    QRgb original;
    float color;

    for(int i=0; i<picture.height(); i++)
    {
        for(int j=0; j<picture.width(); j++)
        {
            original = picture.pixel(j,i);
            color = 0.299 * qRed(original) +
                    0.587 * qGreen(original) +
                    0.114 * qBlue(original);
            result->setPixel(i,j,color);
        }
    }

    return result;
}

QImage Image::toQImage() const
{
    QImage result = QImage(width, height, QImage::Format_RGB32);
    int color;
    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            color = qRound(getPixel(i, j));
            result.setPixel(j, i, qRgb(color,color,color));
        }
    }
    return result;
}

bool Image::toFile(const QString &fileName) const
{
    QImage image = toQImage();
    return image.save(fileName, "JPEG");
}

float Image::getPixel(int i, int j, EdgeMode mode) const
{
    if(i<height && j<width && i>=0 && j>=0)
    {
        return image[i*width + j];
    }
    else
    {
        //qFatal("out of the range");
        switch(mode)
        {
            case EdgeMode::ZEROS: return 0;
            case EdgeMode::COPY:
                 return image[min(max(i,0), height-1)*width + min(max(j,0), width-1)];
            case EdgeMode::MIRROR:
                if(i<0) i = i*(-1) - 1;
                if(j<0) j = j*(-1) - 1;
                if(i>=height) i = height - (i%height) - 1;
                if(j>=width) j = width - (j%width) - 1;
                return image[i*width + j];
        }
    }
    return 0;
}

float Image::setPixel(int i, int j, float value)
{
    if(i<height && j<width && i>=0 && j>=0)
    {
        image[i*width + j] = value;
        return value;
    }
    else
    {
        qFatal("setPixel: index out of the range");
        return -1;
    }
}

shared_ptr<Image> Image::convolution(shared_ptr<Mask> mask,EdgeMode mode)
{
    shared_ptr<Image> result = make_shared<Image>(height, width);
    int maskH = mask->getHeight();
    int maskW = mask->getWidth();
    for(int i=0; i< height; i++)
    {
        for(int j=0; j<width; j++)
        {
            float sum=0;
            for(int i1=0; i1< maskH; i1++)
            {
                for(int j1=0; j1 < maskW; j1++)
                {
                    float iPix = getPixel(i-(i1-maskH/2), j-(j1-maskW/2),mode);
                    float mPix = mask->getPixel(i1,j1);
                    sum += iPix * mPix;
                }
            }
            result->setPixel(i,j,sum);
        }
    }
    return result;

}

void Image::normalize()
{
    //хз как подружить max_element и unic_ptr
    auto mm = std::minmax_element(&(image[0]),&(image[width*height]));
    float min = * mm.first;
    float max = * mm.second;//  float max = image[0];
   ///for(int i=0; i<width*height; i++)
   ///{
   ///    if(max<image[i])
   ///        max = image[i];
   ///    if(min>image[i])
   ///        min = image[i];
   ///}

    for(int i=0; i<width*height; i++)
    {
        image[i]= (image[i] - min)/(max-min)*255;
    }
}

int Image::getHeight() const
{
    return height;
}

int Image::getWidth() const
{
    return width;
}
