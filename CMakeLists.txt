cmake_minimum_required(VERSION 3.20)
project(xtl)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
