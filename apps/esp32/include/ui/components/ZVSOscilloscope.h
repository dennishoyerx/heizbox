#pragma once

//#include "forward.h"

#include "heater/ZVSDriver.h" // CONVERTED: Use forward.h
#include "ui/ColorPalette.h"
#include "ui/base/Screen.h"

class ZVSOscilloscope {
public:
    ZVSOscilloscope(ZVSDriver* zvs, int displayWidth = 280)
        : zvs(zvs), width(displayWidth) {
        buffer.reserve(width);
    }
    
    void update() {
        // Sample current state
        uint8_t value = zvs->isPhysicallyOn() ? 255 : 0;
        buffer.push_back(value);
        
        // Keep only last N samples
        if (buffer.size() > width) {
            buffer.erase(buffer.begin());
        }
    }
    
    void draw(RenderSurface& s, int x, int y, int h) {
        // Draw waveform
        for (size_t i = 1; i < buffer.size(); i++) {
            int x1 = x + i - 1;
            int x2 = x + i;
            int y1 = y + h - (buffer[i-1] * h / 255);
            int y2 = y + h - (buffer[i] * h / 255);
            
            s.sprite->drawLine(x1, y1, x2, y2, COLOR_SUCCESS);
        }
        
        // Draw baseline
        s.sprite->drawLine(x, y + h, x + width, y + h, COLOR_TEXT_SECONDARY);
    }
    
    void reset() {
        buffer.clear();
    }

private:
    ZVSDriver* zvs;
    std::vector<uint8_t> buffer;
    int width;
};
