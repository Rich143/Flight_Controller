#include <math.h>
#include "fc.h"
#include "calculateAttitude.h"

#define sq(x) ((x)*(x))
FC_Status calculateAttitude(Accel_t *accel, Attitude_t *attitudeOut)
{
    if (accel == NULL) {
        return FC_ERROR;
    }

    float roll  = (atan2((float)-accel->y, (float)accel->z)*180.0)/M_PI;
    float tmpSqrt = sqrt(sq((float)(accel->y)) + sq((float)(accel->z)));
    float pitch = (atan2(((double)accel->x), tmpSqrt)*180.0)/M_PI;
    attitudeOut->roll = roll * 100;
    attitudeOut->pitch = pitch * 100;
    
    return FC_OK;
}
