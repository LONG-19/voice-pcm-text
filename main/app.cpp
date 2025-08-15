#include "app.h"
#include <esp_log.h>
#include <lwip/def.h>
#include "board.h"
#include "p3.h"

void App::init_uart()
{
    const uart_config_t uart_config = {
        .baud_rate = 921600,  // 或者根据你的需求设置
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(UART_NUM_0, 4096*2, 4096*2, 10, nullptr, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    

    xTaskCreatePinnedToCore([](void*arg){
        App*app = (App*)arg;
        app->uart_receive_task();
        vTaskDelete(NULL);
    }, "audio", 4096 * 6, this, 8, nullptr, 0);


    // esp_log_set_vprintf(nullptr);
}

// CRC8 校验函数
uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
    }
    return crc;
}


// 串口接收任务实现
void App::uart_receive_task() {
    const size_t header_size = 4;
    uint8_t header[header_size];
    
    while(true) {
        // 1. 同步查找帧头
        bool found_header = false;
        while(!found_header) {
            // 读取1字节
            uint8_t byte;
            int len = uart_read_bytes(UART_NUM_0, &byte, 1, pdMS_TO_TICKS(100));
            
            if(len == 1) {
                // 检查是否是帧头第一个字节
                if(byte == 0xAA) {
                    // 尝试读取第二个字节
                    len = uart_read_bytes(UART_NUM_0, &byte, 1, pdMS_TO_TICKS(10));
                    if(len == 1 && byte == 0x55) {
                        header[0] = 0xAA;
                        header[1] = 0x55;
                        found_header = true;
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        // 2. 读取剩余头部
        int len = uart_read_bytes(UART_NUM_0, header + 2, 2, pdMS_TO_TICKS(50));
        if(len != 2) {
            continue; // 重新同步
        }
        
        // 3. 解析数据长度
        uint16_t data_len = (header[2] << 8) | header[3];
        if(data_len == 0 || data_len > 4096) {
            continue; // 无效长度
        }

        // 4. 读取PCM数据
        std::vector<uint8_t> pcm_data(data_len);
        len = uart_read_bytes(UART_NUM_0, pcm_data.data(), data_len, pdMS_TO_TICKS(200));
        if(len != data_len) {
            continue; // 数据不完整
        }
        
        // 5. 读取CRC
        uint8_t received_crc;
        len = uart_read_bytes(UART_NUM_0, &received_crc, 1, pdMS_TO_TICKS(50));
        if(len != 1) {
            continue;
        }
        
        // 6. 校验CRC
        uint8_t calculated_crc = crc8(pcm_data.data(), data_len);
        if(received_crc != calculated_crc) {
            ESP_LOGE("UART", "CRC mismatch: recv 0x%02X, calc 0x%02X", 
                    received_crc, calculated_crc);
            continue;
        }
        
        // 7. 转换并处理PCM数据
        size_t sample_count = data_len / sizeof(int16_t);
        if(sample_count > 0) {
            std::vector<int16_t> pcm(sample_count);
            memcpy(pcm.data(), pcm_data.data(), data_len);
            
            // 添加到任务队列处理
            this->add_task([this, pcm = std::move(pcm)]() mutable {
                std::lock_guard<std::mutex> lock(pcm_mutex);
                // PCM模式处理
                pcm_output_packets_.emplace_back(std::move(pcm));
                
                // 如果音频输入处理没有运行，启动它
                if(!input_audio_process_->is_running()) {
                    input_audio_process_->start();
                }
            });
        }
    }
}


void App::print_all_tasks(){
    char task_list_buffer[1024];//缓冲区大小建议>=512字节
    vTaskList(task_list_buffer);//获取任务信息
    // printf("Task List:\n%s\n",task_list_buffer);
}

App::App(/* args */)
{
    work_task = new WorkTask(4096*10, 1);  

    init_uart();

    opus_decoder_ = std::make_unique<OpusDecoderWrapper>(16000,1,60);   //用于OPUS

    /*测试LED*/
    gpio_config_t gpio_init_struct = {0};
    gpio_init_struct.intr_type = GPIO_INTR_DISABLE; /* 失能引脚中断 */
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT; /* 输入输出模式 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE; /* 使能上拉 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE; /* 失能下拉 */
    gpio_init_struct.pin_bit_mask = 1ull << GPIO_NUM_48; /* 设置的引脚的位掩码*/
    gpio_config(&gpio_init_struct); /* 配置 GPIO */
    LED(1); /* 关闭 LED */
    /*测试LED*/

}

App::~App()
{
    delete work_task;
}

#if OPUS_MODE
//OPUS 解码
void App::audio_output_process(AudioHAL* audio_){
    std::unique_lock<std::mutex>lock(pcm_mutex);
    if(opus_queue_.empty())
    {
        opus_queue_cv_.notify_one();
        return;
    }
    
    auto opus = std::move(opus_queue_.front());
    opus_queue_.pop_front();
    lock.unlock();
    audio_->enable_output();
    work_task->add_task([this, opus = std::move(opus), audio_]() mutable{
        std::vector<int16_t> pcm;
        if (!opus_decoder_->Decode(std::move(opus), pcm))
        {
            return;
        }
        audio_->output_data(pcm);
    });
}


#else
// PCM 输出 //后面用于接送后串口数据进行播放
void App::audio_output_process(AudioHAL* audio_){

    std::unique_lock<std::mutex>lock(pcm_mutex);
    if(pcm_output_packets_.empty())     
    {
        //如果未开启录音, 
        if(!input_audio_process_->is_running())
        {
            input_audio_process_->start();  // 启动音频处理
        }       //bu应该放在这里，应该在串口接收数据部分
        return;
    }
    auto pcm = std::move(pcm_output_packets_.front());
    pcm_output_packets_.pop_front();
    lock.unlock();
    audio_->enable_output();

    work_task->add_task([this, pcm = std::move(pcm), audio_]() mutable{
        std::lock_guard<std::mutex>lock(pcm_mutex);
        LED(0); /* 关闭 LED */
        audio_->output_data(pcm);
    });
}
#endif // DEBUG

/**
 * @brief 放一个整数的每一位数字音频
 * 
 * @param number要播放的数字（如1234）
 */
void App::play_number(int number)
{
    if(number < 0){
        number =- number;//处理负数（可选）
    }
    //将数字转为字符串，方便逐字符处理
    std::string num_str = std::to_string(number);
    for(char c : num_str){
        int digit = c - '0';//字符转数字（0-9）
        if(digit >= 0 && digit <= 9){
            play_p3_audio(P3SoundLable::DIGIT_AUDIO[digit]);    //通过数组直接访问
        }
    }
}

//OPUS  播放 p3 音频
void App::play_p3_audio(const std::string_view& p3_sound_lable)
{
    {
        std::unique_lock<std::mutex> lock(pcm_mutex);
        opus_queue_cv_.wait(lock,[this](){
            return opus_queue_.empty();
        });
    }

    work_task-> wait_work_task_completion();

    const char*data = p3_sound_lable.data();
    size_t size = p3_sound_lable.size();

    for (const char* p = data; p < data + size;)
    {
        auto p3_frame_header = (P3HeaderStructure*)p;
        auto paly_size = ntohs(p3_frame_header->payload_size);
        
        std::vector<uint8_t> opus(paly_size);
        memcpy(opus.data(), p3_frame_header->payload, paly_size);

        p += sizeof(P3HeaderStructure) + paly_size;

        std::lock_guard<std::mutex>lock(pcm_mutex);
        opus_queue_.emplace_back(std::move(opus));
    }
}

void App::audio_task_loop()
{
    auto audio_ = Board::GetInstance().GetAudioHAL();
    while(true)
    {
        App::audio_input_process(audio_);
        App::audio_output_process(audio_);
        App::audio_speek_process();
    }
}


/**
 * @brief 获取音频PcM数据
 * @param pcm_data 存储PcM数据的向量 
 * @param samples 样本数
 * @return ref 是否需要处理参考通道 
 * @return true表示获取成功，false表示获取失败 
 */
bool App::get_audio_pcm_resampl(std::vector<int16_t>& pcm_data, int samples, bool ref)
{
    auto audio_=Board::GetInstance().GetAudioHAL();

    //处理参考通道
    if(audio_->is_input_ref())
    {
        samples = samples *2;
    }

    pcm_data.resize(samples);
    if(!audio_->input_data(pcm_data))
    {
        return false;
    }

    //如果音频未设置参考通道（单通道）
    if ((!audio_->is_input_ref()))
    {
        return true;
    }
    
    auto mic_pcm = std::vector<int16_t>(pcm_data.size()/ 2);
    auto ref_pcm = std::vector<int16_t>(pcm_data.size()/ 2);
    for(size_t i = 0, j = 0; i < mic_pcm.size(); ++i, j+=2)
    {
        mic_pcm[i] = pcm_data[j];
        ref_pcm[i] = pcm_data[j + 1];
    }

    if (!ref)
    {
        pcm_data = std::move(mic_pcm);
        return true;
    }
    //处理参考通道
    pcm_data.resize(mic_pcm.size() + ref_pcm.size());
    for(size_t i=0,j=0;i<mic_pcm.size();++i,j+= 2){
        pcm_data[j] = mic_pcm[i];
        pcm_data[j+1]=ref_pcm[i];
    }
    return true;
    
}

void App::audio_input_process(AudioHAL* audio_)
{
    if(input_audio_process_->is_running()){
        int samples = input_audio_process_->get_feed_size(2);
        std::vector<int16_t>pcm_data(samples);
        get_audio_pcm_resampl(pcm_data, samples);
        input_audio_process_->feed(pcm_data);
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(30));
}


void App::audio_speek_process()
{
    if(!input_audio_process_->get_vadspeek_state())
    {
        vadspeek_count++;
        if(vadspeek_count > 50)
        {
            vadspeek_count = 0;
            pcm_packets_.clear(); 
        }
    }
    else
    {
        vadspeek_count = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(1));
}


void App::run()
{
    auto audio_ = Board::GetInstance().GetAudioHAL();
    audio_->enable_input();

    input_audio_process_= std::make_unique<InputAudioProcess>(audio_);
    
    //  检查一句话是否说完  后面用于发送到串口
    input_audio_process_->set_vad_callback([this](){
        this->add_task([this](){
            std::unique_lock<std::mutex>lock(pcm_mutex);
            
            input_audio_process_-> stop();   //这段话说完了，将音频发送。串口接收到开始并播放完后打开。若没有数据，可过段时间打开。也可不停止，一直发
            
            //pcm_output_packets_.clear();
            for (auto& packet : pcm_packets_) {
                //pcm_output_packets_.push_back(std::move(packet));
                LED(1); /* 关闭 LED */

                uint16_t data_len = packet.size() * sizeof(int16_t);

                // 构建数据帧
                uint8_t header[4];
                header[0] = 0xAA;
                header[1] = 0x55;
                header[2] = (data_len >> 8) & 0xFF;
                header[3] = data_len & 0xFF;

                // 计算 CRC（仅对 PCM 数据）
                uint8_t* pcm_bytes = reinterpret_cast<uint8_t*>(packet.data());
                uint8_t crc = crc8(pcm_bytes, data_len);

                // 发送帧头
                uart_write_bytes(UART_NUM_0, header, sizeof(header));

                // 发送 PCM 数据
                uart_write_bytes(UART_NUM_0, pcm_bytes, data_len);

                // 发送 CRC
                uart_write_bytes(UART_NUM_0, &crc, sizeof(crc));
            }

            // // 计算总数据长度（字节数）
            // size_t total_bytes = 0;
            // for (const auto& packet : pcm_packets_) {
            //     total_bytes += packet.size() * sizeof(int16_t);
            // }

            // // 构建数据帧
            // uint8_t header[4];
            // header[0] = 0xAA;
            // header[1] = 0x55;
            // header[2] = (total_bytes >> 8) & 0xFF;  // 长度高字节
            // header[3] = total_bytes & 0xFF;        // 长度低字节

            // // 创建连续缓冲区存放所有PCM数据
            // std::vector<uint8_t> combined_data;
            // combined_data.reserve(total_bytes);  // 预分配空间提高效率

            // // 将所有包的数据复制到连续缓冲区
            // for (const auto& packet : pcm_packets_) {
            //     const uint8_t* packet_bytes = reinterpret_cast<const uint8_t*>(packet.data());
            //     size_t packet_size = packet.size() * sizeof(int16_t);
            //     combined_data.insert(combined_data.end(), packet_bytes, packet_bytes + packet_size);
            // }

            // // 获取连续数据的指针
            // uint8_t* pcm_bytes = combined_data.data();

            // // 计算CRC（对整个拼接后的数据）
            // uint8_t crc = crc8(pcm_bytes, total_bytes);

            // // 发送帧头
            // uart_write_bytes(UART_NUM_0, header, sizeof(header));

            // // 发送PCM数据
            // uart_write_bytes(UART_NUM_0, pcm_bytes, total_bytes);

            // // 发送CRC
            // uart_write_bytes(UART_NUM_0, &crc, sizeof(crc));

            pcm_packets_.clear();       
        });
    });

    input_audio_process_->set_data_output_callback([this](std::vector<int16_t>&& pcm){
        work_task->add_task([this,pcm = std::move(pcm)]() mutable{
            this->add_task([this,pcm =std::move(pcm)](){
                pcm_packets_.emplace_back(std::move(pcm));     
            });
        });
    });


    xTaskCreatePinnedToCore([](void*arg){
        App*app = (App*)arg;
        app->audio_task_loop();
        vTaskDelete(NULL);
    }, "audio", 4096 * 6, this, 8, &audio_task_handle_, 1);



#if OPUS_MODE
    static int random = 0;
    random = Board::GetInstance().getRandom();

    play_p3_audio(P3SoundLable::P3_welcome);
    play_number(random);
#endif
    
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        MutexUniqueLock lock(task_mutex);
        // 等待条件：任务队列非空（使用lambda表达式作为判断条件）
        task_condition_variable_.wait(lock, [this](){
            return !task_list.empty();
        });

        // 移动任务列表到局部变量（减少锁持有时间）
        ListFunction func_list = std::move(task_list);
        lock.unlock();  // 提前释放锁

        // 执行所有任务
        for (auto& func : func_list) {
            func();
        }
    }
}



void App::add_task(FuncVoid task)
{
    MutexLockGuard lock(task_mutex);
    task_count++;
    task_list.emplace_back([call = std::move(task),this]()
    { 
        call();
        {
            MutexLockGuard lock(task_mutex);
            task_count--;
            if (task_count == 0 && task_list.empty()) {
                task_condition_variable_.notify_one();
            }
        }
    });
    
    // 通知工作线程有新任务
    task_condition_variable_.notify_all();
}

