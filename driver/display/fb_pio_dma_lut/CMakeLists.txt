add_library(fb_pio_dma_lut  ${CMAKE_CURRENT_LIST_DIR}/source/fb_dma_pio_lut.c
                            ${CMAKE_CURRENT_LIST_DIR}/source/lut.c 
)

pico_generate_pio_header(fb_pio_dma_lut ${CMAKE_CURRENT_LIST_DIR}/source/addr_lut.pio)

target_include_directories(fb_pio_dma_lut PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(fb_pio_dma_lut    hardware_dma
                                        hardware_pio
                                        hardware_spi
)