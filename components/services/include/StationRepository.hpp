#pragma once

#include <vector>

#include "IStationRepository.hpp"
#include "UiTypes.hpp"

namespace services {

class StationRepository : public IStationRepository {
   public:
    explicit StationRepository();
    ~StationRepository() override = default;

    bool init() override;

    const std::vector<common::StationData> &getStations() const override;

   private:
    std::vector<common::StationData> mStations;
    bool mInitialized;
};

}  // namespace services
