#pragma once

#include <DisplayDriver.h>
#include <ui/base/SurfaceFactory.h>
#include <unordered_map>
#include <string>


struct Position {
    int16_t x;
    int16_t y;
};
struct Dimension {
    int16_t w;
    int16_t h;
};

struct State {

};

using CompCallback = std::function<void(RenderSurface&)>;
std::function<int(int, int)> callback = [](int a, int b) { return a + b; };


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

class Components {
public:
    Components(SurfaceFactory* factory): _surfaceFactory(factory) {

    };

    void add(std::string key, Component comp) {
        comps[key] = comp;
    };

    void renderComponent(const std::string& key) {
        auto it = comps.find(key);
        if (it != comps.end()) {
            // it->second ist die Comp-Instanz
            // it->second.renderExecute(); // Dies könnte die beabsichtigte Aktion sein
        }
    };

private: 
    SurfaceFactory* _surfaceFactory;
    std::unordered_map<std::string, Component> comps;
};


class UI {
public:
    UI(DisplayDriver* driver);

    // Surface
    RenderSurface createSurface(int16_t w, int16_t h);
    void releaseSurface(RenderSurface& s);
    void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY, SurfaceCallback cb, bool clear = true);
    void withSurface(int16_t w, int16_t h, int16_t targetX, int16_t targetY,
                     const std::unordered_map<std::string, StateValue>& state,
                     SurfaceCallback cb, bool clear = true);

    void clear();
    
    // Force all surfaces to redraw on next render cycle
    void forceRedraw();
    
    // Invalidate all cached surface states
    void invalidateAll();

    Component createComp(Position pos = {0, 0}, Dimension dim = {0, 0}, CompCallback cb = nullptr) {
        Component comp = Component(pos, dim, cb);

        _comps.push_back(comp);

        return comp;
    };

private:
    DisplayDriver* _driver;
    SurfaceFactory _surfaceFactory;
    std::vector<Component> _comps;
};