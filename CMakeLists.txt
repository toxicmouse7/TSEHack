cmake_minimum_required(VERSION 3.30)
project(showdown_hack)

set(CMAKE_CXX_STANDARD 23)

add_library(showdown_hack SHARED library.cpp
        lua_functions.cpp
        lua_functions.h
        utils.hpp
)