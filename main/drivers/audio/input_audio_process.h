#pragma once

#include "audio_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <esp_afe_sr_iface.h>
#include <functional>
#include <type_def.h>

class InputAudioProcess
{
private:
    EventGroupHandle_t event_group_;
    esp_afe_sr_iface_t* afe_iface_handle_ = nullptr;
    esp_afe_sr_data_t* afe_sr_data_ = nullptr;
    void input_audio_process_task();
    std::function<void(std::vector<int16_t>&& data)>data_output_callback_;

    FuncVoid vad_callback_= nullptr;
    bool is_speech=false;
    
public:
    inline void set_vad_callback(FuncVoid callback)
    {
        vad_callback_= callback;
    }

    inline void set_data_output_callback(std::function<void(std::vector<int16_t>&& data)> callback){
        data_output_callback_= callback;
    }
    
    InputAudioProcess(AudioHAL* audio_hal);
    ~InputAudioProcess();
    void stop();
    void start();
    bool is_running();
    void feed(const std::vector<int16_t>& pcm);
    size_t get_feed_size(int channel_num);

    bool get_vadspeek_state();
};



