#include <Arduino.h>

namespace Input {

    class Manager {};

    template <typename T>
    class Source {
    public:
        virtual bool isPressed(T b) = 0;
    };

}