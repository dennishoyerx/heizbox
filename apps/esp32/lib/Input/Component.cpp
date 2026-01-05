#pragma once

#include "Component.h"
Component::Component(Position pos, Dimension dim, CompCallback cb) {
    position = pos;
    dimension = dim;
    callback = cb;
}

Component::~Component() {
    for (const auto& unsub : unsubscribe_callbacks) {
        unsub();
    }
}

// Copy constructor
Component::Component(const Component& other)
    : position(other.position), dimension(other.dimension), callback(other.callback), dirty(other.dirty) {
    // Do not copy listeners. The new component must attach its own.
}

// Copy assignment operator
Component& Component::operator=(const Component& other) {
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

template <typename... Observables> Component* Component::attachStates(Observables&... observables) {
    (attachState(observables), ...);
    return this;
};

template <typename T> void Component::attachState(Observable<T>& observable) {
    auto listenerId = observable.addListener([this](T value) { dirty = true; });
    unsubscribe_callbacks.emplace_back([&observable, listenerId]() { observable.removeListener(listenerId); });
}
