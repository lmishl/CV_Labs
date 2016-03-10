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

    float curSigma = sigma0;
    for(int i = 0; i < numOctave; i++ )
    {
        PyramidLevel level(curSigma, k, i);
        level.add(curIm);
        for(int j = 0; j < _numLevels; j++)
        {
            curIm = curIm->GaussFilterSep(k, _mode);
            level.add(curIm);
        }
        curIm = curIm->DownScale();
        vec.push_back(level);
        curSigma *= 2;
    }

}

void Pyramid::output(const QString &dirName) const
{
    for(int i = 0; i < vec.size(); i++)
    {
        for(int j = 0; j < vec[i].size(); j++)
        {
            //float trueSigma = sigma0 * pow(k,i  - (i  / (numLevels+1)));
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
    return vec[level].L(_x, _y, _sigma);
}
