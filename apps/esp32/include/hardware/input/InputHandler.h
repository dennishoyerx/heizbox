// include/hardware/InputHandler.h
#pragma once

#include "forward.h"

#include "hardware/input/InputManager.h"
#include "ui/base/ScreenManager.h" // CONVERTED: Use forward.h
#include "ui/base/Screen.h"
#include "ui/base/ScreenTransition.h"

/**
 * @brief Handles input events and dispatches them to the appropriate screens or global shortcuts.
 *
 * This class centralizes the logic for processing raw input events from the InputManager,
 * checking for global shortcuts, and then passing the event to the currently active screen.
 */
class InputHandler {
public:
    /**
     * @brief Constructs an InputHandler instance.
     * @param screenManager Reference to the ScreenManager instance.
     */
    InputHandler(ScreenManager& screenManager);

    /**
     * @brief Processes an input event, checking for global shortcuts and dispatching to the screen manager.
     * @param event The InputEvent to handle.
     */
    void handleInput(InputEvent event);

    void setDebug(bool val) { debug = val; }

private:
    ScreenManager& screenManager;
    bool debug = false;

    /**
     * @brief Checks for and handles global shortcuts.
     * @param event The InputEvent to check.
     * @return True if a global shortcut was handled, false otherwise.
     */
    bool handleGlobalShortcuts(InputEvent event);
};
