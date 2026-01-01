#include "StationRepository.hpp"

namespace services {

StationRepository::StationRepository() {}

bool StationRepository::init() { return false; }

std::vector<core::StationData> &StationRepository::load() {

  // TODO: insert return statement here
  static std::vector<core::StationData> dummy;
  dummy.emplace_back(core::StationData{"Station 1", "http://stream1"});
  dummy.emplace_back(core::StationData{"Station 2", "http://stream2"});

  return dummy;
}

bool StationRepository::saveAtomic(std::vector<core::StationData> &stations) {
  return false;
}

} // namespace services