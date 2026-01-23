#include "Task.h"

using namespace dh;


Task::Task(const Params p): config(p.config), callback(std::move(p.callback)), name(p.config.name) {

};

bool Task::start() {
    return createTask(false);
};

bool Task::run() {
    return createTask(true);
};

bool Task::createTask(bool cleanTask) {
    BaseType_t ok = xTaskCreatePinnedToCore(
        cleanTask ?
        [](void* arg) {
            Task* self = static_cast<Task*>(arg);
            self->callback();
            vTaskDelete(nullptr);
        }
        : [](void* arg) {
            Task* self = static_cast<Task*>(arg);
            self->callback();
        },
        name.c_str(),
        config.stack_size_bytes,
        this,
        config.priority,
        &task,
        config.core_id
    );

    return running = ok == pdPASS;
}

void Task::stop() {
    if (!running) return;
    vTaskDelete(task);
    running = false;
};