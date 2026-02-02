#include "MjpegServer.hpp"
#include "Logger.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>

MjpegServer::MjpegServer(int port) : port(port), serverFd(-1), running(false) {}

MjpegServer::~MjpegServer() {
    stop();
}

void MjpegServer::start() {
    running = true;
    serverThread = std::thread(&MjpegServer::serverLoop, this);
}

void MjpegServer::stop() {
    running = false;
    if (serverFd != -1) {
        shutdown(serverFd, SHUT_RDWR);
        close(serverFd);
        serverFd = -1;
    }
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void MjpegServer::updateFrame(const cv::Mat& frame) {
    if (frame.empty()) return;
    
    std::vector<uchar> buf;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 70};
    cv::imencode(".jpg", frame, buf, params);
    
    std::lock_guard<std::mutex> lock(frameMutex);
    currentJpeg = std::move(buf);
}

void MjpegServer::serverLoop() {
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        Logger::log(LogLevel::ERROR, "Could not create socket for MjpegServer");
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        Logger::log(LogLevel::ERROR, "MjpegServer bind failed on port " + std::to_string(port));
        return;
    }

    if (listen(serverFd, 5) < 0) {
        Logger::log(LogLevel::ERROR, "MjpegServer listen failed");
        return;
    }

    Logger::log(LogLevel::INFO, "Web Preview Server started on port " + std::to_string(port));

    while (running) {
        int clientSocket = accept(serverFd, nullptr, nullptr);
        if (clientSocket < 0) {
            if (running) Logger::log(LogLevel::WARNING, "MjpegServer accept error");
            continue;
        }
        std::thread([this, clientSocket]() { this->handleClient(clientSocket); }).detach();
    }
}

void MjpegServer::handleClient(int clientSocket) {
    // Standard MJPEG HTTP Header
    std::string header = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Max-Age: 0\r\n"
        "Expires: 0\r\n"
        "Cache-Control: no-cache, private\r\n"
        "Pragma: no-cache\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=--frame\r\n\r\n";
    
    if (write(clientSocket, header.c_str(), header.length()) < 0) {
        close(clientSocket);
        return;
    }

    while (running) {
        std::vector<uchar> jpeg;
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            if (!currentJpeg.empty()) {
                jpeg = currentJpeg;
            }
        }

        if (!jpeg.empty()) {
            std::stringstream ss;
            ss << "--frame\r\n"
               << "Content-Type: image/jpeg\r\n"
               << "Content-Length: " << jpeg.size() << "\r\n"
               << "X-Timestamp: " << std::time(nullptr) << "\r\n"
               << "\r\n";
            std::string partHeader = ss.str();
            
            if (write(clientSocket, partHeader.c_str(), partHeader.length()) < 0) break;
            if (write(clientSocket, jpeg.data(), jpeg.size()) < 0) break;
            if (write(clientSocket, "\r\n", 2) < 0) break;
        } else {
            // Wait for a frame if empty
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(65)); // ~15 FPS target
    }
    
    close(clientSocket);
}
