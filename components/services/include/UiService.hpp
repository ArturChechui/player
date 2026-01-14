#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace common {
struct UiEvent;
}  // namespace common

namespace adapters {
class IDisplay;
}  // namespace adapters

namespace services {
class IStationRepository;

class UiService {
   public:
    explicit UiService(adapters::IDisplay &display, IStationRepository &stationRepo);
    bool init();
    void onEvent(const common::UiEvent &e);

#ifdef UNIT_TESTS
    const std::vector<uint8_t> &getFramebuffer() {
        return mFramebuffer;
    }
#endif
   private:
    void renderBoot();
    void renderStatus();
    void renderStations(int selectedIndex);

    void clearFramebuffer();
    void flushFramebuffer();

    void drawText(uint8_t x, uint8_t y, const std::string_view &txt);
    void drawChar(uint8_t x, uint8_t y, char c);
    // DEPRECATED: decide if I need drawing pixel by pixel or Y with page alignment only
    // void drawPixel(uint8_t x, uint8_t y, bool on);

    adapters::IDisplay &mDisplay;
    IStationRepository &mStationRepo;

    std::vector<uint8_t> mFramebuffer;
};

}  // namespace services
