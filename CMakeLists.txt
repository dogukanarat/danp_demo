cmake_minimum_required(VERSION 3.20.0)

# Register your modules
list(APPEND ZEPHYR_EXTRA_MODULES 
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/osal
    ${CMAKE_CURRENT_SOURCE_DIR}/libraries/danp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/zephyr-radio
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(danp_demo)

set(SOURCES
    src/main.c
    src/server.c
    src/client.c
    src/utilities.c
    src/shell.c
)

set(INCLUDE_DIRS
    include
)



# set(DEPS)

target_sources(app PRIVATE ${SOURCES})
target_include_directories(app PRIVATE ${INCLUDE_DIRS})
# target_link_libraries(app PRIVATE ${DEPS})