#include "gtest/gtest.h"
#include "fff.h"

extern "C" {
#include "fc.h"
#include "imu.h"
#include "ImuRegisters.h"
}

FAKE_VALUE_FUNC(FC_Status, AccelGyro_RegWrite, uint8_t, uint8_t);
FAKE_VALUE_FUNC(FC_Status, AccelGyro_RegRead, uint8_t, uint8_t*, int);

int AccelGyroRegVal[6];

FC_Status AccelGyro_RegRead_custom_fake(uint8_t regAddress, uint8_t *val, int size)
{
    regAddress++; // To avoid warnings for unused variable

    for (int i = 0; i < size; i++) {
        val[i] = AccelGyroRegVal[i];
    }
    return FC_OK;
}

FC_Status AccelGyro_RegWrite_custom_fake(uint8_t regAddress, uint8_t val)
{
    regAddress++; // To avoid warnings for unused variable
    val++;
    return FC_OK;
}

class AccelTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            RESET_FAKE(AccelGyro_RegRead);
            RESET_FAKE(AccelGyro_RegWrite);
            FFF_RESET_HISTORY();

            AccelGyro_RegRead_fake.custom_fake = AccelGyro_RegRead_custom_fake;
            AccelGyro_RegWrite_fake.custom_fake = AccelGyro_RegWrite_custom_fake;
        }

};

class GyroTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            RESET_FAKE(AccelGyro_RegRead);
            RESET_FAKE(AccelGyro_RegWrite);
            FFF_RESET_HISTORY();

            AccelGyro_RegRead_fake.custom_fake = AccelGyro_RegRead_custom_fake;
            AccelGyro_RegWrite_fake.custom_fake = AccelGyro_RegWrite_custom_fake;
        }

};

TEST_F(AccelTest, Test610mG)
{
    // 610mg 
    // mg = raw * sensitivity = raw * 0.061 (for 2g sensitivity)
    // raw = 610/0.061 = 10000
    // raw = 0b0010 0111 0001 0000
    AccelGyroRegVal[0] = 0b00010000;
    AccelGyroRegVal[1] = 0b00100111;
    AccelGyroRegVal[2] = 0b00010000;
    AccelGyroRegVal[3] = 0b00100111;
    AccelGyroRegVal[4] = 0b00010000;
    AccelGyroRegVal[5] = 0b00100111;

    Accel_t accel;

    EXPECT_EQ(FC_OK, getAccel(&accel));
    EXPECT_EQ(OUT_X_L_XL, AccelGyro_RegRead_fake.arg0_history[0]);
    EXPECT_EQ(610, accel.x);
    EXPECT_EQ(610, accel.y);
    EXPECT_EQ(610, accel.z);
}

TEST_F(AccelTest, TestNeg610mG)
{
    // 610mg 
    // mg = raw * sensitivity = raw * 0.061 (for 2g sensitivity)
    // raw = -610/0.061 = -10000
    // raw = 1101 1000 1111 0000
    AccelGyroRegVal[0] = 0b11110000;
    AccelGyroRegVal[1] = 0b11011000;
    AccelGyroRegVal[2] = 0b11110000;
    AccelGyroRegVal[3] = 0b11011000;
    AccelGyroRegVal[4] = 0b11110000;
    AccelGyroRegVal[5] = 0b11011000;

    Accel_t accel;

    EXPECT_EQ(FC_OK, getAccel(&accel));
    EXPECT_EQ(OUT_X_L_XL, AccelGyro_RegRead_fake.arg0_history[0]);
    EXPECT_EQ(-610, accel.x);
    EXPECT_EQ(-610, accel.y);
    EXPECT_EQ(-610, accel.z);
}

TEST_F(GyroTest, Test1400dps)
{
    // 1400dps = 1400 000mdps
    // dps = raw * sensitivity = raw * 70 (for 2000dps sensitivity)
    // raw = 1400000/70 = 20000
    // raw = 0b0100 1110 0010 0000
    AccelGyroRegVal[0] = 0b00100000;
    AccelGyroRegVal[1] = 0b01001110;
    AccelGyroRegVal[2] = 0b00100000;
    AccelGyroRegVal[3] = 0b01001110;
    AccelGyroRegVal[4] = 0b00100000;
    AccelGyroRegVal[5] = 0b01001110;

    Gyro_t gyro;

    EXPECT_EQ(FC_OK, getGyro(&gyro));
    EXPECT_EQ(OUT_X_L_G, AccelGyro_RegRead_fake.arg0_history[0]);
    EXPECT_EQ(1400000, gyro.x);
    EXPECT_EQ(1400000, gyro.y);
    EXPECT_EQ(1400000, gyro.z);
}

TEST_F(GyroTest, TestNeg1400dps)
{
    // -1400dps = -1400 000mdps
    // dps = raw * sensitivity = raw * 70 (for 2000dps sensitivity)
    // raw = -1400000/70 = -20000
    // raw = 0b10110001 11100000
    AccelGyroRegVal[0] = 0b11100000;
    AccelGyroRegVal[1] = 0b10110001;
    AccelGyroRegVal[2] = 0b11100000;
    AccelGyroRegVal[3] = 0b10110001;
    AccelGyroRegVal[4] = 0b11100000;
    AccelGyroRegVal[5] = 0b10110001;

    Gyro_t gyro;

    EXPECT_EQ(FC_OK, getGyro(&gyro));
    EXPECT_EQ(OUT_X_L_G, AccelGyro_RegRead_fake.arg0_history[0]);
    EXPECT_EQ(-1400000, gyro.x);
    EXPECT_EQ(-1400000, gyro.y);
    EXPECT_EQ(-1400000, gyro.z);
}
