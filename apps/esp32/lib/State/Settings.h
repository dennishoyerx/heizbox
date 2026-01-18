#pragma once

#include <Arduino.h>

template <typename T>
struct Setting {
    String ns;
    String key;
    T initialValue;
};

struct Settings {
    Setting<bool> isOnline{"device", "online", false};
};

class SettingsManager {};

namespace Settings {
    SettingsManager Manager;
};