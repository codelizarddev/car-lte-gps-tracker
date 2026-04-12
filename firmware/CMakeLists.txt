cmake_minimum_required(VERSION 3.16)

# Resolve ESP-IDF built-ins from IDF_PATH, and only the intended local components
# from this project, so legacy folders under ./components cannot shadow built-ins.
set(COMPONENT_DIRS
    "$ENV{IDF_PATH}/components"
    "${CMAKE_CURRENT_SOURCE_DIR}/main"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/gnss"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/modem"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/mqtt_wrapper"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/power"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/relay"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(car-lte-gps-tracker)
