#pragma once

#include "core/include/types.hpp"

namespace services {

class StationRepository {
public:
  explicit StationRepository();
  bool init();

  std::vector<core::StationData> &load();
  bool saveAtomic(std::vector<core::StationData> &stations);
};

} // namespace services
