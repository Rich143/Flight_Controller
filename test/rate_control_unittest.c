#include "rate_control.h"
#include "gtest/gtest.h"

#define limitCheck(roll, pitch, yaw) \
    EXPECT_LE((roll), ROTATION_AXIS_OUTPUT_MAX); \
    EXPECT_LE((pitch), ROTATION_AXIS_OUTPUT_MAX); \
    EXPECT_LE((yaw), ROTATION_AXIS_OUTPUT_MAX); \
    EXPECT_GE((roll), ROTATION_AXIS_OUTPUT_MIN); \
    EXPECT_GE((pitch), ROTATION_AXIS_OUTPUT_MIN); \
    EXPECT_GE((yaw), ROTATION_AXIS_OUTPUT_MIN);

class RateControlTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            desiredRates.roll = 0;
            desiredRates.pitch = 0;
            desiredRates.yaw = 0;

            actualRates.roll = 0;
            actualRates.pitch = 0;
            actualRates.yaw = 0;

            resetRateInfo();
        }

        Rates_t actualRates;
        Rates_t desiredRates;
};

TEST_F(RateControlTest, rollPositive)
{
    desiredRates.roll = 50;
    desiredRates.pitch = 0;
    desiredRates.yaw = 0;

    actualRates.roll = 0;
    actualRates.pitch = 0;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_GT(outputs->roll, 0);
    EXPECT_EQ(outputs->pitch, 0);
    EXPECT_EQ(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, rollNegative)
{
    desiredRates.roll = -50;
    desiredRates.pitch = 0;
    desiredRates.yaw = 0;

    actualRates.roll = 0;
    actualRates.pitch = 0;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_LT(outputs->roll, 0);
    EXPECT_EQ(outputs->pitch, 0);
    EXPECT_EQ(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, pitchPositive)
{
    desiredRates.roll = 0;
    desiredRates.pitch = 50;
    desiredRates.yaw = 0;

    actualRates.roll = 0;
    actualRates.pitch = 0;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_EQ(outputs->roll, 0);
    EXPECT_GT(outputs->pitch, 0);
    EXPECT_EQ(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, pitchNegative)
{
    desiredRates.roll = 0;
    desiredRates.pitch = -50;
    desiredRates.yaw = 0;

    actualRates.roll = 0;
    actualRates.pitch = 0;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_EQ(outputs->roll, 0);
    EXPECT_LT(outputs->pitch, 0);
    EXPECT_EQ(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, yawPositive)
{
    desiredRates.roll = 0;
    desiredRates.pitch = 0;
    desiredRates.yaw = 50;

    actualRates.roll = 0;
    actualRates.pitch = 0;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_EQ(outputs->roll, 0);
    EXPECT_EQ(outputs->pitch, 0);
    EXPECT_GT(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, yawNegative)
{
    desiredRates.roll = 0;
    desiredRates.pitch = 0;
    desiredRates.yaw = -50;

    actualRates.roll = 0;
    actualRates.pitch = 0;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_EQ(outputs->roll, 0);
    EXPECT_EQ(outputs->pitch, 0);
    EXPECT_LT(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, rollPitchYawCombo)
{
    desiredRates.roll = 50;
    desiredRates.pitch = 50;
    desiredRates.yaw = -50;

    actualRates.roll = 100;
    actualRates.pitch = -100;
    actualRates.yaw = 0;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_LT(outputs->roll, 0);
    EXPECT_GT(outputs->pitch, 0);
    EXPECT_LT(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, limitsPositive)
{
    desiredRates.roll = RATES_MAX;
    desiredRates.pitch = RATES_MAX;
    desiredRates.yaw = RATES_MAX;

    actualRates.roll = RATES_MIN;
    actualRates.pitch = RATES_MIN;
    actualRates.yaw = RATES_MIN;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_GT(outputs->roll, 0);
    EXPECT_GT(outputs->pitch, 0);
    EXPECT_GT(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}

TEST_F(RateControlTest, limitsNegative)
{
    desiredRates.roll = RATES_MIN;
    desiredRates.pitch = RATES_MIN;
    desiredRates.yaw = RATES_MIN;

    actualRates.roll = RATES_MAX;
    actualRates.pitch = RATES_MAX;
    actualRates.yaw = RATES_MAX;

    RotationAxisOutputs_t *outputs = controlRates(&actualRates, &desiredRates);

    EXPECT_LT(outputs->roll, 0);
    EXPECT_LT(outputs->pitch, 0);
    EXPECT_LT(outputs->yaw, 0);

    limitCheck(outputs->roll, outputs->pitch, outputs->yaw);
}
