#include "gtest/gtest.h"
extern "C" {
#include "fc.h"
#include "calculateAttitude.h"
#include "imu.h"
}

TEST(AttitudeTest, testLevel)
{
    Accel_t accel = {0,0,1};
    Attitude_t attitude = {0,0,0};

    EXPECT_EQ(FC_OK, calculateAttitude(&accel, &attitude));
    EXPECT_EQ(0, attitude.roll);
    EXPECT_EQ(0, attitude.pitch);
}

TEST(AttitudeTest, testY)
{
    Accel_t accel = {0,1,0};
    Attitude_t attitude = {0,0,0};

    EXPECT_EQ(FC_OK, calculateAttitude(&accel, &attitude));
    EXPECT_EQ(-9000, attitude.roll);
    EXPECT_EQ(0, attitude.pitch);
}

TEST(AttitudeTest, testYNeg)
{
    Accel_t accel = {0,-1,0};
    Attitude_t attitude = {0,0,0};

    EXPECT_EQ(FC_OK, calculateAttitude(&accel, &attitude));
    EXPECT_EQ(9000, attitude.roll);
    EXPECT_EQ(0, attitude.pitch);
}

TEST(AttitudeTest, testRollLimit)
{
    Accel_t accel = {0,0,-1};
    Attitude_t attitude = {0,0,0};

    EXPECT_EQ(FC_OK, calculateAttitude(&accel, &attitude));
    EXPECT_EQ(18000, attitude.roll);
    EXPECT_EQ(0, attitude.pitch);
}

TEST(AttitudeTest, testX)
{
    Accel_t accel = {1,0,0};
    Attitude_t attitude = {0,0,0};

    EXPECT_EQ(FC_OK, calculateAttitude(&accel, &attitude));
    EXPECT_EQ(0, attitude.roll);
    EXPECT_EQ(9000, attitude.pitch);
}

TEST(AttitudeTest, testXNeg)
{
    Accel_t accel = {-1,0,0};
    Attitude_t attitude = {0,0,0};

    EXPECT_EQ(FC_OK, calculateAttitude(&accel, &attitude));
    EXPECT_EQ(0, attitude.roll);
    EXPECT_EQ(-9000, attitude.pitch);
}
