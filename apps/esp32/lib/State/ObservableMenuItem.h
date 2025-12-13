#pragma once
#include "Menu.h"
#include "Observable.h"
#include <functional>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <type_traits>
#include <Arduino.h>

// Generic value item that binds directly to Observable<T> (or PersistedObservable<T>).
template<typename T>
class ObservableValueItem : public IMenuItem {
public:
    using Formatter = std::function<String(const T&)>;

    // Bind to an Observable<T>
    ObservableValueItem(String name, Observable<T>& obs, T minv, T maxv, T step, Formatter fmt = nullptr)
        : name_(std::move(name)), obs_(&obs), min_(minv), max_(maxv), step_(step), fmt_(fmt) {}

    String name() const override { return name_; }

    String valueString() const override {
        T v = obs_->get();
        if (fmt_) return fmt_(v);

        if constexpr (std::is_floating_point<T>::value) {
            std::ostringstream os;
            // precision decision: if step < 1.0 show 1..2 decimals
            int prec = (step_ < 0.1) ? 2 : (step_ < 1.0 ? 1 : 0);
            os << std::fixed << std::setprecision(prec) << v;
            return String(os.str().c_str());
        } else {
            return String(std::to_string(static_cast<long long>(v)).c_str());

        }
    }

    void increment() override {
        T v = obs_->get();
        v = add(v, step_);
        v = clamp(v);
        obs_->set(v); // will notify and persist if PersistedObservable
    }
    void decrement() override {
        T v = obs_->get();
        v = add(v, -step_);
        v = clamp(v);
        obs_->set(v);
    }

private:
    T clamp(T v) const {
        if (v < min_) return min_;
        if (v > max_) return max_;
        return v;
    }

    T add(T a, T b) const {
        if constexpr (std::is_floating_point<T>::value) {
            double r = static_cast<double>(a) + static_cast<double>(b);
            // snap to grid defined by step_ to avoid drift
            double steps = std::round(r / static_cast<double>(step_));
            return static_cast<T>(steps * static_cast<double>(step_));
        } else {
            return a + b;
        }
    }

    String name_;
    Observable<T>* obs_;
    T min_, max_, step_;
    Formatter fmt_;
};

// Enum-like menu item for Observables holding integer enums
class ObservableEnumItem : public IMenuItem {
public:
    using Getter = std::function<int()>;
    using Setter = std::function<void(int)>;

    ObservableEnumItem(String name, Observable<int>& obs, const std::vector<String>& labels)
        : name_(std::move(name)), obs_(&obs), labels_(labels) {}

    String name() const override { return name_; }

    String valueString() const override {
        int v = obs_->get();
        if (v >= 0 && static_cast<size_t>(v) < labels_.size()) return labels_[v];
        return (String) v;
    }

    void increment() override {
        int v = obs_->get();
        v = (v + 1) % static_cast<int>(labels_.size());
        obs_->set(v);
    }
    void decrement() override {
        int v = obs_->get();
        v = (v - 1 + static_cast<int>(labels_.size())) % static_cast<int>(labels_.size());
        obs_->set(v);
    }

private:
    String name_;
    Observable<int>* obs_;
    std::vector<String> labels_;
};