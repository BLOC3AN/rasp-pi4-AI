#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <netinet/in.h>

class MjpegServer {
public:
    MjpegServer(int port);
    ~MjpegServer();

    void start();
    void stop();
    void updateFrame(const cv::Mat& frame);

private:
    void serverLoop();
    void handleClient(int clientSocket);

    int port;
    int serverFd;
    std::atomic<bool> running;
    std::thread serverThread;
    
    cv::Mat currentFrame;
    std::vector<uchar> currentJpeg;
    std::mutex frameMutex;
};
