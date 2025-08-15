#include "audio_es7210.h"
#include "esp_log.h"
#include "esp_err.h"
#include <cstdio>
#include <cstring>
#include "driver/i2s_tdm.h"
#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <es7210.h>
#include <esp_check.h>

#define TAG "ES7210"

esp_err_t AudioEs7210::enable()
{
    return i2s_channel_enable(i2s_chan);
}

esp_err_t AudioEs7210::disable()
{
    return i2s_channel_disable(i2s_chan);
}

esp_err_t AudioEs7210::read(void *dest, size_t size,size_t *bytes_read)
{
    return i2s_channel_read(i2s_chan, dest, size, bytes_read,pdMS_TO_TICKS(1000));
}


AudioEs7210::AudioEs7210(uint32_t sample_rate, uint8_t channel_num):sample_rate(sample_rate),channel_num(channel_num){
    
    i2s_chan = es7210_i2s_init();
    es7210_code_init();
    
}

i2s_chan_handle_t AudioEs7210::es7210_i2s_init(void)
{
    i2s_chan_handle_t i2s_rx_chan = NULL;   // 定义接收通道句柄
    i2s_chan_config_t i2s_rx_conf = I2S_CHANNEL_DEFAULT_CONFIG(EXAMPLE_I2S_NUM, I2S_ROLE_MASTER);//配置接收通道
    ESP_ERROR_CHECK(i2s_new_channel(&i2s_rx_conf, NULL, &i2s_rx_chan));// 创建i2s通道
    ESP_LOGI(TAG,"Configure I2S receive channel to TDM mode");

    //定义接收通道为I2STDM模式并配置
    i2s_tdm_config_t i2s_tdm_rx_conf={
        .clk_cfg = {
            .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
            .clk_src= I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = EXAMPLE_I2S_MCLK_MULTIPLE
        },
        .slot_cfg = I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(EXAMPLE_I2S_SAMPLE_BITS, I2S_SLOT_MODE_STEREO, EXAMPLE_I2S_TDM_SLOT_MASK),

        .gpio_cfg={
            .mclk = EXAMPLE_I2S_MCK_IO,
            .bclk = EXAMPLE_I2S_BCK_IO,
            .ws = EXAMPLE_I2S_WS_IO,
            .dout = GPIO_NUM_NC,    // ES7210 only has ADC capability
            .din = EXAMPLE_I2S_DI_IO
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(i2s_rx_chan, &i2s_tdm_rx_conf));//初始化I2S通道为TDM模式
    
    return i2s_rx_chan;

}

/*
 @brief初始化ES7210芯片
*/
void AudioEs7210::es7210_code_init(void)
{
    //初始化I2C接口
    ESP_LOGI(TAG,"Init I2C used to configure ES7210");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = EXAMPLE_I2C_SDA_IO,
        .scl_io_num = EXAMPLE_I2C_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master{
            .clk_speed= EXAMPLE_ES7210_I2C_CLK,
        }
    };
    ESP_ERROR_CHECK(i2c_param_config(EXAMPLE_I2C_NUM,&i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(EXAMPLE_I2C_NUM,i2c_conf.mode,0,0,0));
    
    //配置es7210器件句柄
    es7210_dev_handle_t es7210_handle = NULL;
    es7210_i2c_config_t es7210_i2c_conf = {
        .i2c_port = EXAMPLE_I2C_NUM,
        .i2c_addr = EXAMPLE_ES7210_I2C_ADDR
    };
    ESP_ERROR_CHECK(es7210_new_codec(&es7210_i2c_conf,&es7210_handle));

    //初始化es7210芯片
    ESP_LOGI(TAG, "Configure ES7210 codec parameters");
    es7210_codec_config_t codec_conf = {
        .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
        .mclk_ratio = EXAMPLE_I2S_MCLK_MULTIPLE,
        .i2s_format = EXAMPLE_I2S_TDM_FORMAT,
        .bit_width = (es7210_i2s_bits_t)EXAMPLE_I2S_SAMPLE_BITS,
        .mic_bias = EXAMPLE_ES7210_MIC_BIAS,
        .mic_gain = EXAMPLE_ES7210_MIC_GAIN,
        .flags{
            .tdm_enable = true
        }
    };

    ESP_ERROR_CHECK(es7210_config_codec(es7210_handle, &codec_conf));
    ESP_ERROR_CHECK(es7210_config_volume(es7210_handle, EXAMPLE_ES7210_ADC_VOLUME));
}


