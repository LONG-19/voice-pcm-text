set(SOURCES "main.cc"
    "app.cpp"
    "work_task.cpp"
    "drivers/storage/sd_card.cpp"
    "drivers/audio/wav_recorder.cpp"
    "drivers/audio/audio_hal.cpp"
    "drivers/audio/audio_es8311_es7210.cpp"
    "board/board.cpp"
    "drivers/audio/input_audio_process.cpp"
)

set(INCLUDE_DIRS  "."
    "drivers"
    "drivers/storage"
    "drivers/audio"
    "board"
    "resources"
)

#新增开发板需要再这里类似写一个配置BOARD_TYPE的宏为板子类型文件夹名称
if(CONFIG_BOARD_TYPE_VOICE_DEV)
    set(BOARD_TYPE "voice_dev")
#elseif(CONFIG_BOARD_TYPE_XXX_XXX)
#   set(BOARD_TYPE "xXX-xXX")

endif()

file(GLOB BOARD_TYPE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/board/${BOARD_TYPE}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/board/${BOARD_TYPE}/*.c
)

list(APPEND SOURCES ${BOARD_TYPE_SOURCES})


file(GLOB P3_SOUNDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.p3)


idf_component_register(SRCS ${SOURCES}
                    EMBED_FILES ${P3_SOUNDS}
                    INCLUDE_DIRS ${INCLUDE_DIRS}
)

