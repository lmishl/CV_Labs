#ifndef IMAGE_H
#define IMAGE_H


#include <memory>
#include <limits>
#include <QImage>
#include <QtDebug>
#include <math.h>
#include <time.h>
#include <mask.h>
using namespace std;

enum class EdgeMode
{
    ZEROS = 1,
    MIRROR = 2,
    COPY = 3
};

class Image
{
public:
    Image(int h, int w);

    static shared_ptr<Image> fromFile(const QString &fileName);
    static shared_ptr<Image> fromQImage(QImage picture);
    shared_ptr<Image> convolution(shared_ptr<Mask> mask, EdgeMode mode);

    QImage toQImage() const;
    bool toFile(const QString &fileName)const;

    float getPixel(int i, int j, EdgeMode mode=EdgeMode::ZEROS) const;
    float setPixel(int i, int j, float value);

    int getHeight() const;
    int getWidth() const;\

    void normalize();


private:
    unique_ptr<float[]> image;
    int height;
    int width;
};

#endif // IMAGE_H
