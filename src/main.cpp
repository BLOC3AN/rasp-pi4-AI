#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include "Config.hpp"
#include "Logger.hpp"
#include "OledDisplay.hpp"
#include "OcrEngine.hpp"
#include "MjpegServer.hpp"

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
        int webPort = cfg.getInt("WEBSERVER_PORT", 8080);
        int previewWidth = cfg.getInt("PREVIEW_WIDTH", 320);

        // 0. Initialize Web Server for Alignment
        MjpegServer webServer(webPort);
        webServer.start();

        // 1. Initialize Display
        OledDisplay oled(i2cDev, oledAddr);
        oled.showText("System Starting", "Please wait...");

        // 2. Initialize Camera (Using GStreamer for RPi optimization)
        // Note: libcamerasrc is the modern stack, v4l2src is the legacy/v4l2 stack.
        // We try both or stick to a robust one.
        std::string pipeline = "v4l2src device=" + i2cDev + " ! video/x-raw, width=" + std::to_string(width) + 
                               ", height=" + std::to_string(height) + ", framerate=30/1 ! videoconvert ! appsink";
        
        // Revised pipeline for modern libcamera-based OS
        std::string libcamera_pipeline = "libcamerasrc ! video/x-raw, width=" + std::to_string(width) + 
                                         ", height=" + std::to_string(height) + " ! videoconvert ! appsink";

        cv::VideoCapture cap;
        Logger::log(LogLevel::INFO, "Opening camera with GStreamer...");
        
        // Try libcamera first (modern)
        cap.open(libcamera_pipeline, cv::CAP_GSTREAMER);
        
        if (!cap.isOpened()) {
            Logger::log(LogLevel::WARNING, "libcamerasrc failed, trying v4l2src...");
            cap.open(pipeline, cv::CAP_GSTREAMER);
        }

        if (!cap.isOpened()) {
            Logger::log(LogLevel::WARNING, "GStreamer failed, trying default V4L2...");
            cap.open(camIdx, cv::CAP_V4L2);
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

            // Update web preview (Resized for performance)
            if (previewWidth > 0 && previewWidth < frame.cols) {
                cv::Mat preview;
                float aspect = (float)frame.rows / frame.cols;
                cv::resize(frame, preview, cv::Size(previewWidth, (int)(previewWidth * aspect)));
                webServer.updateFrame(preview);
            } else {
                webServer.updateFrame(frame);
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

        webServer.stop();
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
