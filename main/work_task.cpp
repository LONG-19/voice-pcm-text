#include "work_task.h"
#include "esp_mac.h"

WorkTask::WorkTask(uint32_t stack_size, bool xcore_id)
{
    task_count = 0;
    xTaskCreatePinnedToCore([](void *pvParameters){
        WorkTask *work_task = static_cast<WorkTask*>(pvParameters);
        work_task->work_task_loop();
        vTaskDelete(NULL);
    }, "work_task", stack_size, this, 3, NULL,xcore_id);
}

WorkTask::~WorkTask()
{
    // 可以添加清理逻辑，如通知条件变量等
    
}


void WorkTask::work_task_loop()
{   
    while (1)
    {
        MutexUniqueLock lock(mutex_);
        
        // 等待条件：任务队列非空（使用lambda表达式作为判断条件）
        condition_variable_.wait(lock, [this](){
            return !task_list.empty();
        });

        // 移动任务列表到局部变量（减少锁持有时间）
        ListFunction func_list = std::move(task_list);
        lock.unlock();  // 提前释放锁

        // 执行所有任务
        for (auto& func : func_list) {
            func();
        }
    }
}


void WorkTask::add_task(FuncVoid task)
{
    MutexLockGuard lock(mutex_);
    task_count++;
    task_list.emplace_back([call = std::move(task),this]()
    { 
        call();
        {
            MutexLockGuard lock(mutex_);
            task_count--;
            if (task_count == 0 && task_list.empty()) {
                condition_variable_.notify_one();
            }
        }
    });
    
    // 通知工作线程有新任务
    condition_variable_.notify_all();
}


void WorkTask::wait_work_task_completion()
{
    MutexUniqueLock lock(mutex_);
    condition_variable_.wait(lock,[this](){
        return task_list.empty() && task_count == 0;
    });
}



