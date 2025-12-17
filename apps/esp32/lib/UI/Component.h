#pragma once

#include <Arduino.h>
#include <unordered_map>
#include <string>

#include "RenderSurface.h"
#include <Observable.h>

struct Position {
    int16_t x;
    int16_t y;
};
struct Dimension {
    int16_t w;
    int16_t h;
};

using CompCallback = std::function<void(RenderSurface&)>;


template<typename T>
void useState(Observable<T>& observable) {};


class Component {
public:
    Component(Position pos = {0, 0}, Dimension dim = {0, 0}, CompCallback cb = nullptr) {
        position = pos;
        dimension = dim;
        callback = cb;
    }

    ~Component() {
        for (const auto& unsub : unsubscribe_callbacks) {
            unsub();
        }
    }

    // Copy constructor
    Component(const Component& other)
        : position(other.position), dimension(other.dimension), callback(other.callback), dirty(other.dirty) {
        // Do not copy listeners. The new component must attach its own.
    }

    // Copy assignment operator
    Component& operator=(const Component& other) {
        if (this != &other) {
            // Remove old listeners
            for (const auto& unsub : unsubscribe_callbacks) {
                unsub();
            }
            unsubscribe_callbacks.clear();

            position = other.position;
            dimension = other.dimension;
            callback = other.callback;
            dirty = other.dirty;
        }
        return *this;
    }

    template<typename... Observables>
    Component* attachStates(Observables&... observables) {
        (attachState(observables), ...);
        return this;
    };

    template<typename T>
    void attachState(Observable<T>& observable) {
        auto listenerId = observable.addListener([this](T value) {
            dirty = true;
        });
        unsubscribe_callbacks.emplace_back([&observable, listenerId]() {
            observable.removeListener(listenerId);
        });
    }


private:
    Position position;
    Dimension dimension;
    CompCallback callback;
    bool dirty = true;
    std::vector<std::function<void()>> unsubscribe_callbacks;


    void renderExecute() {
        //_ui->withSurface(dimension.w, dimension.h, position.x, position.y, callback); 
    };
};
