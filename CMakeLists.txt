cmake_minimum_required(VERSION 3.10)
project(RaspPi4OCR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenCV REQUIRED)
find_package(Tesseract REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/include/tesseract)
include_directories(/usr/include/leptonica)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Executable
add_executable(ocr_app ${SRC_FILES})

# Link libraries
target_link_libraries(ocr_app 
    ${OpenCV_LIBS} 
    tesseract 
    lept 
    i2c
    pthread
)

# Custom command to copy .env to build directory
# add_custom_command(TARGET ocr_app POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/.env ${CMAKE_BINARY_DIR}/.env)
