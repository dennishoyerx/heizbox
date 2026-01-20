#include "app/App.h"
#include "heater/HeaterCycle.h"
#include "heater/HeaterState.h"
#include "core/DeviceState.h"
#include "utils/Logger.h"
#include "core/EventBus.h"
#include "app/HeatData.h"
#include "net/WebSocketManager.h"

namespace App {
    void bindToState() {
        auto& hs = HeaterState::instance();
        auto& ds = DeviceState::instance();
        
        /*ds.isHeating.addListener([&](bool isHeating) {
            WebSocketManager::instance().sendStatusUpdate(true, isHeating);
        });*/
    };
};
