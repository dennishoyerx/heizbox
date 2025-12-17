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


class Comp {
public:
    Comp(Position pos = {0, 0}, Dimension dim = {0, 0}, CompCallback cb = nullptr) {
        position = pos;
        dimension = dim;
        callback = cb;
    }

    template<typename T>
    void use(Observable<T>& observable) {};
    
    template<typename T>
    Comp* attachState(Observable<T>& observable) {
        return this;
    };



private:
    Position position;
    Dimension dimension;
    CompCallback callback;
    UI* _ui;

    void renderExecute() {
        //_ui->withSurface(dimension.w, dimension.h, position.x, position.y, callback); 
    };
};

class Components {
public:
    Components(SurfaceFactory* factory): _surfaceFactory(factory) {

    };

    void add(std::string key, Comp comp) {
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
    std::unordered_map<std::string, Comp> comps;
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

    Comp createComp(Position pos = {0, 0}, Dimension dim = {0, 0}, CompCallback cb = nullptr) {
        Comp comp = Comp(pos, dim, cb);

        _comps.push_back(comp);

        return comp;
    };

private:
    DisplayDriver* _driver;
    SurfaceFactory _surfaceFactory;
    std::vector<Comp> _comps;
};