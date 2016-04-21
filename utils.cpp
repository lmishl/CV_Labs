#include "utils.h"



pair<float, float> rotate(float x0, float y0, float x1, float y1, float angle)
{
    if(x0 == x1 && y0 == y1)
        return pair<float, float>(x0, y0);

    //находим угол
    float deltaX = x1 - x0;
    float deltaY = y0 - y1;
    float dist = hypot(deltaX, deltaY);

    float angleQQ = acos(deltaX / dist);
    if(deltaY < 0)
        angleQQ = (2 * M_PI - angleQQ) / M_PI * 180;
    else
        angleQQ = angleQQ / M_PI * 180;

    float newAngle = angleQQ - angle;
    if(newAngle < 0)
        newAngle += 360;

    float newDeltaX = cos(newAngle / 180 * M_PI) * dist;
    float newDeltaY = sin(newAngle / 180 * M_PI) * dist;

    float newX = x0 + newDeltaX;
    float newY = y0 - newDeltaY;

    return pair<float, float>(newX,newY);
}

int myProc(int a, int b)
{
    return (a + b) % b;
}

