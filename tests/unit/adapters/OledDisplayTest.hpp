#pragma once

#include <gtest/gtest.h>

#include "MockI2cBus.hpp"
#include "OledSsd1306Display.hpp"

class OledDisplayTest : public ::testing::Test {
   protected:
    void SetUp() override;
    void TearDown() override;

    void initDisplay();

    adapters::MockI2cBus mockI2cBus;
    std::unique_ptr<adapters::OledSsd1306Display> display;
};
