cmake_minimum_required(VERSION 3.20)
project(screen_recorder_cpp_gui)

set(CMAKE_CXX_STANDARD 14)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})
add_executable(screen_recorder_cpp_gui cApp.cpp cMain.cpp include/Recorder.cpp)

if(WIN32)
    include_directories(C:/FFmpeg/include)

    target_link_libraries(${PROJECT_NAME}
            "C:/FFmpeg/lib/avcodec.lib"
            "C:/FFmpeg/lib/avdevice.lib"
            "C:/FFmpeg/lib/avfilter.lib"
            "C:/FFmpeg/lib/avformat.lib"
            "C:/FFmpeg/lib/avutil.lib"
            "C:/FFmpeg/lib/postproc.lib"
            "C:/FFmpeg/lib/swresample.lib"
            "C:/FFmpeg/lib/swscale.lib"
            ${wxWidgets_LIBRARIES}
            )
endif(WIN32)

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