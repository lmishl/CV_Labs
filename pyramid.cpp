#include "pyramid.h"

Pyramid::Pyramid(const Image& _im, float _sigma0, int _numLevels, EdgeMode _mode)
{
    sigma0 = _sigma0;
    numLevels = _numLevels;

    int minS = min(_im.getHeight(), _im.getWidth());
    numOctave = log2(minS) - 5;      //строим новую октаву пока изображение не станет меньше 64

    //досглаживаем до sigma0
    MaskFactory factory;
    float deltaSigma = sqrt(sigma0 * sigma0 - 0.25);
    shared_ptr<Image> curIm = _im.GaussFilterSep(deltaSigma, _mode);

    //начинаем вычислять октавы
    k = pow(2,1./_numLevels);

    for(int i = 0; i < numOctave; i++ )
    {
        PyramidLevel level(sigma0, k, i);
        level.add(curIm);
        float curSigma = sigma0;
        for(int j = 0; j < _numLevels; j++)
        {
            float newSigma = curSigma * k;
            float deltaSigma = sqrt(newSigma * newSigma - curSigma * curSigma);

            curIm = curIm->GaussFilterSep(deltaSigma, _mode);
            level.add(curIm);
            curSigma = newSigma;
        }
        curIm = curIm->DownScale();
        vec.emplace_back(level);
    }

}

void Pyramid::output(const QString &dirName) const
{
    for(int i = 0, ei = int(vec.size()); i < ei; i++)
    {
        for(int j = 0; j < vec[i].size(); j++)
        {
            QString fileName = dirName + "\\" + QString::number(i) + "-" + QString::number(j) + "-sig-" + QString::number(vec[i].globalSigma(j)) + ".jpg";
            vec[i].get(j)->toFile(fileName);
        }
    }

}

float Pyramid::L(int _x, int _y, float _sigma) const
{
    int level = 1;
    while(vec[level].globalSigma(0) < _sigma && level < vec.size())
        level++;
    level--;
    return vec[level].L(_x, _y, _sigma);
}
