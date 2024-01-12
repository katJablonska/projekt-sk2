cmake_minimum_required(VERSION 3.27)
project(SK2-projekt)

set(CMAKE_CXX_STANDARD 20)

add_executable(SK2-projekt main.cpp
        Server.cpp
        Server.h
        Client.cpp
        Client.h
        Channel.cpp
        Channel.h
        Channel.cpp
        InvalidRequest.h
)
