cmake_minimum_required(VERSION 3.30)
project(FastSwitchAudio)

set(CMAKE_CXX_STANDARD 20)

add_executable(FastSwitchAudio WIN32 src/FastSwitchAudio.cpp
        src/Audio/Device.cpp
        src/Audio/DeviceManager.cpp
        src/Audio/NotificationClient.cpp)

target_sources(FastSwitchAudio PRIVATE src/FastSwitchAudio.rc)

target_include_directories(FastSwitchAudio PRIVATE src)

target_compile_definitions(FastSwitchAudio PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(FastSwitchAudio PRIVATE
            /Wall
            /external:anglebrackets
            /external:W0
            /external:templates-)
else ()
    target_compile_options(FastSwitchAudio PRIVATE
            -Wall
            -Wno-shadow-field-in-constructor
            -Wno-language-extension-token
            -Wno-missing-designated-field-initializers
            -Wno-switch-default
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-c++98-compat-extra-semi)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(FastSwitchAudio PRIVATE _DEBUG)
endif ()
