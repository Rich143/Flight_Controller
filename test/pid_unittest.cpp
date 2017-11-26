#include "gtest/gtest.h"

extern "C" {
#include "pid.h"
}

class PIDTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            gains.K_P = 0.1;
            gains.K_I = 0.01;
            gains.K_D = 2;

            limits.max = 100;
            limits.min = -100;

            controlInfo.dt = 1;
            controlInfo.integratedError = 0;
            controlInfo.saturated = 0;
            controlInfo.lastError = 0;
        }

        ControlInfo_t controlInfo;
        Limits_t limits;
        PID_Gains_t gains;
        PidValues_t PIDs;
};

TEST_F(PIDTest, zeroErrorZeroOutInitially)
{
   EXPECT_EQ(0, controlLoop(0, &controlInfo, &gains, &limits, &PIDs));
}

TEST_F(PIDTest, posErrorPosOut)
{
    EXPECT_GT(controlLoop(10000, &controlInfo, &gains, &limits, &PIDs), 0);
}

TEST_F(PIDTest, negErrorNegOut)
{
    EXPECT_LT(controlLoop(-10000, &controlInfo, &gains, &limits, &PIDs), 0);
}

TEST_F(PIDTest, limits)
{
    int errorMax = (limits.max + 1) / gains.K_P;
    int errorMin = (limits.min - 1) / gains.K_P;

    EXPECT_EQ(limits.max, controlLoop(errorMax, &controlInfo, &gains, &limits, &PIDs));
    EXPECT_EQ(limits.min, controlLoop(errorMin, &controlInfo, &gains, &limits, &PIDs));
}

TEST_F(PIDTest, outputIncreasesWithTime)
{
    int posError = limits.max / gains.K_P / 6;
    // Set the last error to the same as the current to ensure the derivative term doesn't cause the output to saturate
    controlInfo.lastError = posError;

    int firstOutput = controlLoop(posError, &controlInfo, &gains, &limits, &PIDs);
    int secondOutput = controlLoop(posError, &controlInfo, &gains, &limits, &PIDs);

    EXPECT_GT(secondOutput, firstOutput);
}

TEST_F(PIDTest, outputDecreasesWithTime)
{
    int negError = limits.min / gains.K_P / 6;
    // Set the last error to the same as the current to ensure the derivative term doesn't cause the output to saturate
    controlInfo.lastError = negError;

    int firstOutput = controlLoop(negError, &controlInfo, &gains, &limits, &PIDs);
    int secondOutput = controlLoop(negError, &controlInfo, &gains, &limits, &PIDs);

    EXPECT_LT(secondOutput, firstOutput);
}


TEST_F(PIDTest, integratedErrorSaturates)
{
    int error = limits.max / gains.K_I / controlInfo.dt -1;

    controlLoop(error, &controlInfo, &gains, &limits, &PIDs);
    int firstError = controlInfo.integratedError;

    controlLoop(error, &controlInfo, &gains, &limits, &PIDs);
    int secondError = controlInfo.integratedError;

    controlLoop(error, &controlInfo, &gains, &limits, &PIDs);
    int thirdError = controlInfo.integratedError;

    EXPECT_GT(secondError, firstError);
    EXPECT_EQ(secondError, thirdError);
}

TEST_F(PIDTest, derivativeTest)
{
    gains.K_I = 0; // remove integrator so it doesn't influence the results

    int error = 50;
    int errorChange = 10;

    controlInfo.lastError = error;

    // Change slowly
    controlLoop(error, &controlInfo, &gains, &limits, &PIDs);
    controlLoop(error + (errorChange/2), &controlInfo, &gains, &limits, &PIDs);
    int slowChangeOutput = controlLoop(error + errorChange, &controlInfo, &gains, &limits, &PIDs);

    // Change Rapidly
    controlLoop(error, &controlInfo, &gains, &limits, &PIDs);
    int fastChangeOutput = controlLoop(error + errorChange, &controlInfo, &gains, &limits, &PIDs);

    EXPECT_GT(fastChangeOutput, slowChangeOutput);
}
