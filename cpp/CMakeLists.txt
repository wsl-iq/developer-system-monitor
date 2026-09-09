cmake_minimum_required(VERSION 3.20)

project(SystemMonitorNative VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all source files - use full paths relative to this CMakeLists.txt
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/disk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/exports.cpp
)

# Create shared library (DLL)
add_library(SystemMonitorNative SHARED ${SOURCES})

# Include directories
target_include_directories(SystemMonitorNative
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Add compile definitions
target_compile_definitions(SystemMonitorNative PRIVATE 
    SYSTEMMONITORNATIVE_EXPORTS
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
    _WIN32_WINNT=0x0A00
    WINVER=0x0A00
)

# Windows-specific libraries
if(WIN32)
    target_link_libraries(SystemMonitorNative
        PRIVATE
            advapi32
            psapi
            pdh
            iphlpapi
            ws2_32
            d3d11
            dxgi
            setupapi
            cfgmgr32
            ole32
            oleaut32
            wbemuuid
            version
    )
    
    # Set output name
    set_target_properties(SystemMonitorNative PROPERTIES
        OUTPUT_NAME "SystemMonitorNative"
        PREFIX ""
    )
    
    # Post-build copy to Python directory
    add_custom_command(TARGET SystemMonitorNative POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SystemMonitorNative>
            ${CMAKE_SOURCE_DIR}/python/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SystemMonitorNative>
            ${CMAKE_SOURCE_DIR}/python/core/
    )
endif()