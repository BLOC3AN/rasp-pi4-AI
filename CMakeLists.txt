cmake_minimum_required(VERSION 3.10)
project(RaspPi4OCR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenCV REQUIRED)

# Use PkgConfig for Tesseract and Leptonica as they often don't provide CMake configs on Debian
find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT REQUIRED tesseract)
pkg_check_modules(LEPTONICA REQUIRED lept)

# Include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${TESSERACT_INCLUDE_DIRS}
    ${LEPTONICA_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Executable
add_executable(ocr_app ${SRC_FILES})

# Link libraries
target_link_libraries(ocr_app 
    ${OpenCV_LIBS} 
    ${TESSERACT_LIBRARIES}
    ${LEPTONICA_LIBRARIES}
    i2c
    pthread
)

# Custom command to copy .env to build directory
# add_custom_command(TARGET ocr_app POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/.env ${CMAKE_BINARY_DIR}/.env)
