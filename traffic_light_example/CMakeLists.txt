cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(traffic_light_simple)

pico_sdk_init()

add_executable(traffic_light_simple src/main_simple.c)
pico_add_extra_outputs(traffic_light_simple)
target_link_libraries(traffic_light_simple pico_stdlib)
