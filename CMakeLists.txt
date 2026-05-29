cmake_minimum_required(VERSION 3.16)

project(StudyFlow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

qt_standard_project_setup()

qt_add_executable(StudyFlow
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(StudyFlow PRIVATE Qt6::Widgets Qt6::Multimedia)