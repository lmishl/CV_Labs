#include "maskfactory.h"


MaskFactory::MaskFactory()
{

}
Mask MaskFactory::SobelX()
{
    static float mask[] = {-1,0,1,-2,0,2,-1,0,1};
    Mask result(3,3);
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            result.set(i,j,mask[i*3+j]);
        }
    }
    return result;
}
Mask MaskFactory::SobelY()
{
    static float mask[] = {-1,-2,-1,0,0,0,1,2,1};
    Mask result(3,3);
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            result.set(i,j,mask[i*3+j]);
        }
    }
    return result;
}

Mask MaskFactory::Gauss(float sigma)
{
    int half = ceil(3*sigma);
    int size = 2 * half + 1;
    float sigma2 = sigma*sigma;
    Mask result(size,size);
    for(int i = -half; i <= half; i++)
        for(int j = -half; j <= half; j++)
        {
            float val= pow(M_E,-(i*i+j*j) / (2*sigma2)) / (2* M_PI*sigma2);
            result.set(i,j,val);
        }
    return result;

}
pair<Mask,Mask> MaskFactory::GaussSeparated(float sigma)
{
    int half = ceil(3*sigma);
    int size = 2 * half + 1;
    float sigma2 = sigma*sigma;

    pair<Mask,Mask> result (Mask(1,size), Mask(size,1));

    float value;
    for(int i=-half; i<=half; i++)
    {

        value = (pow(M_E,-(i*i)/(2*sigma2)))/(sqrt(2*M_PI)*sigma);
        result.first.set(0, i, value);
        result.first.set(i, 0, value);
    }

    return result;
}

