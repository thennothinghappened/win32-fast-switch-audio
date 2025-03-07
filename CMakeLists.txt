cmake_minimum_required(VERSION 3.30)
project(FastSwitchAudio)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-DWIN32_LEAN_AND_MEAN)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif ()

add_executable(FastSwitchAudio WIN32 src/FastSwitchAudio.cpp
        src/Audio/Device.cpp
        src/Audio/DeviceManager.cpp
        src/Audio/NotificationClient.cpp)

target_sources(FastSwitchAudio PRIVATE src/FastSwitchAudio.rc)
