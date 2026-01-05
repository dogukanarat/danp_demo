cmake_minimum_required(VERSION 3.20.0)

# Register your modules
list(APPEND ZEPHYR_EXTRA_MODULES
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/osal
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/danp
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/danp_ftp
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/danp_zephyr_support
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/zephyr_radio
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/zephyr_tmtc
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/cfl
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/cfl_zephyr_support
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(danp_demo)

option(SERVER_MODE "Build the application in server mode" OFF)
option(DGRAM_MODE "Build the application in datagram mode" OFF)

set(SOURCES
    src/main.c
    src/tmtc.c
)

set(INCLUDE_DIRS
    include
)

# set(DEPS)

target_sources(app PRIVATE ${SOURCES})
target_include_directories(app PRIVATE ${INCLUDE_DIRS})
target_compile_definitions(app PRIVATE
    $<$<BOOL:${SERVER_MODE}>:SERVER_MODE>
    $<$<BOOL:${DGRAM_MODE}>:DGRAM_MODE>
)
# target_link_libraries(app PRIVATE ${DEPS})
