#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

class OcrEngine {
public:
    OcrEngine(const std::string& lang);
    ~OcrEngine();

    std::string recognize(const cv::Mat& img);

private:
    tesseract::TessBaseAPI* api;
};
