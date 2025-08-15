#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <Cstdio>
#include <esp_err.h>
#include <driver/uart.h>
#include "file_interface.h"
#include "sd_card.h"
#include "work_task.h"
#include "audio_hal.h"
#include <mutex>
#include <opus_decoder.h>
#include "type_def.h"
#include "input_audio_process.h"

/*测试LED*/
#include "driver/gpio.h"


/* 引脚的输出的电平状态 */
enum GPIO_OUTPUT_STATE
{
    PIN_RESET,
    PIN_SET
};
/* LED端口定义 */
#define LED(x)          do { x ?                                      \
                             gpio_set_level(GPIO_NUM_48, PIN_SET) :  \
                             gpio_set_level(GPIO_NUM_48, PIN_RESET); \
                        } while(0)  /* LED翻转 */

/* LED取反定义 */
#define LED_TOGGLE()    do { gpio_set_level(GPIO_NUM_48, !gpio_get_level(GPIO_NUM_48)); } while(0)  /* LED翻转 */

/*测试LED*/



#define OPUS_MODE 0     //0：PCM模式    1：opus

class App       //单例模式
{
private:
    /* data */
    App();
    ~App();
    void print_all_tasks();

    WorkTask *work_task = nullptr;  
    std::list<std::vector<int16_t>> pcm_packets_;  //每个vector,是一个pcm包 录音队列
    std::list<std::vector<int16_t>> pcm_output_packets_;  //每个vector,是一个pcm包 播放队列

    std::unique_ptr<OpusDecoderWrapper> opus_decoder_;  //OPUS

    std::list<std::vector<uint8_t>> opus_queue_;
    std::condition_variable opus_queue_cv_;

    std::mutex pcm_mutex;

    std::unique_ptr<InputAudioProcess> input_audio_process_= nullptr;

    TaskHandle_t audio_task_handle_ = nullptr;
    void audio_task_loop();
    void audio_output_process(AudioHAL* audio_);

    bool get_audio_pcm_resampl(std::vector<int16_t>& pcm_data, int samples, bool ref=true);

    ListFunction task_list;
    std::mutex task_mutex;
    std::condition_variable task_condition_variable_;
    uint32_t task_count;

    void audio_input_process(AudioHAL* audio_);

    void audio_speek_process();
    uint16_t vadspeek_count;

    void init_uart();   //新增串口
    // 串口接收任务函数
    void uart_receive_task();

public:
    static App& GetInstance(){
        static App instance;
        return instance;
    }

    //删除拷贝构造函数和赋值运算符
    App(const App&) = delete;
    App& operator = (const App&) = delete;
    void run();

    void play_p3_audio(const std::string_view& p3_sound_lable);
    void play_number(int number);

    void add_task(FuncVoid task);

};

 
