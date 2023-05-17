add_library(hagl_hal ${CMAKE_CURRENT_LIST_DIR}/source/hagl_hal.c)

target_include_directories(hagl_hal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(hagl_hal  hagl
                                spi_st7735
                                hardware_spi
                                fb_pio_dma_lut
                                pathfinder_hal
)