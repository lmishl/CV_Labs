#include "pyramidlevel.h"

PyramidLevel::PyramidLevel(float _sigma, float _k, int _number)
{
    sigma = _sigma;
    k = _k;
    number = _number;
}


void PyramidLevel::add(const shared_ptr<Image> &_im)
{
    vec.emplace_back(_im);
}

shared_ptr<Image> PyramidLevel::get(int i) const
{
    return vec[i];
}

shared_ptr<Image> PyramidLevel::getFromSigma(float _sigma) const
{
    int level = 1;
    while(globalSigma(level) < _sigma && level < vec.size())
        level++;
    level--;
    return vec[level];
}

int PyramidLevel::size() const
{
    return vec.size();
}

float PyramidLevel::globalSigma(int i) const
{
    return localSigma(i) * pow(2, number);
}

float PyramidLevel::localSigma(int i) const
{
    return sigma * pow(k,i);
}

int PyramidLevel::getNumber() const
{
    return number;
}
float PyramidLevel::L(int _x, int _y, float _sigma) const
{
    int level = 1;
    while(globalSigma(level) < _sigma && level < vec.size())
        level++;
    level--;
    int x = _x / pow(2, number);
    int y = _y / pow(2, number);

    return vec[level]->getPixel(x,y);
}

shared_ptr<PyramidLevel> PyramidLevel::getDOGLevel() const
{
    shared_ptr<PyramidLevel> res = make_shared<PyramidLevel>(sigma, k, number);
    int resSize = vec.size() - 1;

    for(int i = 0; i < resSize; i++)
    {
        res->add(vec[i]->minus(*vec[i + 1]));
    }

    return res;
}

vector<KeyPoint> PyramidLevel::findExtemums() const
{
    int w = vec[0]->getWidth();
    int h = vec[0]->getHeight();
    vector<KeyPoint> res;

    for(int q = 1; q < vec.size() - 1; q++)
    {
        for(int i = 1; i < h - 1; i++)
        {
            for(int j = 1; j < w - 1; j++)
            {
                if(fabs(vec[q]->getPixel(i, j)) > 0.002)
                    if(isExtremum(q, i, j))
                    {
                        res.emplace_back(i, j, 0, globalSigma(q), number);
                    }
            }
        }
    }
    return res;
}


bool PyramidLevel::isExtremum(int _q, int _i, int _j) const
{
    std::array<float,26> mas;
    //низ
    mas[0] = vec[_q - 1]->getPixel(_i - 1, _j - 1);
    mas[1] = vec[_q - 1]->getPixel(_i - 1, _j);
    mas[2] = vec[_q - 1]->getPixel(_i - 1, _j + 1);
    mas[3] = vec[_q - 1]->getPixel(_i, _j - 1);
    mas[4] = vec[_q - 1]->getPixel(_i, _j);
    mas[5] = vec[_q - 1]->getPixel(_i, _j + 1);
    mas[6] = vec[_q - 1]->getPixel(_i + 1, _j - 1);
    mas[7] = vec[_q - 1]->getPixel(_i + 1, _j);
    mas[8] = vec[_q - 1]->getPixel(_i + 1, _j + 1);
    //середина
    mas[9] = vec[_q]->getPixel(_i - 1, _j - 1);
    mas[10] = vec[_q]->getPixel(_i - 1, _j);
    mas[11] = vec[_q]->getPixel(_i - 1, _j + 1);
    mas[12] = vec[_q]->getPixel(_i, _j - 1);
    mas[13] = vec[_q]->getPixel(_i, _j + 1);
    mas[14] = vec[_q]->getPixel(_i + 1, _j - 1);
    mas[15] = vec[_q]->getPixel(_i + 1, _j);
    mas[16] = vec[_q]->getPixel(_i + 1, _j + 1);
    //верх
    mas[17] = vec[_q + 1]->getPixel(_i - 1, _j - 1);
    mas[18] = vec[_q + 1]->getPixel(_i - 1, _j);
    mas[19] = vec[_q + 1]->getPixel(_i - 1, _j + 1);
    mas[20] = vec[_q + 1]->getPixel(_i, _j - 1);
    mas[21] = vec[_q + 1]->getPixel(_i, _j);
    mas[22] = vec[_q + 1]->getPixel(_i, _j + 1);
    mas[23] = vec[_q + 1]->getPixel(_i + 1, _j - 1);
    mas[24] = vec[_q + 1]->getPixel(_i + 1, _j);
    mas[25] = vec[_q + 1]->getPixel(_i + 1, _j + 1);

    auto mm = minmax_element(mas.begin(), mas.end());
    float min = * mm.first;
    float max = * mm.second;
    float val = vec[_q]->getPixel(_i, _j);

    return val > max  || val < min;

}
