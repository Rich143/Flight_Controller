#include "gtest/gtest.h"

extern "C" {
#include "fake_logic.h"
}

// Tests valid values
TEST(FakeLogicTest, HandlesValidInput) {
    EXPECT_EQ(2, add(1,1));
}

TEST(FakeLogicTest, HandlesZeroInput) {
    EXPECT_EQ(0, add(0,0));
}
