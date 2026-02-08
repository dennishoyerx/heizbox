#pragma once
#include <functional>
#include "RenderSurface.h"

using RenderFn = std::function<void(RenderSurface)>;
struct Component {
    struct Config {
        RenderFn render;
    };

    Component(Config conf);

};

struct Row {
    Row Column(int w, Component c);
};

struct Element {
    int height;
    int width;
};


struct Area: public Element {
    Row row(int height);
    Row grid(int height);
};



Component::Config TempComponent = {
    .render = [](RenderSurface s) {
        s.text(0, 0, "Text");
    }
};

void FireScreen(Area a) {
    a.row(40)
    .Column(50, TempComponent);

};

