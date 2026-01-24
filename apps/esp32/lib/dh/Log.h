#pragma once 
#include <string> 
#include <Arduino.h> 

namespace dh {

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
    };

    struct Config {
        std::string_view tag;
        Level level;
    };

    explicit Logger(Config config): _tag(config.tag), _level(config.level) {}

    void log(std::string_view msg, std::string_view level) {
        Serial.println(formatMessage(msg, level).c_str());
    }

    void info(std::string msg) { if (_level >= Level::INFO) log(msg, "INFO"); }
    void warn(std::string msg) { if (_level >= Level::WARN) log(msg, "WARN"); }
    void error(std::string msg) { if (_level >= Level::ERROR) log(msg, "ERROR"); }

    void setLevel(Level level) { _level = level; }

    private:
    std::string _tag;
    Level _level;
    
    std::string formatMessage(std::string_view msg, std::string_view level) {
        return "[" + std::string(level) + "] " + "[" + _tag + "]  " + std::string(msg);
    }

    bool checkLevel(Level l) { return l >= _level; }
};


}