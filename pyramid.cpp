#include "pyramid.h"

Pyramid::Pyramid(const Image& _im)
{
    int minS = min(_im.getHeight(), _im.getWidth());
    numOctave = log2(minS) - 5;      //строим новую октаву пока изображение не станет меньше 64

    //досглаживаем до Sigma0
    float deltaSigma = sqrt(Sigma0 * Sigma0 - 0.25);
    shared_ptr<Image> curIm = _im.GaussFilterSep(deltaSigma, EdgeMode::COPY);

    //начинаем вычислять октавы
    k = pow(2,1./NumLevels);

    for(int i = 0; i < numOctave; i++ )
    {
        PyramidLevel level(Sigma0, k, i);
        level.add(curIm->ot0do1());
        float curSigma = Sigma0;
        for(int j = 0; j < NumLevels + 1; j++)
        {
            float newSigma = curSigma * k;
            float deltaSigma = sqrt(newSigma * newSigma - curSigma * curSigma);

            curIm = curIm->GaussFilterSep(deltaSigma,  EdgeMode::COPY);
            level.add(curIm->ot0do1());
            curSigma = newSigma;
        }
        curIm = level.get(NumLevels)->DownScale();
        vec.emplace_back(level);
    }

}

Pyramid::Pyramid(int _numOctave, float _k)
{
    numOctave = _numOctave;
    k = _k;

}


shared_ptr<Pyramid> Pyramid::getDOG() const
{
    shared_ptr<Pyramid> res = make_shared<Pyramid>(numOctave, k);

    for(int i = 0; i < numOctave; i++ )
    {
        PyramidLevel dogLevel = *vec[i].getDOGLevel();
        res->vec.emplace_back(dogLevel);
    }

    return res;
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

shared_ptr<Image> Pyramid::getImage(float _sigma) const
{
    int level = 1;
    while(vec[level].globalSigma(0) < _sigma && level < vec.size())
        level++;
    level--;
    return vec[level].getFromSigma(_sigma);
}


vector<KeyPoint> Pyramid::findExtemums() const
{
    vector<KeyPoint> result;

    for(int i = 0; i < numOctave; i++)
    {
        vector<KeyPoint> points = vec[i].findExtemums();
        result.insert(result.end(),points.begin(),points.end());
    }
    return result;
}
