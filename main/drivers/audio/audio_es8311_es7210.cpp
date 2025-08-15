#include "audio_es8311_es7210.h"
#include <driver/i2s_common.h>
#include <driver/i2s_std.h>
#include <driver/i2s_tdm.h>
#include <esp_codec_dev_defaults.h>
#include <esp_err.h>

void AudioEs8311Es7210::init_i2s_channel(gpio_num_t mclk_pin,gpio_num_t bclk_pin,gpio_num_t ws_pin, gpio_num_t dout_pin,gpio_num_t din_pin)
{
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role=I2S_ROLE_MASTER, //i2s的主从模式
        .dma_desc_num = 6,
        .dma_frame_num = 256,
        .auto_clear_after_cb = true,
        .auto_clear_before_cb = false,
        .intr_priority = 0,//中断优先级，数值越小优先级越高
    };
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle));
    /*初始化i2s为std模式 并打开i2s发送通道*/
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = (uint32_t)output_sample_rate,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .ext_clk_freq_hz = 0,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_BOTH,
            .ws_width= I2S_DATA_BIT_WIDTH_16BIT,
            .ws_pol = false,
            .bit_shift = true,
            .left_align = true,
            .big_endian = false,
            .bit_order_lsb = false
        },
        .gpio_cfg = {
            .mclk = mclk_pin,
            .bclk = bclk_pin,
            .ws= ws_pin,
            .dout = dout_pin,
            .din = GPIO_NUM_NC,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            }
        }
    };

    //定义接收通道为I2STDM模式并配置
    i2s_tdm_config_t tdm_cfg={
        .clk_cfg = {
            .sample_rate_hz = (uint32_t)input_sample_rate,
            .clk_src= I2S_CLK_SRC_DEFAULT,
            .ext_clk_freq_hz = 0,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            .bclk_div = 8
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = i2s_tdm_slot_mask_t(I2S_TDM_SLOT0| I2S_TDM_SLOT1| I2S_TDM_SLOT2),
            .ws_width= I2S_TDM_AUTO_WS_WIDTH,
            .ws_pol = false,
            .bit_shift = true,
            .left_align = false,
            .big_endian = false,
            .bit_order_lsb = false,
            .skip_mask = false,
            .total_slot = I2S_TDM_AUTO_SLOT_NUM
        },

        .gpio_cfg={
            .mclk = mclk_pin,
            .bclk = bclk_pin,
            .ws = ws_pin,
            .dout = GPIO_NUM_NC,    // ES7210 only has ADC capability
            .din = din_pin,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            }
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle,&std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(rx_handle,&tdm_cfg));
}


void AudioEs8311Es7210::init_es8311(void* i2c_bus_handle,i2c_port_t i2c_port,uint8_t es8311_i2c_addr,gpio_num_t pa_pin,bool pa_reverted)
{
    //Output
    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = i2c_port,
        .addr = es8311_i2c_addr,
        .bus_handle = i2c_bus_handle
    };

    out_ctrl_if= audio_codec_new_i2c_ctrl(&i2c_cfg);

    gpio_if = audio_codec_new_gpio();
    es8311_codec_cfg_t es8311_cfg={};
    es8311_cfg.ctrl_if = out_ctrl_if;
    es8311_cfg.gpio_if = gpio_if;
    es8311_cfg.codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC;
    es8311_cfg.pa_pin = pa_pin;
    es8311_cfg.pa_reverted = pa_reverted;
    es8311_cfg.use_mclk = true;
    es8311_cfg.hw_gain.pa_voltage = 5.0;
    es8311_cfg.hw_gain.codec_dac_voltage = 3.3;
    out_codec_if= es8311_codec_new(&es8311_cfg);

    esp_codec_dev_cfg_t dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
        .codec_if= out_codec_if,
        .data_if= data_if
    };

    audio_output_dev = esp_codec_dev_new(&dev_cfg);

}

void AudioEs8311Es7210::init_es7210(void* i2c_bus_handle,i2c_port_t i2c_port,uint8_t es7210_i2c_addr)
{
    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = i2c_port,
        .addr = es7210_i2c_addr,
        .bus_handle = i2c_bus_handle
    };

    in_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);

    es7210_codec_cfg_t es7210_cfg = {};
    es7210_cfg.ctrl_if = in_ctrl_if;
    es7210_cfg.mic_selected = ES7120_SEL_MIC1 | ES7120_SEL_MIC2| ES7120_SEL_MIC3;
    in_codec_if = es7210_codec_new(&es7210_cfg);

    esp_codec_dev_cfg_t dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
        .codec_if = in_codec_if,
        .data_if = data_if,
    };

    audio_input_dev = esp_codec_dev_new(&dev_cfg);

}



