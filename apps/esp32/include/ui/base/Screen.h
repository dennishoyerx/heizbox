// include/Screen.h
#ifndef SCREEN_H
#define SCREEN_H

#include <unordered_map>
#include <string>
#include "ui/base/ScreenTransition.h"
#include "ui/base/UIComponent.h"
#include "hardware/input/InputManager.h"
#include "utils/CallbackMixin.h" // For TimeoutMixin

class DisplayManager;
class ScreenManager;
class DisplayDriver;

class Components {
public:
    void addComponent(const std::string& id, std::unique_ptr<UIComponent> c) {
        components_[id] = std::move(c);
    }

    void removeComponent(const std::string& id) {
        components_.erase(id);
    }

    UIComponent* getComponent(const std::string& id) const {
        auto it = components_.find(id);
        return it != components_.end() ? it->second.get() : nullptr;
    }

    void forEach(const std::function<void(UIComponent&, DisplayDriver&)>& fn, DisplayDriver& display) {
        for (auto it = components_.begin(); it != components_.end(); ++it) {
            fn(*it->second, display);
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<UIComponent>> components_;
};


// Base Screen mit Common Functionality
class Screen {
public:
    Screen();
    virtual ~Screen();

    // Pure virtual (muss implementiert werden)
    virtual void draw(DisplayDriver& display) = 0;
    virtual void update() = 0;
    virtual void handleInput(InputEvent event) = 0;
    virtual ScreenType getType() const = 0;

    // Optional overrides
    virtual void onEnter() {}  // Called when screen becomes active
    virtual void onExit() {}   // Called when screen is left
    virtual bool needsRedraw() const { return false; }

    // State management (einfacher Key-Value Store)
    void setState(const std::string& key, int value);
    int getState(const std::string& key, int defaultValue = 0) const;

    // Manager access
    void setManager(ScreenManager* mgr);



protected:
    virtual void initState() {}
    ScreenManager* manager;
    Components* components;

    // Helper für häufige Operationen
    void markDirty();
    void centerText(DisplayDriver& display, int16_t y, const char* text,
                    uint16_t color, uint8_t size = 2);

private:
    std::unordered_map<std::string, int> state_;
};

// Mixin for timeout functionality
class TimeoutMixin {
protected:
    unsigned long timeoutDuration_ = 0;
    unsigned long timeoutStart_ = 0;
    bool timeoutActive_ = false;
    std::function<void()> onTimeoutCallback_ = nullptr;

public:
    void startTimeout(unsigned long duration, std::function<void()> callback) {
        timeoutDuration_ = duration;
        onTimeoutCallback_ = std::move(callback);
        timeoutStart_ = millis();
        timeoutActive_ = true;
    }

    void stopTimeout() {
        timeoutActive_ = false;
    }

    void resetTimeout() {
        if (timeoutActive_) {
            timeoutStart_ = millis();
        }
    }

    void updateTimeout() {
        if (timeoutActive_ && (millis() - timeoutStart_ >= timeoutDuration_)) {
            timeoutActive_ = false;
            if (onTimeoutCallback_) {
                onTimeoutCallback_();
            }
        }
    }
};

// Convenience class for screens with timeout
class TimedScreen : public Screen, protected TimeoutMixin {
public:
    TimedScreen() = default;
    virtual ~TimedScreen() = default;

    void update() override {
        updateTimeout();
    }
};

// Mixin for animation functionality
class AnimationMixin {
protected:
    unsigned long animationDuration_ = 0;
    unsigned long animationStart_ = 0;
    bool animationActive_ = false;
    std::function<void(float)> onAnimateCallback_ = nullptr; // progress 0.0 - 1.0
    std::function<void()> onAnimationCompleteCallback_ = nullptr;
    float currentProgress_ = 0.0f;

public:
    float getProgress() const { return currentProgress_; }
    void startAnimation(unsigned long duration, std::function<void(float)> animateCb, std::function<void()> completeCb = nullptr) {
        animationDuration_ = duration;
        onAnimateCallback_ = std::move(animateCb);
        onAnimationCompleteCallback_ = std::move(completeCb);
        animationStart_ = millis();
        animationActive_ = true;
    }

    void stopAnimation() {
        animationActive_ = false;
    }

    void updateAnimation() {
        if (animationActive_) {
            unsigned long elapsed = millis() - animationStart_;
            float progress = static_cast<float>(elapsed) / animationDuration_;
            currentProgress_ = progress;

            if (progress >= 1.0f) {
                progress = 1.0f;
                animationActive_ = false;
                if (onAnimateCallback_) onAnimateCallback_(progress);
                if (onAnimationCompleteCallback_) onAnimationCompleteCallback_();
            } else {
                if (onAnimateCallback_) onAnimateCallback_(progress);
            }
        }
    }
};

// Convenience class for screens with animation
class AnimatedScreen : public Screen, protected AnimationMixin {
public:
    AnimatedScreen() = default;
    virtual ~AnimatedScreen() = default;

    void update() override {
        updateAnimation();
    }

    void forceCompleteAnimation() {
        animationActive_ = false;
        if (onAnimationCompleteCallback_) {
            onAnimationCompleteCallback_();
        }
    }
};

#endif
