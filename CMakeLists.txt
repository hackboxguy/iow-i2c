cmake_minimum_required(VERSION 2.8.9)
set(COMPONENT_NAME iow-i2c)  #set the name of the binary

project(${COMPONENT_NAME})
file(GLOB SOURCES "*.cpp")
file(GLOB SOURCES2 "*.c")
list (APPEND SOURCES ${SOURCES2})

# Add #include path of all dependent components
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(${COMPONENT_NAME} ${SOURCES})

install(TARGETS ${COMPONENT_NAME} DESTINATION sbin)
