#pragma once

#include <vector>
#include <cstdint>
#include <driver/i2s_types.h>
#include <audio_codec_data_if.h>
#include <audio_codec_ctrl_if.h>
#include <audio_codec_if.h>
#include <audio_codec_gpio_if.h>
#include <esp_codec_dev.h>

class AudioHAL
{
private:

public:
    AudioHAL();
    ~AudioHAL();

    virtual void enable_input() = 0;
    virtual void enable_output() = 0;
    virtual void disable_input() = 0;
    virtual void disable_output() = 0;
    void output_data(std::vector<int16_t>& pcm);
    bool input_data(std::vector<int16_t>& pcm);
    bool is_input_ref();
    
    virtual void set_out_volume(int volume) = 0;

protected:
    virtual void write(const int16_t* data,  int samples) = 0;
    virtual int read(int16_t* data,int samples) = 0;
    i2s_chan_handle_t tx_handle = NULL;
    i2s_chan_handle_t rx_handle = NULL;
    int output_volume = 100;
    int output_sample_rate = 0;
    int input_sample_rate = 0;
    const audio_codec_data_if_t* data_if = nullptr;
    const audio_codec_ctrl_if_t* out_ctrl_if = nullptr;
    const audio_codec_ctrl_if_t* in_ctrl_if= nullptr;
    const audio_codec_if_t* out_codec_if = nullptr;
    const audio_codec_if_t* in_codec_if = nullptr;
    const audio_codec_gpio_if_t* gpio_if =nullptr;
    esp_codec_dev_handle_t audio_output_dev = nullptr;
    esp_codec_dev_handle_t audio_input_dev= nullptr;

    bool enabled_input = false;
    bool enabled_output = false;
    bool input_ref = false;
    
};



