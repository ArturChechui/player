#pragma once

#include "AudioTypes.hpp"
#include "II2sBus.hpp"
#include "IPlayer.hpp"

namespace adapters {
class EspAdfPlayer : public IPlayer {
   public:
    EspAdfPlayer(II2sBus& i2sBus);  // Inject I2S adapter (to be created)
    bool init() override;
    void deinit() override;
    bool play(const std::string& url) override;
    bool stop() override;
    void setStatusCallback(common::PlayerStatusCallback cb) override;

   private:
    II2sBus& mI2sBus;
    audio_pipeline_handle_t mPipeline = nullptr;
    audio_element_handle_t mHttpStream = nullptr;
    audio_element_handle_t mDecoder = nullptr;
    audio_element_handle_t mI2sStream = nullptr;
    common::PlayerStatusCallback mStatusCb;
    bool mIsPlaying = false;

    static void handleAdfEvent(audio_event_iface_msg_t* msg, void* userData);
};
}  // namespace adapters
