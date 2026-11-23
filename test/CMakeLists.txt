project(x_tests)

find_package(GTest REQUIRED)
file(GLOB_RECURSE TEST_SRC_LIST CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(${PROJECT_NAME} ${TEST_SRC_LIST})
add_dependencies(${PROJECT_NAME} x)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE x GTest::gtest_main pthread)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
