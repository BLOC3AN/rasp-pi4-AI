#pragma once
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <stdint.h>

class OledDisplay {
public:
    OledDisplay(const std::string& device, uint8_t address);
    ~OledDisplay();

    void init();
    void display(const cv::Mat& img);
    void clear();
    void showText(const std::string& line1, const std::string& line2 = "");

private:
    int i2c_fd;
    uint8_t addr;
    static const int WIDTH = 132;
    static const int HEIGHT = 64;

    void sendCommand(uint8_t cmd);
    void sendData(const std::vector<uint8_t>& data);
};
