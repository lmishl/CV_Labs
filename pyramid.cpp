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
        PyramidLevel level;
        level.add(curIm);
        for(int j = 0; j < _numLevels; j++)
        {
            curIm = curIm->GaussFilterSep(k, _mode);
            level.add(curIm);
        }
        curIm = curIm->DownScale();
        vec.push_back(level);
    }

    //вывод
    //for(int i = 0; i < vec.size(); i++)
    //{
    //    float trueSigma = _sigma0 * pow(k,i  - (i  / (_numLevels+1)));
    //    vec[i]->toFile(("C:\\1\\output\\" + to_string(i) + "sig-" +to_string(trueSigma) + ".jpg").c_str());
    //}
}
