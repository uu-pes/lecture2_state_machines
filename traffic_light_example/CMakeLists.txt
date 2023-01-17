cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(traffic_light_simple)
project(traffic_light_queue)
project(traffic_light_table_driven)

pico_sdk_init()

add_executable(traffic_light_simple src/main_simple.c)
pico_add_extra_outputs(traffic_light_simple)
target_link_libraries(traffic_light_simple pico_stdlib)

add_executable(traffic_light_queue src/main_queue.c)
pico_add_extra_outputs(traffic_light_queue) 
target_link_libraries(traffic_light_queue pico_stdlib) 

add_executable(traffic_light_table_driven src/main_table_driven.c)
pico_add_extra_outputs(traffic_light_table_driven)
target_link_libraries(traffic_light_table_driven pico_stdlib)
