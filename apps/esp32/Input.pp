#include "driver/input/InputManager.h"
#include "driver/input/InputHandler.h"

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