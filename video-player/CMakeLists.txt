cmake_minimum_required(VERSION 3.8)
project(ui)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES Sockets/ClientSocket.cpp Sockets/ClientSocket.h Sockets/Protocolo.h Sockets/Socket.cpp Sockets/Socket.h
        Funcs.cpp Funcs.h mainwindow.cpp mainwindow.h mainwindow.ui videowidget.cpp videowidget.h videowidget.ui resources.qrc)

find_package(Qt5Core REQUIRED )
find_package(Qt5Widgets REQUIRED )
find_package(Qt5Gui REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5MultimediaWidgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
add_executable(ui ${SOURCE_FILES} main.cpp)

qt5_use_modules(ui Core Widgets Gui Multimedia MultimediaWidgets)

# see https://cmake.org/cmake/help/latest/module/FindBoost.html
find_package(Boost REQUIRED COMPONENTS filesystem serialization)

include_directories(${Boost_INCLUDE_DIR})
# Note: a target should be already defined using 'add_executable' or 'add_library'
target_link_libraries(ui ${Boost_LIBRARIES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(ui Threads::Threads)