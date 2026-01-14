#pragma once

#include "gtest/gtest.h"
#include "MockDisplay.hpp"
#include "MockStationRepository.hpp"
#include "UiService.hpp"

class UiServiceTest : public ::testing::Test {
   protected:
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<adapters::MockDisplay> mockDisplay;
    std::unique_ptr<services::MockStationRepository> mockRepo;

    std::unique_ptr<services::UiService> uiService;
};
