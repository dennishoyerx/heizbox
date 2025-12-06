#include "core/EventBus.h"

EventBus& EventBus::instance() {
    static EventBus instance;
    return instance;
}