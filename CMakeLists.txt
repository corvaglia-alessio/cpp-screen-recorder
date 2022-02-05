cmake_minimum_required(VERSION 3.20)
project(screen_recorder_cpp_gui)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ./../bin)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})
add_executable(screen_recorder_cpp_gui cApp.cpp cMain.cpp lib/Recorder.cpp)

IF(UNIX OR APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBS REQUIRED IMPORTED_TARGET
            libavcodec
            libavdevice
            libavfilter
            libavformat
            libavutil
            libswscale
            libswresample
            )

    include_directories(.)

    target_link_libraries(${PROJECT_NAME}
            PkgConfig::LIBS
            ${wxWidgets_LIBRARIES}
            )
endif(UNIX OR APPLE)