#include "ui/base/Screen.h"
#include "ui/base/ScreenManager.h"
#include "ui/base/UI.h"



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

void Screen::redrawUI() {
    if (_ui) {
        _ui->forceRedraw();
    }
}

void Screen::clear() {
    if (_ui) {
        _ui->clear();
    }
}

void Screen::dirty() {
    if (manager) {
        manager->setDirty();
    }
}
