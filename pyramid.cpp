#include "pyramid.h"

Pyramid::Pyramid(const Image& _im, float _sigma0, int _numLevels, EdgeMode _mode)
{
    int minS = min(_im.getHeight(), _im.getWidth());
    int numOktav = log2(minS) - 5;      //строим новую октаву пока изображение не станет меньше 64

    //досглаживаем до sigma0
    MaskFactory factory;
    float deltaSigma = sqrt(_sigma0 * _sigma0 - 0.25);
    shared_ptr<Image> curIm = _im.GaussFilterSep(deltaSigma, _mode);

    //начинаем вычислять октавы
    float k = pow(2,1./_numLevels);
    vector<shared_ptr<Image>> vec;//= new vector<Image>();

    for(int i = 0; i < numOktav; i++ )
    {
        vec.push_back(curIm);
        for(int j = 0; j < _numLevels; j++)
        {
            curIm = curIm->GaussFilterSep(k, _mode);
            vec.push_back(curIm);
        }
        curIm = curIm->DownScale();
    }

    //вывод
    //for(int i = 0; i < vec.size(); i++)
    //{
    //    float trueSigma = _sigma0 * pow(k,i  - (i  / (_numLevels+1)));
    //    vec[i]->toFile(("C:\\1\\output\\" + to_string(i) + "sig-" +to_string(trueSigma) + ".jpg").c_str());
    //}
}
