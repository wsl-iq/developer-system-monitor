cmake_minimum_required(VERSION 3.20)
project(DeveloperSystemMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add C++ subdirectory
add_subdirectory(cpp)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    add_definitions(-DSYSTEMMONITORNATIVE_EXPORTS)
endif()