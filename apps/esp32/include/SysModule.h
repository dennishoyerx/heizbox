#pragma once
#include <Arduino.h>
#include "Config.h"

struct SysModuleBoot {
    String key;
    uint32_t time;
};

class SysModules {
public:
    using bootCallback = std::function<void()>;
    void add(SysModuleBoot boot) {
        if (!DebugFlags::LOG_BOOT) return;
        modules.push_back(boot);
    }

    static bootCallback booting(String key) {
        u_int32_t start = millis();
        return [&start, key]() {
            SysModules::instance().add({key, millis() - start});
        };
    }

    const std::vector<SysModuleBoot>& get() const {
        return modules;
    }

    static SysModules& instance() {
        static SysModules instance;
        return instance;
    }

private:
    SysModules() = default;
    std::vector<SysModuleBoot> modules;
};
