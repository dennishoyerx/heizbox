#pragma once

#include <functional>
#include <vector>
#include <map>
#include <Preferences.h>
#include <type_traits>
#include <nvs_flash.h> 
#include <mutex>
#include <atomic>

template<typename T>
class Observable {
public:
    using ListenerId = uint32_t;
    using Listener = std::function<void(const T&)>;

    Observable() = default;
    explicit Observable(T initialValue) : value_(std::move(initialValue)) {}

    
    // Set value and notify listeners (thread-safe)
    T set(const T& newValue) {
        std::map<ListenerId, Listener> listenersCopy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (value_ == newValue) return value_;
            value_ = newValue;
            listenersCopy = listeners_; // copy under lock
        }
        // notify without holding the lock
        for (auto const& kv : listenersCopy) {
            try {
                kv.second(value_);
            } catch (...) {
                // swallow exceptions - embedded environment
            }
        }
        return value_;
    }

    // Set without notifying listeners (for initialization)
    void setSilent(const T& newValue) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ = newValue;
    }

    // Return a copy of the value (safe)
    T get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }

    // Implicit conversion to T (copy)
    operator T() const { return get(); }

    // Register listener, returns id to remove later
    ListenerId addListener(Listener listener) {
        std::lock_guard<std::mutex> lock(mutex_);
        ListenerId id = nextListenerId_++;
        listeners_.emplace(id, std::move(listener));
        return id;
    }

    void removeListener(ListenerId id) {
        std::lock_guard<std::mutex> lock(mutex_);
        listeners_.erase(id);
    }

    // Update with transformation function
    template<typename Fn>
    T update(Fn&& fn) {
        T cur = get();
        T next = fn(cur);
        return set(next);
    }

private:
    mutable std::mutex mutex_;
    T value_{};
    std::map<ListenerId, Listener> listeners_;
    std::atomic<ListenerId> nextListenerId_{0};
};

// ============================================================================ 
// Persisted Observable - Mit NVS-Backing
// ============================================================================ 

template<typename T>
class PersistedObservable : public Observable<T> {
public:
    PersistedObservable(const char* ns, const char* key, T defaultValue)
        : Observable<T>(defaultValue), namespace_(ns), key_(key) {
        load();
    }

    T set(T newValue, bool persist = true) {
        Observable<T>::set(newValue);
        if (persist) save();
        return newValue;
    }

    void load() {
        Preferences prefs;
        if (!prefs.begin(namespace_, true)) {
            return;
        }

        T value = this->get(); // Initialize with current (default) value

        if constexpr (std::is_same_v<T, bool>) {
            value = static_cast<T>(prefs.getBool(key_, value));
        } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            int32_t v = prefs.getInt(key_, static_cast<int32_t>(value));
            value = static_cast<T>(v);
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            uint32_t v = prefs.getUInt(key_, static_cast<uint32_t>(value));
            value = static_cast<T>(v);
        } else if constexpr (std::is_floating_point_v<T>) {
            float v = prefs.getFloat(key_, static_cast<float>(value));
            value = static_cast<T>(v);
        } else {
            // unsupported type for NVS
        }

        this->setSilent(value);
        prefs.end();
    }

    void save() {
        Preferences prefs;
        if (!prefs.begin(namespace_, false)) {
            //logPrint("StateManager", "ERROR: Failed to open NVS '%s' read-write.", namespace_);
            return;
        }

        T value = this->get();

        if constexpr (std::is_same_v<T, bool>) {
            prefs.putBool(key_, value);
        } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
            prefs.putInt(key_, static_cast<int32_t>(value));
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            prefs.putUInt(key_, static_cast<uint32_t>(value));
        } else if constexpr (std::is_floating_point_v<T>) {
            prefs.putFloat(key_, static_cast<float>(value));
        } else {
            // unsupported type for NVS
        }

        prefs.end(); // end() must be called to commit. 
    }

private:
    const char* namespace_;
    const char* key_;
};
