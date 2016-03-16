#include "image.h"

Image::Image(int h, int w) :
    height(h),
    width(w),
    image(make_unique<float[]>(h*w))
{
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

shared_ptr<Image> Image::fromQImage(const QImage& picture)
{
    int h =  picture.height();
    int w = picture.width();
    shared_ptr<Image> result = make_shared<Image>(h, w);
    QRgb original;
    float color;

    for(int i=0; i<h; i++)
    {
        for(int j=0; j<w; j++)
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

shared_ptr<Image> Image::convolution(const Mask& _mask, EdgeMode _mode) const
{
    shared_ptr<Image> result = make_shared<Image>(height, width);
    int maskH = _mask.getHeight();
    int maskW = _mask.getWidth();
    for(int i=0; i< height; i++)
    {
        for(int j=0; j<width; j++)
        {
            float sum=0;
            for(int i1=0; i1< maskH; i1++)
            {
                for(int j1=0; j1 < maskW; j1++)
                {
                    float iPix = getPixel(i-(i1-maskH/2), j-(j1-maskW/2),_mode);
                    float mPix = _mask.get(i1,j1);
                    sum += iPix * mPix;
                }
            }
            result->setPixel(i,j,sum);
        }
    }
    return result;

}

shared_ptr<Image> Image::convolution(const Mask& _row, const Mask& _column, EdgeMode _mode) const
{
    shared_ptr<Image> result = convolution(_row, _mode);
    result = result->convolution(_column, _mode);
    return result;

}

void Image::normalize()
{
    auto mm = minmax_element(&image[0],&image[width*height]);
    float min = * mm.first;
    float max = * mm.second;

    for(int i = 0; i < width * height; i++)
    {
        image[i] = (image[i] - min)/(max - min)*255;
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

shared_ptr<Image> Image::DownScale() const
{
    int h = height / 2;
    int w = width / 2;
    shared_ptr<Image> result = make_shared<Image>(h, w);
    for(int i = 0; i < h; i++)
        for(int j = 0; j < w; j++)
            result->setPixel(i,j, getPixel(i*2,j*2));
    return result;
}

shared_ptr<Image> Image::GaussFilterSep(float _sigma, EdgeMode _mode) const
{
    MaskFactory factory;
    auto pair = factory.GaussSeparated(_sigma);
    return convolution(pair.first,pair.second,_mode);
}

shared_ptr<Image> Image::GaussFilter(float _sigma, EdgeMode _mode) const
{
    MaskFactory factory;
    return convolution(factory.Gauss(_sigma),_mode);
}

vector<QPoint> Image::Moravec(EdgeMode _mode) const
{
    int halfW = 1;     //стоит добавить в параметры?
    int halfH = 1;

    int px = 5;         //стоит добавить в параметры?
    int py = 5;
    float T = 50;

    Image S(height, width);
    vector<QPoint> res;

    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
        {
            vector <float> c;
            for(int dy = - halfH; dy < halfH; dy++)
                for(int dx = -halfW; dx < halfW; dx++)
                {
                    if(dx==0 && dy==0)
                        continue;
                    float sum = 0;
                    for(int u = - halfH; u < halfH; u++)
                        for(int v = -halfW; v < halfW; v++)
                        {

                            float dif = getPixel(i + u, j + v, _mode) - getPixel(i + u + dy, j + v + dx, _mode);
                            sum += dif * dif;

                        }
                    c.emplace_back(sum);
                }

            float minV = *min_element(c.begin(),c.end());
            S.setPixel(i, j, minV);
        }
    //Функция S получена, надо отфильровать
    for(int i = 0; i < height; i += py)
        for(int j = 0; j < width; j += px)
        {
            float maxV = S.getPixel(i, j);
            QPoint maxP(i,j);
            for(int u = i; u < i + py; u++)
                for(int v = j; v < j + px; v++)
                {
                    float val = S.getPixel(u, v);
                    if(val > maxV)
                    {
                        maxV = val;
                        maxP.setX(u);
                        maxP.setY(v);
                    }

                }
            if(maxV > T)
                res.emplace_back(maxP);

        }

return res;
}
