#include "maskfactory.h"

MaskFactory::MaskFactory()
{

}
shared_ptr<Mask> MaskFactory::SobelX()
{
    static float mask[] = {-1,0,1,-2,0,2,-1,0,1};
    shared_ptr<Mask> result = make_shared<Mask>(3,3);
    for(int i=0; i<3; i++)
        {
            for(int j=0; j<3; j++)
            {
                result->setPixel(i,j,mask[i*3+j]);
            }
        }
        return result;
}
shared_ptr<Mask> MaskFactory::SobelY()
{
    static float mask[] = {-1,-2,-1,0,0,0,1,2,1};
    shared_ptr<Mask> result = make_shared<Mask>(3,3);
    for(int i=0; i<3; i++)
        {
            for(int j=0; j<3; j++)
            {
                result->setPixel(i,j,mask[i*3+j]);
            }
        }
        return result;
}

