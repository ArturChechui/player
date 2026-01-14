#include "StationRepositoryTest.hpp"

void StationRepositoryTest::SetUp() {
    stationRepository = std::make_unique<services::StationRepository>();
}

void StationRepositoryTest::TearDown() {
    stationRepository.reset();
}

TEST_F(StationRepositoryTest, getStations_ReturnsInitializedStations) {
    // Arrange
    std::vector<common::StationData> expectedStations = {
        {"radio1_aac_h", "Radio 1 (AAC High)",
         "https://playerservices.streamtheworld.com/api/livestream-redirect/"
         "RADIO_1AAC_H.aac"},
        {"radio1_aac_m", "Radio 1 (AAC Med)",
         "https://playerservices.streamtheworld.com/api/livestream-redirect/"
         "RADIO_1AAC_M.aac"},
        {"example_mp3", "Example MP3 Station", "http://example.com/stream.mp3"}};

    // Act
    stationRepository->init();
    const auto& stations = stationRepository->getStations();

    // Expect
    // TODO: create operator== for StationData and use EXPECT_EQ directly
    EXPECT_EQ(expectedStations.size(), stations.size());
}
