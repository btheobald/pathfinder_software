add_library(i2c_ms5637 ms5637.c)

target_include_directories(i2c_ms5637 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

target_link_libraries(i2c_ms5637 pico_stdlib hardware_i2c)