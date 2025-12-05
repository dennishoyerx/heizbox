#include "core/EventBus.h"

EventBus::EventBus() {
    _instance = this;
}

EventBus* EventBus::_instance = nullptr;

EventBus* EventBus::instance() {
    return _instance;
}
