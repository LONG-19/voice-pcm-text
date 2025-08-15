#pragma once

#include "../board.h"
#include <audio_hal.h>


/*I2C port and GPIOs*/
#define AUDIO_I2C_NUM             (I2C_NUM_0)
#define AUDIO_I2C_SDA_IO          GPIO_NUM_2
#define AUDIO_I2C_SCL_IO          GPIO_NUM_1

/*I2S port and GPIOs */
#define AUDIO_I2S_NUM             (I2S_NUM_0)
#define AUDIO_I2S_MCK_IO          GPIO_NUM_42
#define AUDIO_I2S_BCK_IO          GPIO_NUM_40
#define AUDIO_I2S_WS_IO           GPIO_NUM_39
#define AUDIO_I2S_DI_IO           GPIO_NUM_15
#define AUDIO_I2S_DO_IO           GPIO_NUM_41

/* 功放使能IO */
#define AUDIO_PA_EN_IO            GPIO_NUM_4

/***音频输入输出采样率*/
#define AUDIO_OUT_SAMPLE_RATE     (16000)
#define AUDIO_IN_SAMPLE_RATE      (16000)

#define ES8311_I2C_ADDR           (0x18 << 1)
#define ES7210_I2C_ADDR           (0x40 << 1)


class VoiceDevBoard:public Board
{
private:
    i2c_master_bus_handle_t i2c0_bus;
    AudioHAL* audio_hal;

public:
    VoiceDevBoard();
    ~VoiceDevBoard();

    AudioHAL* GetAudioHAL() override;
};


