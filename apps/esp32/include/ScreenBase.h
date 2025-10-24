// include/ScreenBase.h
#ifndef SCREENBASE_H
#define SCREENBASE_H

#include "Screen.h"
#include "DisplayManager.h"

// ============================================================================
// Screen Mixins - Wiederverwendbare Funktionalität
// ============================================================================

// Mixin für Timeout-basierte Screens (z.B. Screensaver)
class TimeoutMixin {
protected:
    TimeoutMixin(uint32_t timeoutMs) 
        : timeout_(timeoutMs), lastActivity_(millis()) {}
    
    void resetTimeout() { lastActivity_ = millis(); }
    
    bool isTimedOut() const {
        return (millis() - lastActivity_) > timeout_;
    }
    
    void setTimeout(uint32_t ms) { timeout_ = ms; }

private:
    uint32_t timeout_;
    uint32_t lastActivity_;
};

// Mixin für Animation-Support
class AnimationMixin {
protected:
    AnimationMixin(uint32_t durationMs) 
        : startTime_(millis()), duration_(durationMs), complete_(false) {}
    
    void resetAnimation() {
        startTime_ = millis();
        complete_ = false;
    }
    
    bool isAnimationComplete() const {
        return complete_ || (millis() - startTime_) >= duration_;
    }
    
    float getProgress() const {
        if (complete_) return 1.0f;
        uint32_t elapsed = millis() - startTime_;
        return std::min(1.0f, elapsed / static_cast<float>(duration_));
    }
    
    void completeAnimation() { complete_ = true; }

private:
    uint32_t startTime_;
    uint32_t duration_;
    bool complete_;
};

// ============================================================================
// Base Screen Templates
// ============================================================================

// Einfacher statischer Screen (z.B. Info-Screen)
class StaticScreen : public Screen {
public:
    StaticScreen(ScreenType type) : type_(type) {}
    
    void update() override {}  // Keine Updates nötig
    ScreenType getType() const override { return type_; }

protected:
    ScreenType type_;
};

// Screen mit Timeout (z.B. Screensaver, Confirmation)
class TimedScreen : public Screen, protected TimeoutMixin {
public:
    TimedScreen(ScreenType type, uint32_t timeoutMs, std::function<void()> callback = nullptr)
        : type_(type), TimeoutMixin(timeoutMs), callback_(callback) {}
    
    void update() override {
        if (isTimedOut() && callback_) {
            callback_();
        }
    }
    
    void onEnter() override { resetTimeout(); }
    
    ScreenType getType() const override { return type_; }

protected:
    ScreenType type_;
    std::function<void()> callback_;
};

// Screen mit Animation (z.B. Startup, Loading)
class AnimatedScreen : public Screen, protected AnimationMixin {
public:
    AnimatedScreen(ScreenType type, uint32_t durationMs, std::function<void()> callback = nullptr)
        : type_(type), AnimationMixin(durationMs), callback_(callback) {}
    
    void update() override {
        if (isAnimationComplete() && callback_) {
            callback_();
        }
    }
    
    ScreenType getType() const override { return type_; }

protected:
    ScreenType type_;
    std::function<void()> callback_;
};

#endif // SCREENBASE_H
