#pragma once

#include "audio_hal.h"
#include <soc/gpio_num.h>
#include <hal/i2c_types.h>

class AudioEs8311Es7210: public AudioHAL
{

private:
    /**
     * @brief 初始化i2s通道
     * @param mclk_pin mclk引|脚
     * @param bclk_pin bclk引脚
     * @param ws_pin ws引脚
     * @param dout_pin dout引脚
     * @param din_pin din引脚
     */
    void init_i2s_channel(gpio_num_t mclk_pin,gpio_num_t bclk_pin,gpio_num_t ws_pin, gpio_num_t dout_pin,gpio_num_t din_pin);

    //初始化es8311
    /**
     * @brief 初始化es8311
     * @param i2c_bus_handle i2c总线句柄
     * @param i2c_porti2c 总线端口
     * @param es8311_i2c_addr es8311 i2c地址
     * @param pa_pin 功放使能引脚
     * @param pa_reverted
     * pa_reverted = false表示非反转逻辑：当pa_pin 引脚输出1（高电平）时，功放（PA）被使能；输出θ（低电平）时，功放被禁用。
     * pa_reverted = true 表示反转逻辑：当pa_pin 引脚输出（低电平）时，功放被使能；输出1（高电平）时，功放被禁用。
     */
    void init_es8311(void* i2c_bus_handle,i2c_port_t i2c_port,uint8_t es8311_i2c_addr,gpio_num_t pa_pin,bool pa_reverted);
    /**
     * @brief 初始化es7210
     * @param i2c_bus_handle i2c总线句柄
     * @param i2c_port i2c总线端口
     * @param es7210_i2c_addr es7210 i2c地址
     */
    void init_es7210(void* i2c_bus_handle,i2c_port_t i2c_port,uint8_t es7210_i2c_addr);

public:

    virtual void enable_input()override;
    virtual void enable_output()override;
    virtual void disable_input()override;
    virtual void disable_output()override;

    virtual void write(const int16_t* data, int samples)override;
    virtual int read(int16_t* data, int samples)override;
    virtual void set_out_volume(int volume)override;

    AudioEs8311Es7210(void* i2c_bus_handle, i2c_port_t i2c_port, int input_sample_rate, int output_sample_rate,
        gpio_num_t mclk_pin, gpio_num_t bclk_pin, gpio_num_t ws_pin, gpio_num_t dout_pin, gpio_num_t din_pin,
        gpio_num_t pa_pin,bool pa_reverted,uint8_t es8311_i2c_addr,uint8_t es7210_i2c_addr,bool input_ref);
    ~AudioEs8311Es7210();
    
};

