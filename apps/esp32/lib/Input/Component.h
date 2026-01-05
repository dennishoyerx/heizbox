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
    Component(Position pos = {0, 0}, Dimension dim = {0, 0}, CompCallback cb = nullptr);

    ~Component();

    // Copy constructor
    Component(const Component& other);

    // Copy assignment operator
    Component& operator=(const Component& other);

    template<typename... Observables>
    Component* attachStates(Observables&... observables);

    template<typename T>
    void attachState(Observable<T>& observable);


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
