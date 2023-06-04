add_library(spi_st7789 source/st7789.c)

target_include_directories(spi_st7789 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(spi_st7789 PUBLIC pico_stdlib 
                                        hardware_spi
)