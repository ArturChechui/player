#include "UiService.hpp"

#include <esp_log.h>

#include <algorithm>
#include <array>

#include "IDisplay.hpp"
#include "StationRepository.hpp"
#include "UiTypes.hpp"

namespace services {
static constexpr uint8_t WIDTH = 128U;      // pixels
static constexpr uint8_t HEIGHT = 64U;      // pixels
static constexpr uint8_t PAGE_HEIGHT = 8U;  // pixels
// Only works when PAGE_HEIGHT is a power of 2 (2, 4, 8, 16, etc.).
// used as modulo but faster
static constexpr uint8_t PAGE_BIT_MASK = PAGE_HEIGHT - 1U;              // 0x07
static constexpr uint8_t PAGES = HEIGHT / PAGE_HEIGHT;                  // number of pages
static constexpr uint8_t GLYPH_WIDTH = 5U;                              // pixels
static constexpr uint8_t GLYPH_HEIGHT = 7U;                             // pixels
static constexpr uint8_t CHAR_WIDTH = GLYPH_WIDTH + 1U;                 // 5px glyph + 1px spacing
static constexpr uint8_t CHAR_HEIGHT = GLYPH_HEIGHT + 1U;               // 7px glyph + 1px spacing
static constexpr uint8_t STATUS_BAR_AREA_END = 8U;                      // pixels
static constexpr uint8_t STATION_NAME_START_X = 6U;                     // pixels
static constexpr uint8_t MAX_STATIONS = 6U;                             // max stations to show
static constexpr uint8_t MAX_STATION_NAME = (WIDTH / CHAR_WIDTH) - 1U;  // -1 because of icon
static constexpr uint8_t SPACE_BYTE = 0x00;                             // empty byte for spacing

static const char *TAG = "UiService";

UiService::UiService(adapters::IDisplay &display, IStationRepository &stationRepo)
    : mDisplay(display), mStationRepo(stationRepo), mFramebuffer(WIDTH * PAGES, 0) {
    ESP_LOGI(TAG, "Creating UiService");
}

bool UiService::init() {
    ESP_LOGI(TAG, "Initializing UiService");

    clearFramebuffer();
    flushFramebuffer();

    return true;
}

void UiService::onEvent(const common::UiEvent &e) {
    switch (e.type) {
        case common::UiEvent::Type::RENDER_BOOT:
            ESP_LOGI(TAG, "Rendering boot screen");
            renderBoot();
            break;
        case common::UiEvent::Type::RENDER_STATIONS:
            ESP_LOGI(TAG, "Rendering stations");
            renderStations(e.selectedIndex);
            break;
        case common::UiEvent::Type::RENDER_STATUS:
            ESP_LOGI(TAG, "Rendering UI status");
            renderStatus();
            break;
        default:
            ESP_LOGW(TAG, "Unknown UI event type");
            break;
    }
}

void UiService::renderBoot() {
    // TODO: implement
}

void UiService::renderStatus() {
    // TODO: implement
    // TODO: come up with status bar areas for each icon
    // switch case for each icon to draw at correct position? and argument only
    // the icon type?
    // drawChar(x, y, static_cast<char>(icon));
    // playback status - play stop near the active station
}

void UiService::renderStations(int selectedIndex) {
    ESP_LOGI(TAG, "Rendering stations, selected index: %d", selectedIndex);

    const auto &stations = mStationRepo.getStations();
    for (int i = 0; i < static_cast<int>(stations.size()) && i < MAX_STATIONS; ++i) {
        const uint8_t startY = (STATUS_BAR_AREA_END + (i * 8));

        std::string name = stations[i].name;
        if (name.size() > MAX_STATION_NAME) {
            name.resize(MAX_STATION_NAME);
        }

        drawText(STATION_NAME_START_X, startY, name);
    }

    flushFramebuffer();
}

void UiService::clearFramebuffer() {
    std::fill(mFramebuffer.begin(), mFramebuffer.end(), 0x00);
}

void UiService::flushFramebuffer() {
    mDisplay.showFramebuffer(mFramebuffer.data(), mFramebuffer.size());
}

void UiService::drawText(uint8_t x, uint8_t y, const std::string_view &txt) {
    if ((y + CHAR_HEIGHT) > HEIGHT) {
        ESP_LOGE(TAG, "drawText: Y coordinate out of bounds: %u", y);
        return;
    }

    uint8_t currX = x;

    for (char ch : txt) {
        if ((currX + CHAR_WIDTH) > WIDTH) {
            ESP_LOGE(TAG, "drawText: X coordinate out of bounds: %u", currX);
            break;
        }

        drawChar(currX, y, ch);
        currX += CHAR_WIDTH;
    }
}

void UiService::drawChar(uint8_t x, uint8_t y, char c) {
    if (x >= WIDTH || y >= HEIGHT) {
        ESP_LOGE(TAG, "drawChar: coordinates out of bounds (%u,%u)", x, y);
        return;
    }

    if ((y & PAGE_BIT_MASK) != 0) {
        ESP_LOGW(TAG, "drawChar: Y coordinate not page-aligned: %u", y);
        return;
    }

    uint8_t idx = static_cast<uint8_t>(c);
    if (idx >= common::FONT5x7.size()) {
        ESP_LOGE(TAG, "Character out of range: %c", c);
        idx = static_cast<uint8_t>('?');
    }

    const auto &glyph = common::FONT5x7[idx];
    const uint16_t pageStartIdx = (y / PAGE_HEIGHT) * WIDTH;
    for (uint8_t col = 0; col < GLYPH_WIDTH; ++col) {
        const uint16_t byteIdx = pageStartIdx + (x + col);

        mFramebuffer[byteIdx] = glyph[col];
    }

    // 1px spacing column after glyph
    const uint16_t spaceByteIdx = pageStartIdx + (x + GLYPH_WIDTH);
    if (spaceByteIdx < mFramebuffer.size()) {
        mFramebuffer[spaceByteIdx] = SPACE_BYTE;
    }
}

}  // namespace services
