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

//shared_ptr<Mask> MaskFactory::Gauss(float sigma)
//{
//
//}
