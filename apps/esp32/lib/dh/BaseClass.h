#pragma once 
#include "Log.h"
#include <string>

namespace dh {

class BaseClass {
public:
    struct Config {
        std::string_view tag;
        Logger::Level level;
    };
    

    explicit BaseClass(std::string_view tag, Logger::Level level = Logger::Level::WARN): logger({.tag = tag, .level = level}) {};
    explicit BaseClass(const Logger::Config config) : logger(config) {}

private:
protected:
    Logger logger;
};


}