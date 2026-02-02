#include "OcrEngine.hpp"
#include "Logger.hpp"
#include <algorithm>

OcrEngine::OcrEngine(const std::string& lang) {
    api = new tesseract::TessBaseAPI();
    if (api->Init(NULL, lang.c_str())) {
        Logger::log(LogLevel::ERROR, "Could not initialize Tesseract with lang: " + lang);
        throw std::runtime_error("Tesseract Init Failed");
    }
    api->SetPageSegMode(tesseract::PSM_AUTO);
    Logger::log(LogLevel::INFO, "Tesseract Engine Loaded (" + lang + ")");
}

OcrEngine::~OcrEngine() {
    if (api) {
        api->End();
        delete api;
    }
}

std::string OcrEngine::recognize(const cv::Mat& img) {
    if (img.empty()) return "";

    cv::Mat gray, processed;
    if (img.channels() == 3) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img;
    }

    // Optimization: Shrink image for faster OCR if it's too large
    // But don't shrink too much or accuracy drops. 
    // 640x480 is usually fine.
    
    // Simple preprocessing
    cv::threshold(gray, processed, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    api->SetImage(processed.data, processed.cols, processed.rows, 1, processed.step);
    char* outText = api->GetUTF8Text();
    std::string text(outText);
    delete[] outText;

    // Post-processing: remove newlines and extra spaces
    text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    
    return text;
}
