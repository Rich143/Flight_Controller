#include "gtest/gtest.h"
#include "fc.h"
#include "pressureSensor.h"

int regVal[3];

FC_Status PressureSensor_RegRead(uint8_t regAddress, uint8_t *val, int size)
{
    regAddress++; // To avoid warnings

    for (int i = 0; i < size; i++) {
        val[i] = regVal[i];
    }
    return FC_OK;
}

FC_Status PressureSensor_RegWrite(uint8_t regAddress, uint8_t val)
{
    regAddress++; // To avoid warnings
    val++;
    return FC_OK;
}

TEST(TemperatureTest, Test25C)
{
    // temp = raw / 48 + 425
    // raw = (temp - 425) * 48
    // For 25C = 250 for GetTemp return
    // raw = -8400
    // in twos complement: 1101 1111 0011 0000 
    regVal[0] = 0b00110000;
    regVal[1] = 0b11011111;

    int16_t Temp;

    EXPECT_EQ(FC_OK, pressureSensor_GetTemp(&Temp));
    EXPECT_EQ(250, Temp);
}

TEST(TemperatureTest, TestNeg25C)
{
    // temp = raw / 48 + 425
    // raw = (temp - 425) * 48
    // For -25C = -250 for GetTemp return
    // raw = -32400
    // in twos complement: 1000 0001 0111 0000 
    regVal[0] = 0b01110000;
    regVal[1] = 0b10000001;

    int16_t Temp;

    EXPECT_EQ(FC_OK, pressureSensor_GetTemp(&Temp));
    EXPECT_EQ(-250, Temp);
}

TEST(TemperatureTest, TestZero)
{
    // temp = raw / 48 + 425
    // raw = (temp - 425) * 48
    // For 0C = 0 for GetTemp return
    // raw = -20400
    // in twos complement: 1011 0000 0101 0000
    regVal[0] = 0b01010000;
    regVal[1] = 0b10110000;

    int16_t Temp;

    EXPECT_EQ(FC_OK, pressureSensor_GetTemp(&Temp));
    EXPECT_EQ(0, Temp);
}

TEST(PressureTest, TestSeaLevel)
{
    // pressure = raw*100/4096
    // raw = pressure*4096/100
    // Sea level = 101325 Pa
    // raw = 4150272
    // binary: 00111111 01010100 00000000
    regVal[0] = 0b00000000;
    regVal[1] = 0b01010100;
    regVal[2] = 0b00111111;
    
    int32_t Pout;

    EXPECT_EQ(FC_OK, pressureSensor_GetPressure(&Pout));
    EXPECT_EQ(101325, Pout);
}

TEST(AltitudeTest, TestSeaLevel)
{
    // pressure = raw*100/4096
    // raw = pressure*4096/100
    // Sea level = 101325 Pa
    // raw = 4150272
    // binary: 00111111 01010100 00000000
    regVal[0] = 0b00000000;
    regVal[1] = 0b01010100;
    regVal[2] = 0b00111111;
    
    int32_t altitude;

    EXPECT_EQ(FC_OK, pressureSensor_GetAltitude(&altitude));
    EXPECT_EQ(0, altitude);
}
