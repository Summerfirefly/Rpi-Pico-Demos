cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pio-lcd-ili9341)

pico_sdk_init()

add_executable(${PROJECT_NAME})

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio_write.pio)
target_sources(${PROJECT_NAME} PRIVATE main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib hardware_pio)

pico_add_extra_outputs(${PROJECT_NAME})
