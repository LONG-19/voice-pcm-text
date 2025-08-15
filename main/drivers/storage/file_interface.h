#pragma once
#include <cstddef>
#include <esp_err.h>

enum class SeekMode{
    SET = SEEK_SET, //从文件开头
    CUR = SEEK_CUR, //从当前位置
    END = SEEK_END  //从文件末尾
};

class FileInterface{
public:
    virtual ~FileInterface() = default;  //基类的析构函数一定要是虚(virtua1)函数，这样在销毁派生类的对象时才能正确调用派生类的析构函数

    virtual esp_err_t open(const char* filename, const char* mode) = 0;
    virtual esp_err_t close() = 0; 

    //原接口保留（可选，推荐改用新接口）
    virtual esp_err_t read_file(char* output, size_t output_size) = 0;
    virtual esp_err_t write_file(const char* data, size_t size)=0;
    virtual esp_err_t read_line(int line_num, char* output, size_t output_size) = 0;//=0表示纯虚函数(C++抽象方法)
    virtual esp_err_t seek(size_t offset, SeekMode mode) = 0;

};

