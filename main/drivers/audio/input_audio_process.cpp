#include "input_audio_process.h"
#include <string>
#include <esp_log.h>
#include <esp_afe_sr_models.h>
#include <freertos/mpu_wrappers.h>

#define INPUT_RUNNING_EVENT 1<<0

#define TAG "InputAudioProcess"


InputAudioProcess::~InputAudioProcess(){

    if(event_group_!=nullptr)
    {
        vEventGroupDelete(event_group_);
        event_group_=nullptr;
    }
}

InputAudioProcess::InputAudioProcess(AudioHAL* audio_hal){
    event_group_=xEventGroupCreate();
    std::string input_format;
    input_format.push_back('M');
    if (audio_hal->is_input_ref())
    {
        input_format.push_back('R');
    }
    afe_config_t* afe_config =afe_config_init(input_format.c_str(),NULL,AFE_TYPE_VC,AFE_MODE_HIGH_PERF);

    afe_config->aec_init = true;
    afe_config->ns_init = true;              
    afe_config->afe_ns_mode = AFE_NS_MODE_WEBRTC;
    afe_config->vad_init = true;
    afe_config->vad_mode =VAD_MODE_0;
    afe_config->vad_min_noise_ms = 100;
    // afe_config->vad_mode = VAD_MODE_2;
    // afe_config->vad_min_noise_ms = 250;
    // afe_config->vad_min_speech_ms = 150;
    afe_config->afe_perferred_core =1;
    afe_config->afe_perferred_priority = 1;
    afe_config->agc_init= false;
    afe_config->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;
    afe_iface_handle_= esp_afe_handle_from_config(afe_config);
    afe_sr_data_=afe_iface_handle_->create_from_config(afe_config);
    afe_config_free(afe_config);

    xTaskCreate([](void*arg){
        auto this_=(InputAudioProcess*)arg;;
        this_->input_audio_process_task();
        vTaskDelete(NULL);
    }, "input_audio_processtask",4096,this,3, NULL);

}   

void InputAudioProcess::input_audio_process_task()
{
    // static bool is_speech=false;
    while(true)
    {
        xEventGroupWaitBits(event_group_, INPUT_RUNNING_EVENT, pdFALSE, pdTRUE, portMAX_DELAY);

        auto res = afe_iface_handle_->fetch_with_delay(afe_sr_data_, portMAX_DELAY);

        if (res == nullptr || res->ret_value == ESP_FAIL){
            ESP_LOGE(TAG,"fetch error!\n");
            continue;
        }

        if(res->vad_state == VAD_SPEECH){
            is_speech = true;
        }

        // 当检测到静音且之前有语音时，处理累积的音频
        if(res->vad_state == VAD_SILENCE && is_speech){
            is_speech = false;
            if(vad_callback_){
                vad_callback_();
            }
        }

        if (data_output_callback_){
  
            if (!res || !res->data || res->data_size == 0){
                continue;
            }

            const int16_t* data_start = res->data;
            const int16_t* data_end = data_start + (res->data_size / sizeof(int16_t));
            std::vector<int16_t> audio_data(data_start,data_end);

            data_output_callback_(std::move(audio_data));
            
        }
    }
}


void InputAudioProcess::start(){
    xEventGroupSetBits(event_group_,INPUT_RUNNING_EVENT);
}

void InputAudioProcess::stop(){
    xEventGroupClearBits(event_group_, INPUT_RUNNING_EVENT);
}

bool InputAudioProcess::is_running(){
    return xEventGroupGetBits(event_group_)& INPUT_RUNNING_EVENT;
}

void InputAudioProcess::feed(const std::vector<int16_t>& pcm){
    if (afe_sr_data_ == nullptr){
        return;
    }
    afe_iface_handle_->feed(afe_sr_data_,pcm.data());
}

size_t InputAudioProcess::get_feed_size(int channel_num){
    if(afe_sr_data_ == nullptr){
        return 0;
    }
    return afe_iface_handle_->get_feed_chunksize(afe_sr_data_)* channel_num;
}

bool InputAudioProcess::get_vadspeek_state()
{
    return is_speech;
}



