cmake_minimum_required(VERSION 3.16)

# Include local components explicitly to avoid name collisions with ESP-IDF built-ins
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/components/gnss"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/modem"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/mqtt_wrapper"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/power"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/relay"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(car-lte-gps-tracker)
