#pragma once
#include <esp_err.h>

#include "file_interface.h"

// SD Card
#define PIN_NUM_MISO  GPIO_NUM_12
#define PIN_NUM_MOSI  GPIO_NUM_13  
#define PIN_NUM_CLK   GPIO_NUM_14  
#define PIN_NUM_CS    GPIO_NUM_15

#define EXAMPLE_MAX_CHAR_SIZE    64

#define TAG "example"

#define MOUNT_POINT "/sdcard"

class SdCard :  public FileInterface
{
private:
    FILE* m_file = nullptr;
public:
    SdCard();
    ~SdCard();

    esp_err_t open(const char* filename,const char* mode) override;
    esp_err_t close() override; 

    //原接口保留（可选，推荐改用新接口）
    esp_err_t read_file(char* output,size_t output_size) override;
    esp_err_t write_file(const char* data, size_t size) override;
    esp_err_t read_line(int line_num,char* output,size_t output_size) override; //=g表示纯虚函数(C++抽象方法)
    esp_err_t seek(size_t offset, SeekMode mode) override;

};

