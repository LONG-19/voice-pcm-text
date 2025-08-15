#include "board.h"
#include <esp_random.h>

i2c_master_bus_handle_t Board::init_i2c(i2c_port_t i2c_port,gpio_num_t i2c_sda_pin,gpio_num_t i2c_scl_pin)
{
    i2c_master_bus_handle_t i2c_bus;
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = i2c_port,
        .sda_io_num = i2c_sda_pin,
        .scl_io_num = i2c_scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
        },
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus));
    return i2c_bus;
}


Board::Board()
{
    
}


int Board::getRandom(){
    //使用esp_random（）生成更安全的随机数
    uint32_t random_value = esp_random();
    //确保在100000到999999范围内
    return 100000+(random_value % 900000);
}



