#pragma once
#include <string_view>
#include <cstdint>

namespace P3SoundLable{
    extern const char p3_0_start[] asm("_binary_0_p3_start");
    extern const char p3_0_end[] asm("_binary_0_p3_end");
    static const std::string_view P3_0 {
        static_cast<const char*>(p3_0_start),
        static_cast<size_t>((p3_0_end) - (p3_0_start))
    };

    extern const char p3_1_start[] asm("_binary_1_p3_start");
    extern const char p3_1_start[] asm("_binary_1_p3_start");
    extern const char p3_1_end[] asm("_binary_1_p3_end");
    static const std::string_view P3_1 {
        static_cast<const char*>(p3_1_start),
        static_cast<size_t>((p3_1_end) - (p3_1_start))
    };

    extern const char p3_2_start[] asm("_binary_2_p3_start");
    extern const char p3_2_end[] asm("_binary_2_p3_end");
    static const std::string_view P3_2 {
        static_cast<const char*>(p3_2_start),
        static_cast<size_t>((p3_2_end) - (p3_2_start))
    };

    extern const char p3_3_start[] asm("_binary_3_p3_start");
    extern const char p3_3_end[] asm("_binary_3_p3_end");
    static const std::string_view P3_3 {
        static_cast<const char*>(p3_3_start),
        static_cast<size_t>((p3_3_end) - (p3_3_start))
    };

    extern const char p3_4_start[] asm("_binary_4_p3_start");
    extern const char p3_4_end[] asm("_binary_4_p3_end");
    static const std::string_view P3_4 {
        static_cast<const char*>(p3_4_start),
        static_cast<size_t>((p3_4_end) - (p3_4_start))
    };

    extern const char p3_5_start[] asm("_binary_5_p3_start");
    extern const char p3_5_end[] asm("_binary_5_p3_end");
    static const std::string_view P3_5 {
        static_cast<const char*>(p3_5_start),
        static_cast<size_t>((p3_5_end) - (p3_5_start))
    };

    extern const char p3_6_start[] asm("_binary_6_p3_start");
    extern const char p3_6_end[] asm("_binary_6_p3_end");
    static const std::string_view P3_6 {
        static_cast<const char*>(p3_6_start),
        static_cast<size_t>((p3_6_end) - (p3_6_start))
    };

    extern const char p3_7_start[] asm("_binary_7_p3_start");
    extern const char p3_7_end[] asm("_binary_7_p3_end");
    static const std::string_view P3_7 {
        static_cast<const char*>(p3_7_start),
        static_cast<size_t>((p3_7_end) - (p3_7_start))
    };

    extern const char p3_8_start[] asm("_binary_8_p3_start");
    extern const char p3_8_end[] asm("_binary_8_p3_end");
    static const std::string_view P3_8 {
        static_cast<const char*>(p3_8_start),
        static_cast<size_t>((p3_8_end) - (p3_8_start))
    };

    extern const char p3_9_start[] asm("_binary_9_p3_start");
    extern const char p3_9_end[] asm("_binary_9_p3_end");
    static const std::string_view P3_9 {
        static_cast<const char*>(p3_9_start),
        static_cast<size_t>((p3_9_end) - (p3_9_start))
    };

    extern const char p3_activation_start[] asm("_binary_activation_p3_start");
    extern const char p3_activation_end[] asm("_binary_activation_p3_end");
    static const std::string_view P3_activation {
        static_cast<const char*>(p3_activation_start),
        static_cast<size_t>((p3_activation_end) - (p3_activation_start))
    };

    extern const char p3_upgrade_start[] asm("_binary_upgrade_p3_start");
    extern const char p3_upgrade_end[] asm("_binary_upgrade_p3_end");
    static const std::string_view P3_upgrade {
        static_cast<const char*>(p3_upgrade_start),
        static_cast<size_t>((p3_upgrade_end) - (p3_upgrade_start))
    };


    extern const char p3_welcome_start[] asm("_binary_welcome_p3_start");
    extern const char p3_welcome_end[] asm("_binary_welcome_p3_end");
    static const std::string_view P3_welcome {
        static_cast<const char*>(p3_welcome_start),
        static_cast<size_t>((p3_welcome_end) - (p3_welcome_start))
    };

    extern const char p3_wificonfig_start[] asm("_binary_wificonfig_p3_start");
    extern const char p3_wificonfig_end[] asm("_binary_wificonfig_p3_end");
    static const std::string_view P3_wificonfig {
        static_cast<const char*>(p3_wificonfig_start),
        static_cast<size_t>((p3_wificonfig_end) - (p3_wificonfig_start))
    };


    //新增：数字音频标签数组（按索引0~9存储）
    static const std::string_view DIGIT_AUDIO[] = {
        P3_0, P3_1, P3_2, P3_3, P3_4,
        P3_5, P3_6, P3_7, P3_8, P3_9
    };

}


struct  P3HeaderStructure
{
    uint8_t type;
    uint8_t reserved;
    uint16_t payload_size;
    uint8_t payload[];
}__attribute__((packed));

