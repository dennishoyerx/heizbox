#include "hardware/input/InputManager.h"
#include "hardware/input/InputHandler.h"

class InputController {
public:
    InputController(InputManager& inputManager, InputHandler& inputHandler)
        : inputManager(inputManager), inputHandler(inputHandler) {}

    void setup() {
        inputManager.setCallback([this](InputEvent event) {
            this->inputHandler.handleInput(event);
        });
        inputManager.setup();
    }

    void update() {
        inputManager.update();
    }

private:
    InputManager& inputManager;
    InputHandler& inputHandler;

};