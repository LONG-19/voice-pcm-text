#pragma once
#include <audio_hal.h>
#include <driver/i2c_types.h>
#include <soc/gpio_num.h>
#include <driver/i2c_master.h>

void* create_board();

class Board
{
protected:
    Board();
    i2c_master_bus_handle_t init_i2c(i2c_port_t i2c_port,gpio_num_t i2c_sda_pin,gpio_num_t i2c_scl_pin);

private:
    //禁用拷贝构造函数
    Board(const Board&) = delete;
    //禁用赋值操作
    Board& operator=(const Board&)= delete;

public:
    virtual ~Board()= default;

    static Board& GetInstance()
    {
        static Board* instance = static_cast<Board*>(create_board());
        return *instance;
    }
    virtual AudioHAL*GetAudioHAL() = 0;

    int getRandom();

};

#define REGISTER_BOARD(BOARD_TYPE_CLASS_NAME) \
void* create_board() { \
    return new BOARD_TYPE_CLASS_NAME(); \
}

