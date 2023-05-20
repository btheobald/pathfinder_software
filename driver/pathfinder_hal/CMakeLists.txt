add_library(pathfinder_hal  source/pathfinder.c
                            source/gps_d.c
                            source/utility.c)

target_include_directories(pathfinder_hal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(pathfinder_hal    pico_stdlib
                                        hardware_spi
                                        hardware_i2c 
                                        hardware_uart 
                                        hardware_pio 
                                        hardware_dma
                                        hardware_pwm

)