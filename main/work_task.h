#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>
#include <mutex>
#include <condition_variable>
#include <list>
#include "type_def.h"


class WorkTask
{
private:
    /* data */
    ListFunction task_list;
    std::mutex mutex_;
    std::condition_variable condition_variable_;
    uint32_t task_count;

public:
    WorkTask(uint32_t stack_size, bool xcore_id);
    ~WorkTask();

    void work_task_loop();
    void add_task(FuncVoid task);

    void wait_work_task_completion();
};

