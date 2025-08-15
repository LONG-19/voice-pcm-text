#include "audio_hal.h"

AudioHAL::AudioHAL()
{

}

AudioHAL::~AudioHAL()
{
    
}

void AudioHAL::output_data(std::vector<int16_t>& pcm)
{
    write(pcm.data(),pcm.size());
}

bool AudioHAL::input_data(std::vector<int16_t>& pcm)
{
    int samples = read(pcm.data(), pcm.size());

    if(samples > 0)
    {
        return true;
    }
    return false;
}

bool AudioHAL::is_input_ref()
{
    if(input_ref)
        return true;
    else
        return false;
}


