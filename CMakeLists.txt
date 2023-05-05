cmake_minimum_required(VERSION 3.13)

include(cmake/pico_sdk_import.cmake)

SET(FREERTOS_KERNEL_PATH "${CMAKE_SOURCE_DIR}/thirdparty/FreeRTOS-Kernel-SMP" CACHE STRING "Kernel Path")
include(cmake/FreeRTOS_Kernel_import.cmake)

project(pathfinder C CXX ASM)

set(CMAKE_C_COMPILER "/usr/bin/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/arm-none-eabi-g++")

pico_sdk_init()

add_subdirectory(thirdparty)
add_subdirectory(driver)
add_subdirectory(source)