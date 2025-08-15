#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "wav_recorder.h"
#include "sd_card.h"
#include "app.h"

#define TAG "main"


extern "C" void app_main(void)
{
    App::GetInstance().run();
}
