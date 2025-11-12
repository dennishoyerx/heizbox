#ifndef EVENTBUS_H
#define EVENTBUS_H
#include <functional>
#include <map>
#include <vector>
#include <Arduino.h>
#include <mutex>

enum class EventType {
    SENSOR_TRIGGERED,
    HEATER_STARTED,
    HEATER_STOPPED,
    SCREEN_CHANGED,
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    STATS_UPDATED
};

struct Event {
    EventType type;
    void* data;  // optional payload pointer
};

class EventBus {
public:
    using Callback = std::function<void(const Event&)>;

    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    void subscribe(EventType type, Callback cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers[type].push_back(cb);
    }

    void publish(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers.find(event.type);
        if (it != subscribers.end()) {
            for (auto& cb : it->second) {
                // Dispatch async (FreeRTOS task)
                xTaskCreatePinnedToCore(
                    [](void* arg) {
                        auto pair = static_cast<std::pair<Callback, Event>*>(arg);
                        pair->first(pair->second);
                        delete pair;
                        vTaskDelete(nullptr);
                    },
                    "evt", 2048,
                    new std::pair<Callback, Event>(cb, event),
                    1, nullptr, APP_CPU_NUM
                );
            }
        }
    }

private:
    std::map<EventType, std::vector<Callback>> subscribers;
    std::mutex mutex_;
};

#endif