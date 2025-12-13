#include "ui/base/Screen.h"
#include "ui/base/ScreenManager.h"
#include "ui/base/UI.h"
#include "hardware/input/InputManager.h"


Screen::Screen() : manager(nullptr) {
    initState();
}

Screen::~Screen() {
}

void Screen::setManager(ScreenManager* mgr) {
    manager = mgr;
}

void Screen::setUI(UI* ui) {
    _ui = ui;
}

void Screen::clear() {
    if (_ui) {
        _ui->clear();
    }
    dirty();
}

void Screen::dirty() {
    if (manager) {
        manager->setDirty();
    }
}


bool Screen::input(InputEvent event,
            std::initializer_list<InputButton> buttons,
            std::initializer_list<InputEventType> types) {
    bool bMatch = false;
    for (auto b : buttons) {
        if (event.button == b) {
            bMatch = true;
            break;
        }
    }

    if (!bMatch) return false;

    for (auto t : types) {
        if (event.type == t) {
            return true;
        }
    }

    return false;
}

template <typename... Ts>
void Screen::bindMultiple(Observable<Ts>&... observables) {
    (bind(observables), ...);
}

template <typename T>
void Screen::bind(Observable<T>& observable) {
    observable.addListener([this](T v) {
        dirty();
    });
}


template <typename T>
void Screen::bindTo(T& member, Observable<T>& observable) {
    member = observable.get();
    observable.addListener([this, &member](T v) {
        member = v;
        dirty();
    });
}