AudioEs8311Es7210::AudioEs8311Es7210(void* i2c_bus_handle, i2c_port_t i2c_port, int input_sample_rate, int output_sample_rate,
        gpio_num_t mclk_pin, gpio_num_t bclk_pin, gpio_num_t ws_pin, gpio_num_t dout_pin, gpio_num_t din_pin,
        gpio_num_t pa_pin,bool pa_reverted,uint8_t es8311_i2c_addr,uint8_t es7210_i2c_addr,bool input_ref)
{
    this->input_sample_rate = input_sample_rate;
    this->output_sample_rate = output_sample_rate;
    this->input_ref = input_ref;

    init_i2s_channel(mclk_pin, bclk_pin, ws_pin, dout_pin, din_pin);

    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = I2S_NUM_0,
        .rx_handle = rx_handle,
        .tx_handle = tx_handle
    };
    data_if = audio_codec_new_i2s_data(&i2s_cfg);

    init_es8311(i2c_bus_handle, i2c_port, es8311_i2c_addr, pa_pin, pa_reverted);
    init_es7210(i2c_bus_handle, i2c_port, es7210_i2c_addr);

    i2s_channel_enable(tx_handle);
    i2s_channel_enable(rx_handle);
}


AudioEs8311Es7210::~AudioEs8311Es7210()
{
    esp_codec_dev_close(audio_input_dev);
    esp_codec_dev_close(audio_output_dev);
    esp_codec_dev_delete(audio_output_dev);
    esp_codec_dev_delete(audio_input_dev);
    audio_codec_delete_codec_if(in_codec_if);
    audio_codec_delete_ctrl_if(in_ctrl_if);
    audio_codec_delete_codec_if(out_codec_if);
    audio_codec_delete_ctrl_if(out_ctrl_if);
    audio_codec_delete_gpio_if(gpio_if);
    audio_codec_delete_data_if(data_if);
}

void AudioEs8311Es7210::enable_input()
{
    if(enabled_input)
    {
        return;
    }

    esp_codec_dev_sample_info_t codec_dev = {
        .bits_per_sample = 16,
        .channel = 4,
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0),
        .sample_rate = (uint32_t)input_sample_rate,
        .mclk_multiple = 0,
    };

    if(input_ref)
    {
        codec_dev.channel_mask |= ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1);
    }

    ESP_ERROR_CHECK(esp_codec_dev_open(audio_input_dev, &codec_dev));
    //修改增益
    ESP_ERROR_CHECK(esp_codec_dev_set_in_channel_gain(audio_input_dev, ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0), 38.0));
    enabled_input = true;
}

void AudioEs8311Es7210::set_out_volume(int volume)
{
    ESP_ERROR_CHECK(esp_codec_dev_set_out_vol(audio_output_dev,volume));
    output_volume = volume;
}


void AudioEs8311Es7210::enable_output()
{
    if(enabled_output)
    {
        return;
    }

    esp_codec_dev_sample_info_t codec_dev = {
        .bits_per_sample = 16,
        .channel = 1,
        .channel_mask =0,
        .sample_rate = (uint32_t)output_sample_rate,
        .mclk_multiple = 0,
    };

    ESP_ERROR_CHECK(esp_codec_dev_open(audio_output_dev, &codec_dev));
    ESP_ERROR_CHECK(esp_codec_dev_set_out_vol(audio_output_dev, output_volume));
    enabled_output = true;
}


void AudioEs8311Es7210::disable_input()
{
    ESP_ERROR_CHECK(esp_codec_dev_close(audio_input_dev));
    enabled_input = false;
}

void AudioEs8311Es7210::disable_output()
{
    ESP_ERROR_CHECK(esp_codec_dev_close(audio_output_dev));
    enabled_output = false;
}




void AudioEs8311Es7210::write(const int16_t* data, int samples)
{
    if(enabled_output)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_codec_dev_write(audio_output_dev,(void*)data, samples * sizeof(int16_t)));
    }
}


int AudioEs8311Es7210::read(int16_t* data,int samples)
{
    if (enabled_input)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_codec_dev_read(audio_input_dev,(void*)data , samples * sizeof(int16_t)));
    }
    return  samples * sizeof(int16_t);
}

