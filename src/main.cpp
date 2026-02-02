#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include "Config.hpp"
#include "Logger.hpp"
#include "OledDisplay.hpp"
#include "OcrEngine.hpp"

bool keepRunning = true;

void signalHandler(int signum) {
    Logger::log(LogLevel::INFO, "Interrupt signal received. Stopping...");
    keepRunning = false;
}

int main() {
    signal(SIGINT, signalHandler);

    try {
        Logger::log(LogLevel::INFO, "=== Raspberry Pi 4 Realtime OCR System ===");
        
        Config cfg(".env");
        std::string i2cDev = cfg.get("I2C_DEVICE", "/dev/i2c-1");
        uint8_t oledAddr = (uint8_t)cfg.getHex("OLED_ADDR", 0x3C);
        int camIdx = cfg.getInt("CAMERA_INDEX", 0);
        std::string lang = cfg.get("OCR_LANG", "vie+eng");
        int width = cfg.getInt("FRAME_WIDTH", 320);
        int height = cfg.getInt("FRAME_HEIGHT", 240);
        int interval = cfg.getInt("OCR_INTERVAL_MS", 100);

        // 1. Initialize Display
        OledDisplay oled(i2cDev, oledAddr);
        oled.showText("System Starting", "Please wait...");

        // 2. Initialize Camera
        cv::VideoCapture cap(camIdx, cv::CAP_V4L2);
        if (!cap.isOpened()) {
            Logger::log(LogLevel::WARNING, "Could not open camera with CAP_V4L2, trying default...");
            cap.open(camIdx);
        }
        
        if (!cap.isOpened()) {
            Logger::log(LogLevel::ERROR, "Could not open camera index " + std::to_string(camIdx));
            oled.showText("CAM ERROR", "Check Conn");
            return 1;
        }

        cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
        
        // 3. Initialize OCR Engine
        OcrEngine ocr(lang);
        oled.showText("OCR Ready", "Scanning...");

        cv::Mat frame;
        auto lastOcrTime = std::chrono::steady_clock::now();

        Logger::log(LogLevel::INFO, "Entering main loop...");

        while (keepRunning) {
            cap >> frame;
            if (frame.empty()) {
                Logger::log(LogLevel::WARNING, "Empty frame received.");
                continue;
            }

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastOcrTime).count();

            if (elapsed >= interval) {
                std::string result = ocr.recognize(frame);
                
                if (!result.empty() && result != " ") {
                    Logger::log(LogLevel::INFO, "OCR: " + result);
                    
                    // Display on OLED
                    std::string line1 = result.substr(0, 15);
                    std::string line2 = (result.length() > 15) ? result.substr(15, 15) : "";
                    oled.showText(line1, line2);
                } else {
                    // Optional: show scanning status if result is empty
                    // oled.showText("Scanning...", "");
                }
                
                lastOcrTime = now;
            }

            // Small sleep to prevent 100% CPU usage if OCR is too fast
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        oled.clear();
        oled.showText("System Stopped", "Goodbye!");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        oled.clear();

    } catch (const std::exception& e) {
        Logger::log(LogLevel::ERROR, std::string("Fatal Exception: ") + e.what());
        return 1;
    } catch (...) {
        Logger::log(LogLevel::ERROR, "Unknown Fatal Exception");
        return 1;
    }

    return 0;
}
