#pragma once
#include <cstddef>
#include <esp_err.h>


class AudioInputInterface{
public:
    virtual ~AudioInputInterface() = default;  //基类的析构函数一定要是虚(virtua1)函数，这样在销毁派生类的对象时才能正确调用派生类的析构函数

    virtual esp_err_t enable() = 0;
    virtual esp_err_t disable() = 0; 
    virtual esp_err_t read(void *dest, size_t size, size_t *bytes_read) = 0;
};

