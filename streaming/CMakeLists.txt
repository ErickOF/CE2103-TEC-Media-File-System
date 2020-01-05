cmake_minimum_required(VERSION 3.8)
project(Streaming)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES lib/Sockets/Socket.cpp lib/Sockets/Socket.h lib/Sockets/ServerSocket.cpp lib/Sockets/ServerSocket.h
        lib/Sockets/ClientSocket.cpp lib/Sockets/ClientSocket.h lib/Sockets/Protocolo.h lib/Funcs.h lib/Funcs.cpp)

add_executable(server ${SOURCES} Server.cpp lib/ControllerNode.cpp lib/ControllerNode.h)
add_executable(cliente ${SOURCES} Cliente.cpp)
add_executable(nodo ${SOURCES} nodo.cpp)

# see https://cmake.org/cmake/help/latest/module/FindBoost.html
find_package(Boost REQUIRED COMPONENTS filesystem serialization)

include_directories(${Boost_INCLUDE_DIR})
# Note: a target should be already defined using 'add_executable' or 'add_library'
target_link_libraries(server ${Boost_LIBRARIES})
target_link_libraries(cliente ${Boost_LIBRARIES})
target_link_libraries(nodo ${Boost_LIBRARIES})

set(CMAKE_AUTOMOC ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(server Threads::Threads)
target_link_libraries(nodo Threads::Threads)