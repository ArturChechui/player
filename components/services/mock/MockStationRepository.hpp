#pragma once

#include <vector>

#include "IStationRepository.hpp"

namespace common {
struct StationData;
}  // namespace common

#include <gmock/gmock.h>

namespace services {
class MockStationRepository : public IStationRepository {
   public:
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(const std::vector<common::StationData> &, getStations, (), (const, override));
};

}  // namespace services
