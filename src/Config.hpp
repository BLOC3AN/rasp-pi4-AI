#pragma once
#include <string>
#include <fstream>
#include <map>
#include <iostream>
#include <algorithm>

class Config {
    std::map<std::string, std::string> settings;
public:
    Config(const std::string& filename = ".env") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            // Trim whitespace from both ends
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // Trim key and value
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                
                settings[key] = value;
            }
        }
    }

    std::string get(const std::string& key, const std::string& def = "") {
        if (settings.count(key)) return settings.at(key);
        return def;
    }

    int getInt(const std::string& key, int def = 0) {
        if (settings.count(key)) {
            try {
                return std::stoi(settings.at(key));
            } catch (...) {
                return def;
            }
        }
        return def;
    }
    
    long getHex(const std::string& key, long def = 0) {
        if (settings.count(key)) {
            try {
                return std::stol(settings.at(key), nullptr, 16);
            } catch (...) {
                return def;
            }
        }
        return def;
    }
};
