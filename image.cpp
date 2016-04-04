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
            if(color>255)
                color = 255;
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

shared_ptr<Image> Image::normalize() const
{
    auto mm = minmax_element(&image[0],&image[width*height]);
    float min = * mm.first;
    float max = * mm.second;

    shared_ptr<Image> result = make_shared<Image>(height, width);

    for(int i=0; i< height; i++)
    {
        for(int j=0; j<width; j++)
        {
            float val = (getPixel(i,j) - min)/(max - min)*255;
            result->setPixel(i,j,val);
        }
    }
    return result;
}

shared_ptr<Image> Image::ot0do1() const
{
    shared_ptr<Image> result = make_shared<Image>(height, width);

    for(int i=0; i< height; i++)
    {
        for(int j=0; j<width; j++)
        {
            float val = getPixel(i,j) / 255;
            result->setPixel(i,j,val);
        }
    }
    return result;
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
            result->setPixel(i, j, getPixel(i*2,j*2));
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



vector<KeyPoint> Image::FindLocalMax( float _T, int _N) const
{
    int _py = 2;
    int _px = 2;

    vector<KeyPoint> res;
    //закинем в вектор всё что больше Т
    for(int i = 0; i < height; i ++)
        for(int j = 0; j < width; j ++)
        {
            float curV = getPixel(i, j);
            if(curV <_T)
                continue;
            bool isLocalMax = true;
            for(int dy = - _py; dy < _py; dy++)
                for(int dx = -_px; dx < _px; dx++)
                {
                    if(dx == 0 && dy == 0)
                        continue;
                    float val = getPixel(i + dy, j + dx);
                    if(val > curV)
                        isLocalMax = false;
                    if(val == curV)
                    {
                        if(dx>=0)   //если справа есть такой же - удаляем
                            isLocalMax = false;
                    }
                }
            if (isLocalMax)
                res.emplace_back(i, j, curV);
        }




    int R = 5;

    while(res.size()>_N)
    {
        for(int i = 0; i < res.size(); i ++)
        {
            for(int j = 0; j < res.size(); j ++)
                if(res[i].dist(res[j]) < R)     //нашли соседа
                    if( res[i].val < 0.9 * res[j].val)
                    {
                        res.erase(res.begin() + i);
                        i--;
                        break;
                    }
        }
        R++;

    }


    return res;

}


vector<KeyPoint> Image::Moravec(float _T, int _N) const
{
    int halfW = 2;     //стоит добавить в параметры?
    int halfH = 2;
    int maxU = 1;
    int maxV = 1;

    Image S(height, width);
    shared_ptr<Image> working = ot0do1();

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
                    for(int u = - halfH; u < maxU; u++)
                        for(int v = -halfW; v < maxV; v++)
                        {

                            float dif = working->getPixel(i + u, j + v, EdgeMode::COPY) - working->getPixel(i + u + dy, j + v + dx, EdgeMode::COPY);
                            sum += dif * dif;

                        }
                    c.emplace_back(sum);
                }

            float minV = *min_element(c.begin(),c.end());
            S.setPixel(i, j, minV);
        }

    return S.FindLocalMax(_T, _N);
}

//float Image::Ix(int _i, int _j) const
//{
//    float left = getPixel(_i, _j - 1, EdgeMode::COPY);
//    float right = getPixel(_i, _j + 1, EdgeMode::COPY);
//    return right - left;
//}
//
//float Image::Iy(int _i, int _j) const
//{
//    float up = getPixel(_i + 1, _j, EdgeMode::COPY);
//    float down = getPixel(_i - 1, _j, EdgeMode::COPY);
//    return up - down;
//}

vector<KeyPoint> Image::Harris(float _T, int _N) const
{
    int heightW = 5, widthW = 5;
    float k = 0.06;
    shared_ptr<Image> working = ot0do1();

    MaskFactory factory;
    shared_ptr<Image> gradX = working->convolution(factory.SobelX(), EdgeMode::COPY);
    gradX->toFile("C:\\1\\gradX.tif");
    shared_ptr<Image> gradY = working->convolution(factory.SobelY(), EdgeMode::COPY);
    gradY->toFile("C:\\1\\gradY.tif");
    Image A(height, width), B(height, width), C(height, width);
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
        {
            float a = 0, b = 0, c = 0;

            for(int dy = - heightW/2; dy < heightW/2; dy++)
                for(int dx = -widthW/2; dx < widthW/2; dx++)
                {
                    float x = gradX->getPixel(i + dy, j + dx);
                    float y = gradY->getPixel(i + dy, j + dx);
                    a += x * x;
                    b += x * y;
                    c += y * y;
                }

            A.setPixel(i, j, a);
            B.setPixel(i, j, b);
            C.setPixel(i, j, c);
        }
    //теперь знаем а б и с в каждой точке


    A.toFile("C:\\1\\A.tif");
    B.toFile("C:\\1\\B.tif");
    C.toFile("C:\\1\\C.tif");



    Image F(height, width);

    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
        {
            float a = A.getPixel(i, j);
            float b = B.getPixel(i, j);
            float c = C.getPixel(i, j);

            float det = a * c - b * b;
            float trace = a + c;
            float curF = det - k * trace * trace;

            F.setPixel(i, j, curF);
        }
    vector<KeyPoint> qq = F.FindLocalMax(_T, _N);
    F.normalize();
    F.toFile("C:\\1\\F.tif");

    return qq;
}

QImage Image::addPoints(vector<KeyPoint> _vec) const
{
    QImage res = toQImage();
    int size = _vec.size();
    for(int i = 0; i < size; i++)
    {
        res.setPixel(_vec[i].y, _vec[i].x + 1, qRgb(255,0,0));
        res.setPixel(_vec[i].y, _vec[i].x - 1, qRgb(255,0,0));
        res.setPixel(_vec[i].y + 1, _vec[i].x, qRgb(255,0,0));
        res.setPixel(_vec[i].y - 1, _vec[i].x, qRgb(255,0,0));

    }

    return res;
}


QImage Image::Union(const Image &rightIm) const
{
    QImage left = toQImage();
    QImage right = rightIm.toQImage();
    int h = max(left.height(), right.height());

    QImage destImage( left.width() + right.width(), h, QImage::Format_RGB32 );
    QPainter painter;
    painter.begin(&destImage);
    painter.drawImage(QPoint(0,0), left);
    painter.drawImage(QPoint(left.width(),0), right);
    painter.end();
    return destImage;
}
