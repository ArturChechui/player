#pragma once

#include <gtest/gtest.h>

#include <memory>

#include "AppController.hpp"
#include "MockUiTask.hpp"

class AppControllerTest : public ::testing::Test {
   protected:
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<core::MockUiTask> mockUiTask;
    std::unique_ptr<core::AppController> appController;
};
