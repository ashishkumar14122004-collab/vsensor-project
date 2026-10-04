/**
 * @file test_alert_manager.cpp
 * @brief Unit tests for AlertManager.
 */

#include <gtest/gtest.h>
#include "AlertManager.hpp"

TEST(AlertManagerTest, DefaultThresholds) {
    AlertManager am;
    EXPECT_FALSE(am.checkTemp(74.9f));
    EXPECT_TRUE(am.checkTemp(75.0f));
    EXPECT_FALSE(am.checkLoad(89.9f));
    EXPECT_TRUE(am.checkLoad(90.0f));
}

TEST(AlertManagerTest, CustomThresholds) {
    AlertManager am(60.0f, 80.0f);
    EXPECT_FALSE(am.checkTemp(59.9f));
    EXPECT_TRUE(am.checkTemp(60.0f));
    EXPECT_TRUE(am.checkTemp(80.0f));
    EXPECT_FALSE(am.checkLoad(79.9f));
    EXPECT_TRUE(am.checkLoad(80.0f));
}

TEST(AlertManagerTest, SettersUpdateThresholds) {
    AlertManager am(75.0f, 90.0f);
    am.setTempThreshold(50.0f);
    am.setLoadThreshold(70.0f);
    EXPECT_TRUE(am.checkTemp(50.0f));
    EXPECT_FALSE(am.checkTemp(49.9f));
    EXPECT_TRUE(am.checkLoad(70.0f));
    EXPECT_FALSE(am.checkLoad(69.9f));
}

TEST(AlertManagerTest, GettersReturnCorrectValues) {
    AlertManager am(65.0f, 85.0f);
    EXPECT_FLOAT_EQ(am.getTempThreshold(), 65.0f);
    EXPECT_FLOAT_EQ(am.getLoadThreshold(), 85.0f);
}
