#include "OledDisplay.hpp"
#include "Logger.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cerrno>

OledDisplay::OledDisplay(const std::string& device, uint8_t address) : addr(address) {
    if ((i2c_fd = open(device.c_str(), O_RDWR)) < 0) {
        Logger::log(LogLevel::ERROR, "Failed to open i2c bus: " + device + " (Error: " + std::string(strerror(errno)) + ")");
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
        0xAE,       // Display Off
        0xD5, 0x80, // Set Display Clock Divide Ratio/ Oscillator Frequency
        0xA8, 0x3F, // Set Multiplex Ratio
        0xD3, 0x00, // Set Display Offset
        0x40,       // Set Display Start Line
        0xAD, 0x8B, // Set Charge Pump (SH1106 spec)
        0xA1,       // Set Segment Re-Map (Normal)
        0xC8,       // Set COM Output Scan Direction
        0xDA, 0x12, // Set COM Pins Hardware Configuration
        0x81, 0xBF, // Set Contrast Control
        0xD9, 0x22, // Set Pre-charge Period
        0xDB, 0x40, // Set VCOMH Deselect Level
        0x30,       // Set Pump Voltage (SH1106 specific)
        0xA4,       // Entire Display On (Resume from RAM)
        0xA6,       // Set Normal/Inverse Display
        0xAF        // Display On
    };
    for (uint8_t cmd : init_cmds) sendCommand(cmd);
    clear();
    Logger::log(LogLevel::INFO, "OLED SH1106 (I2C 0x3C) Optimized & Ready!");
}

void OledDisplay::clear() {
    for (int page = 0; page < 8; page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x02); // Lower column address (Offset 2 for SH1106)
        sendCommand(0x10); // Higher column address
        sendData(std::vector<uint8_t>(WIDTH, 0));
    }
}

void OledDisplay::display(const cv::Mat& img) {
    cv::Mat resized;
    // We use 128x64 for drawing, then center it on the 132x64 SH1106
    cv::resize(img, resized, cv::Size(128, 64));
    cv::threshold(resized, resized, 128, 255, cv::THRESH_BINARY);

    for (int page = 0; page < 8; page++) {
        sendCommand(0xB0 + page);
        sendCommand(0x02); // Column address lower 4 bits (Offset 2)
        sendCommand(0x10); // Column address upper 4 bits
        
        std::vector<uint8_t> page_data(128, 0);
        for (int x = 0; x < 128; x++) {
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
