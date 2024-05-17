cmake_minimum_required(VERSION 3.26.4)
project(practice_6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(main main.cpp)
target_link_libraries(main PRIVATE Qt6::Widgets)
