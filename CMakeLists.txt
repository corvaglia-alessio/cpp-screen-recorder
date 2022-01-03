cmake_minimum_required(VERSION 3.20)
project(screen_recorder_cpp_gui)

set(CMAKE_CXX_STANDARD 14)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
include(${wxWidgets_USE_FILE})

#include_directories(D:/Projects/C++/screen-recorder-cpp-gui/libs/wx-3.1.5-x64/include/msvc;D:/Projects/C++/screen-recorder-cpp-gui/libs/wx-3.1.5-x64/include)
add_executable(screen_recorder_cpp_gui cApp.cpp cMain.cpp)

target_link_libraries(screen_recorder_cpp_gui ${wxWidgets_LIBRARIES})
#target_link_libraries(screen_recorder_cpp_gui wx::net wx::core wx::base)
#target_link_libraries(screen_recorder_cpp_gui D:/Projects/C++/screen-recorder-cpp-gui/libs/wx-3.1.5-x64/lib/vc14x_x64_dll)