cmake_minimum_required(VERSION 3.10.2)
SET(CMAKE_CXX_STANDARD 17)
project(realtime_plotter)



find_package(PythonLibs 3.6 REQUIRED)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-w)

include_directories(
    ${CMAKE_SOURCE_DIR}/include/
    ${PYTHON_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    ${PYTHON_LIBRARIES}
)
