#pragma once

#include <unordered_map>
#include <string>
#include "Observable.h"
#include "ui/base/UI.h"
#include "ui/base/ScreenTransition.h"
#include "hardware/input/InputManager.h"

class ScreenManager;

class Screen {
public:
    Screen();
    virtual ~Screen();

    // Pure virtual (muss implementiert werden)
    virtual void draw() = 0;
    virtual void update() = 0;
    virtual void handleInput(InputEvent event/*, std::function<void(InputEventType type, InputButton button)> handler*/) = 0;
    virtual ScreenType getType() const = 0;

    // Optional overrides
    virtual void onEnter() {}  // Called when screen becomes active
    virtual void onExit() {}   // Called when screen is left
    virtual bool needsRedraw() const { return false; }

    // Manager access
    void setManager(ScreenManager* mgr);
    void setUI(UI* ui);
    void clear();


protected:
    virtual void initState() {}

    ScreenManager* manager;
    UI* _ui;

    void dirty();

    template <typename T>
    void bindTo(T& member, Observable<T>& observable) {
        member = observable.get();
        observable.addListener([this, &member](T v) {
            member = v;
            dirty();
        });
    }

    template <typename T>
    void bind(Observable<T>& observable) {
        observable.addListener([this](T v) {
            dirty();
        });
    }

    template <typename... Ts>
    void bindMultiple(Observable<Ts>&... observables) {
        (bind(observables), ...);
    }


protected:
    bool input(InputEvent event,
            std::initializer_list<InputButton> buttons,
            std::initializer_list<InputEventType> types);
private:
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
