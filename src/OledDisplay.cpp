#include "OledDisplay.hpp"
#include "Logger.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <iostream>

OledDisplay::OledDisplay(const std::string& device, uint8_t address) : addr(address) {
    if ((i2c_fd = open(device.c_str(), O_RDWR)) < 0) {
        Logger::log(LogLevel::ERROR, "Failed to open i2c bus: " + device);
        throw std::runtime_error("I2C Open Failed");
    }
    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
        Logger::log(LogLevel::ERROR, "Failed to acquire bus access to " + std::to_string(addr));
        throw std::runtime_error("I2C Access Failed");
    }
    init();
}

OledDisplay::~OledDisplay() {
    if (i2c_fd >= 0) close(i2c_fd);
}

void OledDisplay::sendCommand(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    if (write(i2c_fd, buf, 2) != 2) {
        // Quiet fail or log
    }
}

void OledDisplay::sendData(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> buf;
    buf.reserve(data.size() + 1);
    buf.push_back(0x40);
    buf.insert(buf.end(), data.begin(), data.end());
    if (write(i2c_fd, buf.data(), buf.size()) != (ssize_t)buf.size()) {
        // Quiet fail
    }
}

void OledDisplay::init() {
    uint8_t init_cmds[] = {
        0xAE, // Display OFF
        0xA1, // Segment Re-map
        0xC8, // COM Output Scan Direction
        0xA8, 0x3F, // Multiplex Ratio
        0xD3, 0x00, // Display Offset
        0x40, // Display Start Line
        0x81, 0xCF, // Contrast Control
        0xD9, 0xF1, // Pre-charge Period
        0xDB, 0x40, // VCOM Deselect Level
        0xA4, // Resume to RAM content display
        0xA6, // Normal Display
        0xAF  // Display ON
    };
    for (uint8_t cmd : init_cmds) sendCommand(cmd);
    clear();
    Logger::log(LogLevel::INFO, "OLED SH1106 Initialized.");
}

void OledDisplay::clear() {
    for (int page = 0; page < 8; page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x00);
        sendCommand(0x10);
        sendData(std::vector<uint8_t>(WIDTH, 0));
    }
}

void OledDisplay::display(const cv::Mat& img) {
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(WIDTH, HEIGHT));
    cv::threshold(resized, resized, 128, 255, cv::THRESH_BINARY);

    for (int page = 0; page < 8; page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x00);
        sendCommand(0x10);
        
        std::vector<uint8_t> page_data(WIDTH, 0);
        for (int x = 0; x < WIDTH; x++) {
            uint8_t byte = 0;
            for (int y = 0; y < 8; y++) {
                if (resized.at<uint8_t>(page * 8 + y, x) > 0) {
                    byte |= (1 << y);
                }
            }
            page_data[x] = byte;
        }
        sendData(page_data);
    }
}

void OledDisplay::showText(const std::string& line1, const std::string& line2) {
    cv::Mat canvas = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
    cv::putText(canvas, line1, cv::Point(2, 20), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255), 1);
    if (!line2.empty()) {
        cv::putText(canvas, line2, cv::Point(2, 45), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255), 1);
    }
    display(canvas);
}
