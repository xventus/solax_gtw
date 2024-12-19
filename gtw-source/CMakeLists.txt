cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(LAMP)
set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/components)
set(EXCLUDE_COMPONENTS freemodbus)


