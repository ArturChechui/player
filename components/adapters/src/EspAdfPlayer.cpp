#include "EspAdfPlayer.hpp"

#include <audio_element.h>
#include <audio_pipeline.h>
#include <esp_log.h>
#include <http_stream.h>
#include <i2s_stream.h>
#include <mp3_decoder.h>

namespace adapters {

static constexpr const char* TAG = "EspAdfPlayer";

EspAdfPlayer::EspAdfPlayer(II2sBus& i2sBus) : mI2sBus(i2sBus) {
    ESP_LOGI(TAG, "Creating EspAdfPlayer");
}

bool EspAdfPlayer::init() {
    if (mPipeline != nullptr) {
        ESP_LOGW(TAG, "Player already initialized");
        return true;
    }

    if (!mI2sBus.isAvailable()) {
        ESP_LOGE(TAG, "I2S bus not available");
        return false;
    }

    // Create audio pipeline
    audio_pipeline_cfg_t pipelineCfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    mPipeline = audio_pipeline_init(&pipelineCfg);
    if (mPipeline == nullptr) {
        ESP_LOGE(TAG, "Failed to create audio pipeline");
        return false;
    }

    // HTTP stream element
    http_stream_cfg_t httpCfg = HTTP_STREAM_CFG_DEFAULT();
    httpCfg.event_handle = nullptr;  // Will use pipeline events
    httpCfg.type = AUDIO_STREAM_READER;
    httpCfg.enable_playlist_parser = false;
    mHttpStream = http_stream_init(&httpCfg);
    if (mHttpStream == nullptr) {
        ESP_LOGE(TAG, "Failed to create HTTP stream");
        deinit();
        return false;
    }

    // MP3 decoder element
    mp3_decoder_cfg_t mp3Cfg = DEFAULT_MP3_DECODER_CONFIG();
    mDecoder = mp3_decoder_init(&mp3Cfg);
    if (mDecoder == nullptr) {
        ESP_LOGE(TAG, "Failed to create MP3 decoder");
        deinit();
        return false;
    }

    // I2S stream element (output to speaker)
    i2s_stream_cfg_t i2sCfg = I2S_STREAM_CFG_DEFAULT();
    i2sCfg.type = AUDIO_STREAM_WRITER;
    i2sCfg.i2s_port = static_cast<i2s_port_t>(common::I2S_PORT);
    i2sCfg.use_alc = false;
    i2sCfg.volume = 0;  // Let PlayerService handle volume
    i2sCfg.out_rb_size = 8 * 1024;
    mI2sStream = i2s_stream_init(&i2sCfg);
    if (mI2sStream == nullptr) {
        ESP_LOGE(TAG, "Failed to create I2S stream");
        deinit();
        return false;
    }

    // Register elements with pipeline
    if (audio_pipeline_register(mPipeline, mHttpStream, "http") != ESP_OK ||
        audio_pipeline_register(mPipeline, mDecoder, "mp3") != ESP_OK ||
        audio_pipeline_register(mPipeline, mI2sStream, "i2s") != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register pipeline elements");
        deinit();
        return false;
    }

    // Link: HTTP → MP3 decoder → I2S output
    const char* linkTag[3] = {"http", "mp3", "i2s"};
    if (audio_pipeline_link(mPipeline, linkTag, 3) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to link pipeline elements");
        deinit();
        return false;
    }

    // Set up event listener
    audio_event_iface_cfg_t evtCfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evtCfg);
    if (evt == nullptr) {
        ESP_LOGE(TAG, "Failed to create event interface");
        deinit();
        return false;
    }

    audio_pipeline_set_listener(mPipeline, evt);

    // Note: event handling task would go here if we process ADF events
    // For now, PlayerService will poll status or we add a task in next iteration

    ESP_LOGI(TAG, "Audio pipeline initialized (HTTP → MP3 → I2S)");
    return true;
}

void EspAdfPlayer::deinit() {
    if (mPipeline == nullptr) {
        return;
    }

    ESP_LOGI(TAG, "Deinitializing audio pipeline");

    // Stop and clean up
    audio_pipeline_stop(mPipeline);
    audio_pipeline_wait_for_stop(mPipeline);
    audio_pipeline_terminate(mPipeline);

    audio_pipeline_unregister(mPipeline, mHttpStream);
    audio_pipeline_unregister(mPipeline, mDecoder);
    audio_pipeline_unregister(mPipeline, mI2sStream);

    audio_element_deinit(mHttpStream);
    audio_element_deinit(mDecoder);
    audio_element_deinit(mI2sStream);
    audio_pipeline_deinit(mPipeline);

    mHttpStream = nullptr;
    mDecoder = nullptr;
    mI2sStream = nullptr;
    mPipeline = nullptr;
    mIsPlaying = false;
}

bool EspAdfPlayer::play(const std::string& url) {
    if (mPipeline == nullptr) {
        ESP_LOGE(TAG, "Pipeline not initialized");
        return false;
    }

    if (mIsPlaying) {
        ESP_LOGW(TAG, "Already playing, ignoring play request");
        return false;
    }

    if (url.empty()) {
        ESP_LOGE(TAG, "Empty URL provided");
        return false;
    }

    ESP_LOGI(TAG, "Starting playback: %s", url.c_str());

    // Set HTTP URI
    if (audio_element_set_uri(mHttpStream, url.c_str()) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set HTTP URI");
        return false;
    }

    // Reset pipeline state
    audio_pipeline_reset_ringbuffer(mPipeline);
    audio_pipeline_reset_elements(mPipeline);
    audio_pipeline_change_state(mPipeline, AEL_STATE_INIT);

    // Start playback
    if (audio_pipeline_run(mPipeline) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start pipeline");
        return false;
    }

    mIsPlaying = true;

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::BUFFERING);
    }

    return true;
}

bool EspAdfPlayer::stop() {
    if (mPipeline == nullptr) {
        ESP_LOGE(TAG, "Pipeline not initialized");
        return false;
    }

    if (!mIsPlaying) {
        ESP_LOGW(TAG, "Not playing, ignoring stop request");
        return true;  // Not an error
    }

    ESP_LOGI(TAG, "Stopping playback");

    if (audio_pipeline_stop(mPipeline) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop pipeline");
        return false;
    }

    audio_pipeline_wait_for_stop(mPipeline);
    audio_pipeline_terminate(mPipeline);

    mIsPlaying = false;

    if (mStatusCb) {
        mStatusCb(common::PlayerStatus::STOPPED);
    }

    return true;
}

void EspAdfPlayer::setStatusCallback(common::PlayerStatusCallback cb) {
    mStatusCb = cb;
}

void EspAdfPlayer::handleAdfEvent(audio_event_iface_msg_t* msg, void* userData) {
    // Event handling placeholder for step 2
    // Will parse ADF_MUSIC_INFO_* events and call mStatusCb
    auto* player = static_cast<EspAdfPlayer*>(userData);
    if (player == nullptr || msg == nullptr) {
        return;
    }

    // TODO: Parse msg->source_type, msg->cmd and dispatch to PlayerService via callback
    // Example: AEL_MSG_CMD_REPORT_STATUS → check state → BUFFERING/PLAYING/ERROR
}

}  // namespace adapters