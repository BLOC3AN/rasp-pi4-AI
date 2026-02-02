#pragma once
#include <iostream>
#include <string>
#include <ctime>

enum class LogLevel { INFO, WARNING, ERROR, DEBUG };

class Logger {
public:
    static void log(LogLevel level, const std::string& message) {
        std::time_t now = std::time(nullptr);
        char timestamp[20];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        
        std::string levelStr;
        switch (level) {
            case LogLevel::INFO:    levelStr = "\033[32m[INFO]\033[0m"; break;
            case LogLevel::WARNING: levelStr = "\033[33m[WARN]\033[0m"; break;
            case LogLevel::ERROR:   levelStr = "\033[31m[ERR ]\033[0m"; break;
            case LogLevel::DEBUG:   levelStr = "\033[36m[DEBG]\033[0m"; break;
        }
        
        std::cout << timestamp << " " << levelStr << " " << message << std::endl;
    }
};
