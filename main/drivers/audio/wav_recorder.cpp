#include "wav_recorder.h"
#include "format_wav.h"
#include <esp_timer.h>
#include <esp_check.h>
#include <esp_log.h>

#define TAG "WavRecorder"
#define EXAMPLE_I2S_SAMPLE_RATE 16000
#define EXAMPLE_I2S_CHAN_NUM 2
#define EXAMPLE_I2S_SAMPLE_BITS 16

WavRecorder::WavRecorder(){
	
}

esp_err_t WavRecorder::record(uint16_t seconds){

	esp_err_t ret = ESP_OK;
	//计算音频数据大小
	uint32_t byte_rate = EXAMPLE_I2S_SAMPLE_RATE * EXAMPLE_I2S_CHAN_NUM * EXAMPLE_I2S_SAMPLE_BITS / 8;
	uint32_t wav_size = seconds * byte_rate;
	
	//创建初始WAV头部（数据大小暂时设为0）
	wav_header_t wav_header = WAV_HEADER_PCM_DEFAULT(wav_size , EXAMPLE_I2S_SAMPLE_BITS,
						EXAMPLE_I2S_SAMPLE_RATE,
						EXAMPLE_I2S_CHAN_NUM);
			
	//1.写入WAV头部
	// if (m_file->write_file((char *)&wav_header, sizeof(wav_header)) != ESP_OK){
		ESP_LOGE(TAG, "写入WAV头部失败");
	// 	return ESP_FAIL;
	// }
	
	//2，实时录音并分段写入数据
	/*Start recording*/
	size_t wav_written=0;
	static int16_t i2s_readraw_buff[8192];
	
	//使能通道，如果出错直接跳到err标签位置
	// ESP_GOTO_ON_ERROR(audio_input->enable(),  err, TAG, "error while starting i2s rx channel");
	
	while(wav_written < wav_size){
		if(wav_written % byte_rate < sizeof(i2s_readraw_buff)){
			ESP_LOGI(TAG, "Recording:%"PRIu32"/%ds", wav_written / byte_rate +1, seconds);
			printf(".");
		}

		size_t bytes_read = 0;

		// ESP_GOTO_ON_ERROR(audio_input->read(i2s_readraw_buff, sizeof(i2s_readraw_buff), &bytes_read), err, TAG, "error while reading samples from i2s");	

		// if(m_file->write_file((char *)i2s_readraw_buff,bytes_read)!=ESP_OK){
		// 	ESP_LOGE(TAG, "写入音频数据失败");
		// 	goto err;
		// }
		
		wav_written += bytes_read;
	}
	
	printf("录制完成\n");
// err:
// 	// audio_input->disable();
	return ret;
}


