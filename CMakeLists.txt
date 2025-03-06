cmake_minimum_required(VERSION 3.30)
project(FastSwitchAudio)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-DWIN32_LEAN_AND_MEAN)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif ()

add_executable(FastSwitchAudio WIN32 FastSwitchAudio.cpp
        FastSwitchAudio.h
        PolicyConfig.h
        PopupMenu.h
        utils.h
        resource.h
        targetver.h
        framework.h
        Audio/Audio.h
        Audio/Device.cpp
        Audio/Device.h
        Audio/DeviceManager.cpp
        Audio/DeviceManager.h
        Audio/NotificationClient.cpp)

target_sources(FastSwitchAudio PRIVATE FastSwitchAudio.rc)
