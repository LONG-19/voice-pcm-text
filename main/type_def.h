#pragma once

#include <functional>
#include <mutex>
#include <list>

using FuncVoid = std::function<void()>;
using ListFunction = std::list<FuncVoid>;


using MutexUniqueLock = std::unique_lock<std::mutex>;

using MutexLockGuard = std::lock_guard<std::mutex>;

