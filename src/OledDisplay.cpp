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
        Logger::log(LogLevel::WARNING, "I2C Command Failed: " + std::to_string(cmd) + " (" + std::string(strerror(errno)) + ")");
    }
    usleep(1000); // 1ms delay after command for stabilization
}

void OledDisplay::sendData(const std::vector<uint8_t>& data) {
    // Some I2C controllers (like Pi's bit-banging or high load) fail on large writes.
    // 8 bytes chunk size is extremely safe.
    const size_t CHUNK_SIZE = 8;
    for (size_t i = 0; i < data.size(); i += CHUNK_SIZE) {
        size_t size = std::min(CHUNK_SIZE, data.size() - i);
        std::vector<uint8_t> buf;
        buf.reserve(size + 1);
        buf.push_back(0x40);
        buf.insert(buf.end(), data.begin() + i, data.begin() + i + size);
        
        if (write(i2c_fd, buf.data(), buf.size()) != (ssize_t)buf.size()) {
            Logger::log(LogLevel::WARNING, "I2C Data Write Failed (" + std::string(strerror(errno)) + ")");
            break;
        }
        usleep(500); // 0.5ms delay between data chunks
    }
}

void OledDisplay::init() {
    uint8_t init_cmds[] = {
        0xAE,       // Display Off
        0xD5, 0x80, // Clock Divide Ratio
        0xA8, 0x3F, // Multiplex Ratio
        0xD3, 0x00, // Display Offset
        0x40,       // Start Line
        0xAD, 0x8B, // DC-DC Control Mode (Charge Pump ON)
        0xA1,       // Segment Re-map
        0xC8,       // COM Output Scan Direction
        0xDA, 0x12, // COM Pins Hardware Configuration
        0x81, 0x80, // Contrast Control (Reduced for stability)
        0xD9, 0x22, // Pre-charge Period
        0xDB, 0x35, // VCOMH Deselect Level
        0xA4,       // Entire Display On
        0xA6,       // Normal Display
        0xAF        // Display On
    };
    for (uint8_t cmd : init_cmds) sendCommand(cmd);
    clear();
    Logger::log(LogLevel::INFO, "OLED Stable Init Complete.");
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
