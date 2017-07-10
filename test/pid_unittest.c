#include "pid.h"
#include "gtest/gtest.h"

class PIDTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            controlInfo.K_P = 0.1;
            controlInfo.K_I = 0.01;
            controlInfo.K_D = 2;
            controlInfo.maxValue = 100;
            controlInfo.minValue = -100;
            controlInfo.dt = 1;

            controlInfo.integratedError = 0;
            controlInfo.saturated = 0;
            controlInfo.lastError = 0;
        }

        ControlInfo_t controlInfo;
};

TEST_F(PIDTest, zeroErrorZeroOutInitially)
{
   EXPECT_EQ(0, controlLoop(0, &controlInfo));
}

TEST_F(PIDTest, posErrorPosOut)
{
    EXPECT_GT(controlLoop(10000, &controlInfo), 0);
}

TEST_F(PIDTest, negErrorNegOut)
{
    EXPECT_LT(controlLoop(-10000, &controlInfo), 0);
}

TEST_F(PIDTest, limits)
{
    int errorMax = (controlInfo.maxValue + 1) / controlInfo.K_P;
    int errorMin = (controlInfo.minValue - 1) / controlInfo.K_P;

    EXPECT_EQ(controlInfo.maxValue, controlLoop(errorMax, &controlInfo));
    EXPECT_EQ(controlInfo.minValue, controlLoop(errorMin, &controlInfo));
}

TEST_F(PIDTest, outputIncreasesWithTime)
{
    int posError = controlInfo.maxValue / controlInfo.K_P / 6;
    // Set the last error to the same as the current to ensure the derivative term doesn't cause the output to saturate
    controlInfo.lastError = posError;

    int firstOutput = controlLoop(posError, &controlInfo);
    int secondOutput = controlLoop(posError, &controlInfo);

    EXPECT_GT(secondOutput, firstOutput);
}

TEST_F(PIDTest, outputDecreasesWithTime)
{
    int negError = controlInfo.minValue / controlInfo.K_P / 6;
    // Set the last error to the same as the current to ensure the derivative term doesn't cause the output to saturate
    controlInfo.lastError = negError;

    int firstOutput = controlLoop(negError, &controlInfo);
    int secondOutput = controlLoop(negError, &controlInfo);

    EXPECT_LT(secondOutput, firstOutput);
}


TEST_F(PIDTest, integratedErrorSaturates)
{
    int error = controlInfo.maxValue / controlInfo.K_I / controlInfo.dt -1;

    controlLoop(error, &controlInfo);
    int firstError = controlInfo.integratedError;

    controlLoop(error, &controlInfo);
    int secondError = controlInfo.integratedError;

    controlLoop(error, &controlInfo);
    int thirdError = controlInfo.integratedError;

    EXPECT_GT(secondError, firstError);
    EXPECT_EQ(secondError, thirdError);
}

TEST_F(PIDTest, derivativeTest)
{
    controlInfo.K_I = 0; // remove integrator so it doesn't influence the results

    int error = 50;
    int errorChange = 10;

    controlInfo.lastError = error;

    // Change slowly
    controlLoop(error, &controlInfo);
    controlLoop(error + (errorChange/2), &controlInfo);
    int slowChangeOutput = controlLoop(error + errorChange, &controlInfo);

    // Change Rapidly
    controlLoop(error, &controlInfo);
    int fastChangeOutput = controlLoop(error + errorChange, &controlInfo);

    EXPECT_GT(fastChangeOutput, slowChangeOutput);
}
