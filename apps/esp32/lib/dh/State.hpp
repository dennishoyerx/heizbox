#pragma once 

#include <atomic>
//#include <condition_variable>
#include <functional>
#include <memory>
#include <string> 
//#include <thread>

#if defined(ESP_PLATFORM)
#include <esp_pthread.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#endif

#include "BaseClass.h"

namespace dh {

class State: public dh::BaseClass {
public:
    struct Config {
        std::string name;              /**< Name of the task */
        size_t stack_size_bytes{4096}; /**< Stack Size (B) allocated to the task. */
        size_t priority{0}; /**< Priority of the task, 0 is lowest priority on ESP / FreeRTOS.  */
        int core_id{-1};    /**< Core ID of the task, -1 means it is not pinned to any core.  */
    };

    typedef std::function<void()> callback_fn;

    struct Params {
        callback_fn callback;
        Config config;
    };

    explicit State(const Params p);

private:
    std::string name;
    Config config;
    callback_fn callback;

    bool running;
};






}
