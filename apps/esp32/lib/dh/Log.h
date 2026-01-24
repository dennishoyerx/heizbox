#pragma once 
#include <string> 

namespace dh {

class Logger {
    public:

    enum class Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
    };

    struct BaseConfig {
        std::string name;
        Level level;
    };

    explicit Logger(BaseConfig bc);
};


}