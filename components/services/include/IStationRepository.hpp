#pragma once

#include <vector>

#include "UiTypes.hpp"

namespace services {

class IStationRepository {
   public:
    virtual ~IStationRepository() = default;

    virtual bool init() = 0;
    virtual const std::vector<common::StationData> &getStations() const = 0;
};

}  // namespace services
