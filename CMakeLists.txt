cmake_minimum_required(VERSION 3.30)
project(FastSwitchAudio)

set(CMAKE_CXX_STANDARD 20)

add_executable(FastSwitchAudio WIN32 src/FastSwitchAudio.cpp
        src/Audio/Device.cpp
        src/Audio/DeviceManager.cpp
        src/Audio/NotificationClient.cpp)

target_sources(FastSwitchAudio PRIVATE src/FastSwitchAudio.rc)

target_compile_definitions(FastSwitchAudio PRIVATE
        UNICODE
        _UNICODE
        _DEBUG
        WIN32_LEAN_AND_MEAN
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(FastSwitchAudio PRIVATE _DEBUG)
endif ()
