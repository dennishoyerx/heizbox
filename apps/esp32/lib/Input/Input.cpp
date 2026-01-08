#include <Arduino.h>
namespace Input {
    enum Events {
        PRESS,
        RELEASE,
        HOLD
    };


    class Source {
    public:
        virtual bool isPressed(uint8_t idx) = 0;
    };

    class InputController {
    public:
        InputController();
    };
};