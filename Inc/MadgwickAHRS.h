//=============================================================================================
// MadgwickAHRS.h
//=============================================================================================
//
// Implementation of Madgwick's IMU and AHRS algorithms.
// See: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
//
// From the x-io website "Open-source resources available on this website are
// provided under the GNU General Public Licence unless an alternative licence
// is provided in source."
//
// Date			Author          Notes
// 29/09/2011	SOH Madgwick    Initial release
// 02/10/2011	SOH Madgwick	Optimised for reduced CPU load
//
//=============================================================================================
#ifndef MadgwickAHRS_h
#define MadgwickAHRS_h
#include <math.h>

void madgwickInit(float sampleFrequency);
void madgwickUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void madgwickUpdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

float getRoll();
float getPitch();
float getYaw();
float getRollRadians();
float getPitchRadians();
float getYawRadians();

#endif

