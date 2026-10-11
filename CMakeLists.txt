cmake_minimum_required(VERSION 3.20)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)
project(MineSweeper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 路径，斜杠用 /
set(CMAKE_PREFIX_PATH "D:/Qt/6.5.3/mingw_64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(MineSweeper
        main.cpp
        mainwindow.h
        mainwindow.cpp
        MineSweeper.h
        MineSweeper.cpp
)
target_link_libraries(MineSweeper PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)