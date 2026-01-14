#pragma once

#include "gtest/gtest.h"
#include "StationRepository.hpp"

class StationRepositoryTest : public ::testing::Test {
   protected:
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<services::StationRepository> stationRepository;
};
