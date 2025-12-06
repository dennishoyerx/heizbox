#pragma once

#include <functional>
#include <map>
#include <vector>
#include <Arduino.h>
#include <mutex>
#include <memory>

enum class EventType {
    OTA_UPDATE_STARTED,
    OTA_UPDATE_FINISHED,
    OTA_UPDATE_FAILED,
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
    std::shared_ptr<void> data = nullptr;
};

template<typename T>
using CallbackT = std::function<void(const T&)>;


class EventBus {
public:
    using UntypedCallback = std::function<void(const Event&)>;

    // ---------------------------
    // Untyped Subscribe (wie bisher)
    // ---------------------------
    void subscribe(EventType type, UntypedCallback cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        untypedSubscribers[type].push_back(cb);
    }

    // ---------------------------
    // Typed Subscribe
    // ---------------------------
    template<typename T>
    void subscribe(EventType type, CallbackT<T> cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        typedSubscribers[type].push_back([cb](const void* data){
            cb(*static_cast<const T*>(data));
        });
    }

    // ---------------------------
    // Untyped Publish
    // ---------------------------
    void publish(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);

        // Untyped Dispatch
        if (auto it = untypedSubscribers.find(event.type); it != untypedSubscribers.end()) {
            for (auto& cb : it->second) {
                UntypedCallback copied = cb;
                Event evCopy = event;

                xTaskCreatePinnedToCore(
                    [](void* arg){
                        auto* p = static_cast<std::pair<UntypedCallback, Event>*>(arg);
                        p->first(p->second);
                        delete p;
                        vTaskDelete(nullptr);
                    },
                    "evt_untyped", 4096,
                    new std::pair<UntypedCallback, Event>(copied, evCopy),
                    1, nullptr, APP_CPU_NUM
                );
            }
        }

        // Typed Dispatch (falls vorhanden)
        if (auto it = typedSubscribers.find(event.type); it != typedSubscribers.end()) {
            for (auto& cb : it->second) {

                auto dataCopy = event.data;   // shared_ptr<uint8_t etc> wird kopiert!

                using TaskArg = std::pair<std::function<void(const void*)>, std::shared_ptr<void>>;
                auto* arg = new TaskArg(cb, dataCopy);

                xTaskCreatePinnedToCore(
                    [](void* v){
                        auto* p = static_cast<TaskArg*>(v);
                        p->first(p->second.get());
                        delete p;
                        vTaskDelete(nullptr);
                    },
                    "evt_typed",
                    4096,
                    arg,
                    1,
                    nullptr,
                    APP_CPU_NUM
                );
            }
        }
    }

    // ---------------------------
    // Typed Publish (komfortabel)
    // ---------------------------
    template<typename T>
    void publish(EventType type, const T& payload) {
        Event ev{ type, std::make_shared<T>(payload) };
        publish(ev);
    }

    static EventBus& instance();

private:
    EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    std::map<EventType, std::vector<UntypedCallback>> untypedSubscribers;
    std::map<EventType, std::vector<std::function<void(const void*)>>> typedSubscribers;
    std::mutex mutex_;
};
