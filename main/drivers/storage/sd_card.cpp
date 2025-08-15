#include <string.h>
#include <Cstdio>
#include "sd_card.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <esp_log.h>
#include <cstring>
#include "ff.h"  // FatFs头文件
#include <cerrno>

SdCard::SdCard()
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false  // ← 关键! 必须设为false才能启用LFN
    };
    sdmmc_card_t* card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    ESP_LOGI(TAG, "Using SPI peripheral");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,    //
        .sclk_io_num = PIN_NUM_CLK,     // GPIO_NUM_14  
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    
    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = (spi_host_device_t)host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    ESP_LOGI(TAG, "Filesystem mounted");

    //打印SD卡信息
    sdmmc_card_print_info(stdout, card);  


    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // ESP_LOGI(TAG, "Card unmounted");

    // spi_bus_free((spi_host_device_t)host.slot);
}

SdCard::~SdCard()
{
    // 可选：添加清理逻辑
}


esp_err_t SdCard::open(const char* filename,const char* mode){
    char file_path[50];
    // 获取错误信息
    int err = errno;
    sprintf(file_path,"%s/%s",MOUNT_POINT,filename);

    m_file = fopen(file_path, mode);
    if (m_file == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s, mode: %s, error: %d (%s)", file_path, mode, err, strerror(err));
        return ESP_FAIL;
    }
    return m_file ? ESP_OK : ESP_FAIL;
}

esp_err_t SdCard::close(){
    if(m_file)
    {
        fclose(m_file);
        m_file = nullptr;
    }
    return ESP_OK;
}


/**读取指定行数据
*1ine num:行号
*output:输出缓冲区
*output size：输出缓冲区大小
*/
esp_err_t SdCard::read_line(int line_num,char*output,size_t output_size)
{
    char line[1000];//假设每行最大长度为1gg0字符
    int current_line = 0;

    char file_path[50];
    char data1[100];

    if (output_size>(sizeof(line)-1))
    {
        return ESP_FAIL;
    }

    //检查输出缓冲区是否足够大
    if (output_size < 1){
        ESP_LOGE(TAG,"Output buffer size is too small");
        return ESP_FAIL;
    }

    //逐行读取文件
    while (fgets(line, sizeof(line), m_file) != NULL){
        current_line++;
        //如果当前行是目标行
        if (current_line == line_num){
            //去掉行末的换行符（如果有）
            size_t len = strlen(line);
            if (len > 0 && line[len-1] =='\n'){
                line[len - 1]= '\0'; //去掉换行符
            }
            //将目标行内容复制到输出缓冲区
            strncpy(output,line,output_size -1);
            output[output_size - 1] = '\0';  //确保字符串以nu11结尾
            return ESP_OK;
        }
    }

    //如果文件行数不足
    if (current_line < line_num){
        ESP_LOGE(TAG,"File has only %d lines,requested line %d",current_line,line_num);
        return ESP_FAIL;
    }

    return ESP_FAIL;
}

/*
*读取指定文件内容
*@param filename:文件路径
*@param output:输出缓冲区
*@param output_size：输出缓冲区大小
*/

esp_err_t SdCard::read_file(char*output,size_t output_size){

    if (m_file == NULL){
        ESP_LOGE(TAG,"Failed to open file for reading");
        return ESP_ERR_INVALID_STATE;
    }

    //获取当前文件指针位置
    long current_pos = ftell(m_file);
    ESP_LOGD(TAG,"Current file position before read:%ld",current_pos);
    //获取文件大小
    if (fseek(m_file, 0, SEEK_END) != 0)    //移动到文件末尾
    {
        ESP_LOGE(TAG,"Failed to seek to end");
        return ESP_FAIL;
    }
    
          
    long file_size = ftell(m_file);   //获取文件大小
    if (file_size < 0)    
    {
        ESP_LOGE(TAG,"Failed to get file size");
        return ESP_FAIL;
    }

    //重置文件指针
    if (fseek(m_file, 0, SEEK_SET)  != 0)    //回到文件开头
    {
        ESP_LOGE(TAG,"Failed to seek to start");
        return ESP_FAIL;
    }
    
    //检查缓冲区大小
    //检查缓冲区是否足够大
    if(output_size < (size_t)(file_size + 1)) {   //+1用于字符串结束符
        ESP_LOGE(TAG,"Output buffer is too small (required:%ld,provided:%d)",file_size +1,output_size);
        return ESP_ERR_INVALID_STATE;
    }

    //读取文件内容
    size_t bytes_read = fread(output, 1, file_size, m_file);
    output[bytes_read] = '\0';  //确保字符串终止

    ESP_LOGI(TAG,"Read %zu/%ld bytes:%.*s",
        bytes_read, file_size,
        (int)(bytes_read > 50 ? 50 : bytes_read), output);

    if (bytes_read != (size_t)file_size){
        ESP_LOGW(TAG,"Partial read (expected %ld,got %zu)",
            file_size, bytes_read);
    }

    return ESP_OK;
}

/*
*像指定文件写入数据
*@param filename:文件路径
*@paramdata:要写入的数据
*@param isAppend:是否为追内容，true:追加内容false:覆盖内容
*/
esp_err_t SdCard::write_file(const char* data, size_t size)
{
    return fwrite(data,1,size,m_file)==size ? ESP_OK : ESP_FAIL;
}

esp_err_t SdCard::seek(size_t offset,SeekMode mode)
{
    if (!m_file){
        return ESP_ERR_INVALID_STATE;   //文件未打开
    }

    int origin;
    switch (mode)
    {
    case SeekMode::CUR:
        origin = SEEK_CUR;
        break;
    case SeekMode::END:
        origin = SEEK_END;
        break;
    case SeekMode::SET:
        origin = SEEK_SET;
        break;
    default:
        return ESP_ERR_INVALID_ARG;
    }

    if (fseek(m_file,offset,origin) !=0 )
    {
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

