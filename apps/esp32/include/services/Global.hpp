#pragma once

#include "driver/input/InputManager.h"


class DeviceLock {
    bool isLocked;
    bool encoderPushed;

public:
    void handleEncoderEvent(InputEvent event) {
        if (event.button == ROTARY_ENCODER && event.type == PRESS) isLocked = true;
    }


};