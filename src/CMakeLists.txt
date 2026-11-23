project(x)

file(GLOB_RECURSE LIB_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/x/*.cpp)
add_library(${PROJECT_NAME} ${LIB_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(BIN_NAME ${PROJECT_NAME}_bin)
add_executable(${BIN_NAME} main.cpp)
add_dependencies(${BIN_NAME} ${PROJECT_NAME})
target_link_libraries(${BIN_NAME} PRIVATE ${PROJECT_NAME})
