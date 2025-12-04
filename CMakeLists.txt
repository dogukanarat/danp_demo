cmake_minimum_required(VERSION 3.20.0)

# Register your modules
list(APPEND ZEPHYR_EXTRA_MODULES 
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/osal
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/danp
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_nucleo_app)

target_sources(app PRIVATE src/main.c)