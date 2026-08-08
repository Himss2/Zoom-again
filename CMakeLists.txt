cmake_minimum_required(VERSION 3.22)
project(ZoomRewrite CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "ZoomRewrite must be built for arm64-v8a (got: ${ANDROID_ABI})")
endif()

include(FetchContent)

FetchContent_Declare(
    boost_pfr
    GIT_REPOSITORY https://github.com/boostorg/pfr.git
    GIT_TAG        2.2.0
)
FetchContent_MakeAvailable(boost_pfr)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG        v0.9.6
)
FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.0.2
)
FetchContent_MakeAvailable(fmt)

# Diperbarui ke branch 'main' agar mendukung ABI LeviMC Launcher v1.5.9+
FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(preloader_android)

add_library(ZoomRewrite SHARED
    src/Core/ModContext.cpp
    src/Core/ModEntry.cpp
    src/Core/Config.cpp
    src/CameraHook/CameraHook.cpp
    src/CameraHook/FrameHook.cpp
    src/ZoomController/ZoomController.cpp
    src/TouchController/TouchController.cpp
    src/ZoomButton/ZoomButton.cpp
)

target_include_directories(ZoomRewrite PRIVATE
    include
    ${boost_pfr_SOURCE_DIR}/include
    ${magic_enum_SOURCE_DIR}/include
)
target_link_libraries(ZoomRewrite PRIVATE preloader nlohmann_json::nlohmann_json fmt::fmt log EGL)

set_target_properties(ZoomRewrite PROPERTIES
    OUTPUT_NAME "ZoomRewrite"
)
