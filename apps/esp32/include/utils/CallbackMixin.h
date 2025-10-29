#ifndef CALLBACK_MIXIN_H
#define CALLBACK_MIXIN_H

#include <functional>

template<typename T = void>
class CallbackMixin {
protected:
    std::function<void(T)> callback_;
    bool hasCallback_ = false;

public:
    void setCallback(std::function<void(T)> callback) {
        callback_ = std::move(callback);
        hasCallback_ = true;
    }

    bool hasCallback() const {
        return hasCallback_;
    }

    void invokeCallback(T arg) {
        if (hasCallback_) {
            callback_(arg);
        }
    }
};

template<>
class CallbackMixin<void> {
protected:
    std::function<void()> callback_;
    bool hasCallback_ = false;

public:
    void setCallback(std::function<void()> callback) {
        callback_ = std::move(callback);
        hasCallback_ = true;
    }

    bool hasCallback() const {
        return hasCallback_;
    }

    void invokeCallback() {
        if (hasCallback_) {
            callback_();
        }
    }
};

#endif // CALLBACK_MIXIN_H